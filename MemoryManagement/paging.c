/*
 * Copyright (C) 2017 DeepAQ.
 * All Rights Reserved.
 */

#include <stdlib.h>
#include <time.h>
#include "bottom.h"
#include "paging.h"

m_size_t get_available_mem() {
    return (mem_read(BASE_AVL_MEM) << 8)
            + mem_read(BASE_AVL_MEM + 1);
}

void set_available_mem(m_size_t size) {
    mem_write(size >> 8, BASE_AVL_MEM);
    mem_write(size, BASE_AVL_MEM + 1);
}

m_size_t get_available_hdd() {
    return (mem_read(BASE_AVL_HDD) << 16)
            + (mem_read(BASE_AVL_HDD + 1) << 8)
            + mem_read(BASE_AVL_HDD + 2);
}

void set_available_hdd(m_size_t size) {
    mem_write(size >> 16, BASE_AVL_HDD);
    mem_write(size >> 8, BASE_AVL_HDD + 1);
    mem_write(size, BASE_AVL_HDD + 2);
}

v_address find_vpn(m_size_t p_num) {
    int offset = 0;
    for (int i = 0; i < SIZE_BM_VPN; i++) {
        data_unit bm_byte = mem_read(BASE_BM_VPN + i);
        // start
        int j = 0;
        while (j < 8 && (bm_byte & (1 << (7 - j))) == 0) j++;
        if (offset + j >= p_num)
            return i * 8 - offset;
        if (j >= 8) {
            offset += 8;
            continue;
        }
        // middle
        while (1) {
            while (j < 8 && (bm_byte & (1 << (7 - j))) != 0) j++;
            if (j >= 8)
                break;
            int k = 1;
            while (j + k < 8 && (bm_byte & (1 << (7 - j - k))) == 0) k++;
            if (k >= p_num)
                return i * 8 + j;
            j += k;
        }
        // end
        offset = 0;
        while (offset < 8 && (bm_byte & (1 << offset)) == 0) offset++;
    }
    return -1;
}

unsigned int vpt_get(v_address vpn) {
    if (vpn >= 2 * (NUM_PAGE_MEM + NUM_PAGE_HDD))
        return 0xFFFFFFFF;
    data_unit byte1 = mem_read(BASE_VPT + vpn * 3);
    if ((byte1 & 0x80) == 0)
        return 0xFFFFFFFF;
    return (byte1 << 16)
            + (mem_read(BASE_VPT + vpn * 3 + 1) << 8)
            + mem_read(BASE_VPT + vpn * 3 + 2);
}

unsigned int vpt_put(v_address vpn, unsigned int vpt_entry) {
    mem_write(vpt_entry >> 16, BASE_VPT + vpn * 3);
    mem_write(vpt_entry >> 8, BASE_VPT + vpn * 3 + 1);
    mem_write(vpt_entry, BASE_VPT + vpn * 3 + 2);
}

unsigned int ppt_get(p_address ppn) {
    if (ppn >= NUM_PAGE_MEM + NUM_PAGE_HDD)
        return 0xFFFFFFFF;
    return (mem_read(BASE_PPT + ppn * 3) << 16)
            + (mem_read(BASE_PPT + ppn * 3 + 1) << 8)
            + mem_read(BASE_PPT + ppn * 3 + 2);
}

unsigned int ppt_put(p_address ppn, unsigned int ppt_entry) {
    mem_write(ppt_entry >> 16, BASE_PPT + ppn * 3);
    mem_write(ppt_entry >> 8, BASE_PPT + ppn * 3 + 1);
    mem_write(ppt_entry, BASE_PPT + ppn * 3 + 2);
}

void pt_put(v_address vpn, p_address ppn, m_pid_t pid, m_size_t size, data_unit continuation) {
    // write vpt
    unsigned int vpt_entry = ppn | 0x800000;
    if (continuation > 0) {
        vpt_entry |= 0x400000;
    }
    vpt_put(vpn, vpt_entry);
    // write ppt
    unsigned int ppt_entry = (pid << 12) + size - 1;
    ppt_put(ppn, ppt_entry);
    // update bitmap
    mem_write(
        mem_read(BASE_BM_VPN + vpn / 8) | (1 << (7 - (vpn % 8))),
        BASE_BM_VPN + vpn / 8
    );
    mem_write(
        mem_read(BASE_BM_PPN + ppn / 8) | (1 << (7 - (ppn % 8))),
        BASE_BM_PPN + ppn / 8
    );
    // update available pages
    if (ppn < NUM_PAGE_MEM) {
        set_available_mem(get_available_mem() - 1);
    } else {
        set_available_hdd(get_available_hdd() - 1);
    }
}

void pt_remove(v_address vpn) {
    unsigned int vpt_entry = vpt_get(vpn);
    p_address ppn = vpt_entry & 0x03FFFF;
    mem_write(0, BASE_VPT + vpn * 3);
    // update bitmap
    mem_write(
        mem_read(BASE_BM_VPN + vpn / 8) & (~(1 << (7 - (vpn % 8)))),
        BASE_BM_VPN + vpn / 8
    );
    mem_write(
        mem_read(BASE_BM_PPN + ppn / 8) & (~(1 << (7 - (ppn % 8)))),
        BASE_BM_PPN + ppn / 8
    );
    // update available pages
    if (ppn < NUM_PAGE_MEM) {
        set_available_mem(get_available_mem() + 1);
    } else {
        set_available_hdd(get_available_hdd() + 1);
    }
}

p_address hdd_swap(v_address vpn, unsigned int vpt_entry, unsigned int ppt_entry) {
    p_address hdd_pn = vpt_entry & 0x03FFFF;
    p_address hdd_offset = hdd_pn - NUM_PAGE_MEM;
    p_address mem_pn;
    if (get_available_mem() > 0) {
        // Load directly
        for (int i = 0; i < SIZE_BM_PPN; i++) {
            data_unit bm_byte = mem_read(BASE_BM_PPN + i);
            int j;
            for (j = 0; j < 8; j++) {
                if ((bm_byte & (1 << (7 - j))) == 0) {
                    mem_pn = i * 8 + j;
                    break;
                }
            }
            if (mem_pn == i * 8 + j)
                break;
        }
        // write new vpt entry
        vpt_put(vpn, (vpt_entry & 0xFC0000) + mem_pn);
        // write new ppt entry
        ppt_put(mem_pn, ppt_entry);
        // update bitmap
        mem_write(
            mem_read(BASE_BM_PPN + hdd_pn / 8) & (~(1 << (7 - (hdd_pn % 8)))),
            BASE_BM_PPN + hdd_pn / 8
        );
        mem_write(
            mem_read(BASE_BM_PPN + mem_pn / 8) | (1 << (7 - (mem_pn % 8))),
            BASE_BM_PPN + mem_pn / 8
        );
        // update available pages
        set_available_mem(get_available_mem() - 1);
        set_available_hdd(get_available_hdd() + 1);
        // load data from disk
        disk_load(BASE_MEM + mem_pn * PAGE_SIZE, hdd_offset * PAGE_SIZE, PAGE_SIZE);
    } else {
        // Random choice
        srand(time(NULL));
        while (1) {
            v_address tmp_vpn = (rand() * rand()) % (2 * (NUM_PAGE_MEM + NUM_PAGE_HDD));
            unsigned int tmp_vpt_entry = vpt_get(tmp_vpn);
            p_address tmp_ppn = tmp_vpt_entry & 0x03FFFF;
            if (tmp_vpt_entry < 0xFFFFFF && tmp_ppn < NUM_PAGE_MEM) {
                mem_pn = tmp_ppn;
                unsigned int tmp_ppt_entry = ppt_get(tmp_ppn);
                // swap ppt entry
                ppt_put(tmp_ppn, ppt_entry);
                ppt_put(hdd_pn, tmp_ppt_entry);
                // write new vpt entries
                vpt_put(vpn, (vpt_entry & 0xFC0000) + tmp_ppn);
                vpt_put(tmp_vpn, (tmp_vpt_entry & 0xFC0000) + hdd_pn);
                // no need to update bitmap or available pages
                // swap data
                disk_load(BASE_MEM_SWAP, hdd_offset * PAGE_SIZE, PAGE_SIZE);
                disk_save(BASE_MEM + tmp_ppn * PAGE_SIZE, hdd_offset * PAGE_SIZE, PAGE_SIZE);
                for (int i = 0; i < PAGE_SIZE; i++) {
                    mem_write(mem_read(BASE_MEM_SWAP + i), BASE_MEM + tmp_ppn * PAGE_SIZE + i);
                }
                break;
            }
        }
    }
    return mem_pn;
}

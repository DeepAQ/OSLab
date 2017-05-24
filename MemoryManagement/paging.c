/*
 * Copyright (C) 2017 DeepAQ.
 * All Rights Reserved.
 */

#include "bottom.h"
#include "paging.h"

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
    if (vpn < 0 || vpn >= 2 * (NUM_PAGE_MEM + NUM_PAGE_HDD))
        return 0xFFFFFFFF;
    data_unit byte1 = mem_read(BASE_VPT + vpn * 3);
    if ((byte1 & 0x80) == 0)
        return 0xFFFFFFFF;
    return ((byte1 & 0x03) << 16)
            + (mem_read(BASE_VPT + vpn * 3 + 1) << 8)
            + mem_read(BASE_VPT + vpn * 3 + 2);
}

unsigned int ppt_get(p_address ppn) {
    if (ppn < 0 || ppn >= NUM_PAGE_MEM + NUM_PAGE_HDD)
        return 0xFFFFFFFF;
    return (mem_read(BASE_PPT + ppn * 3) << 16)
            + (mem_read(BASE_PPT + ppn * 3 + 1) << 8)
            + mem_read(BASE_PPT + ppn * 3 + 2);
}

void pt_put(v_address vpn, p_address ppn, m_pid_t pid, m_size_t size) {
    // write vpt
    unsigned int vpt_entry = ppn | 0x800000;
    mem_write(vpt_entry >> 16, BASE_VPT + vpn * 3);
    mem_write(vpt_entry >> 8, BASE_VPT + vpn * 3 + 1);
    mem_write(vpt_entry, BASE_VPT + vpn * 3 + 2);
    // write ppt
    unsigned int ppt_entry = (pid << 12) + size - 1;
    mem_write(ppt_entry >> 16, BASE_PPT + ppn * 3);
    mem_write(ppt_entry >> 8, BASE_PPT + ppn * 3 + 1);
    mem_write(ppt_entry, BASE_PPT + ppn * 3 + 2);
    // update bitmap
    mem_write(
        mem_read(BASE_BM_VPN + vpn / 8) | (1 << (7 - (vpn % 8))),
        BASE_BM_VPN + vpn / 8
    );
    mem_write(
        mem_read(BASE_BM_PPN + ppn / 8) | (1 << (7 - (ppn % 8))),
        BASE_BM_PPN + ppn / 8
    );
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
}

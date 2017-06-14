/*
 * Copyright (C) 2017 DeepAQ.
 * All Rights Reserved.
 */

#include "bottom.h"
#include "paging.h"
#include "call.h"

void init() {
    // clear metadata
    for (int i = 0; i < BASE_MEM; i++)
        mem_write(0, i);
    // init available
    set_available_mem(NUM_PAGE_MEM);
    set_available_hdd(NUM_PAGE_HDD);
}

int read(data_unit *data, v_address address, m_pid_t pid) {
    v_address vpn = address / PAGE_SIZE;
    unsigned int vpt_entry = vpt_get(vpn);
    if (vpt_entry > 0xFFFFFF)
        return -1;
    p_address ppn = vpt_entry & 0x03FFFF;
    unsigned int ppt_entry = ppt_get(ppn);
    m_pid_t pt_pid = ppt_entry >> 12;
    m_size_t pt_size = ppt_entry & 0x0FFF;
    if (pt_pid != pid || address - vpn * PAGE_SIZE > pt_size)
        return -1;
    if (ppn < NUM_PAGE_MEM) {
        *data = mem_read(BASE_MEM + ppn * PAGE_SIZE + address - vpn * PAGE_SIZE);
    } else {
        p_address new_ppn = hdd_swap(vpn, vpt_entry, ppt_entry);
        *data = mem_read(BASE_MEM + new_ppn * PAGE_SIZE + address - vpn * PAGE_SIZE);
    }
    return 0;
}

int write(data_unit data, v_address address, m_pid_t pid) {
    v_address vpn = address / PAGE_SIZE;
    unsigned int vpt_entry = vpt_get(vpn);
    if (vpt_entry > 0xFFFFFF)
        return -1;
    p_address ppn = vpt_entry & 0x03FFFF;
    unsigned int ppt_entry = ppt_get(ppn);
    m_pid_t pt_pid = ppt_entry >> 12;
    m_size_t pt_size = ppt_entry & 0x0FFF;
    if (pt_pid != pid || address - vpn * PAGE_SIZE > pt_size)
        return -1;
    if (ppn < NUM_PAGE_MEM) {
        mem_write(data, BASE_MEM + ppn * PAGE_SIZE + address - vpn * PAGE_SIZE);
    } else {
        p_address new_ppn = hdd_swap(vpn, vpt_entry, ppt_entry);
        mem_write(data, BASE_MEM + new_ppn * PAGE_SIZE + address - vpn * PAGE_SIZE);
    }
    return 0;
}

int allocate(v_address *address, m_size_t size, m_pid_t pid) {
    m_size_t num_p = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    // Check size
    m_size_t avl_mem = get_available_mem();
    if (avl_mem < num_p && avl_mem + get_available_hdd() < num_p)
        return -1;
    // Find and allocate pages
    v_address vpn = find_vpn(num_p);
    *address = vpn * PAGE_SIZE;
    m_size_t num_pn = 0;
    for (int i = 0; i < SIZE_BM_PPN; i++) {
        data_unit bm_byte = mem_read(BASE_BM_PPN + i);
        for (int j = 0; j < 8; j++) {
            if ((bm_byte & (1 << (7 - j))) == 0) {
                num_pn++;
                m_size_t p_size = (num_pn == num_p) ? size - (num_p - 1) * PAGE_SIZE : PAGE_SIZE;
                data_unit continuation = (num_pn < num_p) ? 1 : 0;
                pt_put(vpn + num_pn - 1, i * 8 + j, pid, p_size, continuation);
                if (num_pn >= num_p)
                    break;
            }
        }
        if (num_pn >= num_p)
            break;
    }
    return 0;
}

int free(v_address address, m_pid_t pid) {
    v_address vpn = address / PAGE_SIZE;
    unsigned int vpt_entry = vpt_get(vpn);
    if (vpt_entry > 0xFFFFFF)
        return -1;
    p_address ppn = vpt_entry & 0x03FFFF;
    unsigned int ppt_entry = ppt_get(ppn);
    m_pid_t pt_pid = ppt_entry >> 12;
    if (pt_pid != pid)
        return -1;
    while (1) {
        pt_remove(vpn);
        // Continuation
        if ((vpt_entry & 0x400000) == 0)
            break;
        vpt_entry = vpt_get(++vpn);
        if (vpt_entry > 0xFFFFFF)
            break;
    }
    return 0;
}

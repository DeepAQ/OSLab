/*
 * Copyright (C) 2017 DeepAQ.
 * All Rights Reserved.
 */

#include "bottom.h"
#include "paging.h"
#include "call.h"

void init() {
    // init bitmap
    for (int i = 0; i < SIZE_BM_VPN; i++)
        mem_write(0, BASE_BM_VPN + i);
    for (int i = 0; i < SIZE_BM_PPN; i++)
        mem_write(0, BASE_BM_PPN + i);
}

int read(data_unit *data, v_address address, m_pid_t pid) {
    v_address vpn = address / 4096;
    p_address ppn = vpt_get(vpn);
    if (ppn > 0x03FFFF)
        return -1;
    unsigned int ppt_entry = ppt_get(ppn);
    m_pid_t pt_pid = ppt_entry >> 12;
    m_size_t pt_size = ppt_entry & 0x0FFF;
    if (pt_pid != pid || address - vpn * 4096 > pt_size)
        return -1;
    if (ppn < NUM_PAGE_MEM) {
        *data = mem_read(BASE_MEM + ppn * 4096 + address - vpn * 4096);
    } else {
        // TODO swap
    }
    return 0;
}

int write(data_unit data, v_address address, m_pid_t pid) {
    v_address vpn = address / 4096;
    p_address ppn = vpt_get(vpn);
    if (ppn > 0x03FFFF)
        return -1;
    unsigned int ppt_entry = ppt_get(ppn);
    m_pid_t pt_pid = ppt_entry >> 12;
    m_size_t pt_size = ppt_entry & 0x0FFF;
    if (pt_pid != pid || address - vpn * 4096 > pt_size)
        return -1;
    if (ppn < NUM_PAGE_MEM) {
        mem_write(data, BASE_MEM + ppn * 4096 + address - vpn * 4096);
    } else {
        // TODO swap
    }
    return 0;
}

int allocate(v_address *address, m_size_t size, m_pid_t pid) {
    if (size <= 0)
        return -1;
    m_size_t num_p = size / 4096;
    if (size - num_p * 4096 > 0)
        num_p++;
    p_address ppns[num_p];
    m_size_t num_pn = 0;
    for (int i = 0; i < SIZE_BM_PPN; i++) {
        data_unit bm_byte = mem_read(BASE_BM_PPN + i);
        for (int j = 0; j < 8; j++) {
            if ((bm_byte & (1 << (7 - j))) == 0) {
                ppns[num_pn] = i * 8 + j;
                num_pn++;
                if (num_pn >= num_p)
                    break;
            }
        }
        if (num_pn >= num_p)
            break;
    }
    if (num_pn < num_p)
        return -1;
    v_address vpn = find_vpn(num_p);
    *address = vpn * 4096;
    for (int i = 0; i < num_p; i++) {
        m_size_t p_size = (i == num_p - 1) ? size - (num_p - 1) * 4096 : 4096;
        pt_put(vpn + i, ppns[i], pid, p_size);
    }
    return 0;
}

int free(v_address address, m_pid_t pid) {
    v_address vpn = address / 4096;
    p_address ppn = vpt_get(vpn);
    if (ppn > 0x03FFFF)
        return -1;
    unsigned int ppt_entry = ppt_get(ppn);
    m_pid_t pt_pid = ppt_entry >> 12;
    if (pt_pid != pid)
        return -1;
    pt_remove(vpn);
    return 0;
}

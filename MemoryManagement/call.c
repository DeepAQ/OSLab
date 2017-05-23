/*
 * Copyright (C) 2017 DeepAQ.
 * All Rights Reserved.
 */

#include "bottom.h"
#include "paging.h"
#include "call.h"

void init() {
    // init bitmap
    for (int i = 0; i < SIZE_BM_PPN; i++) {
//        mem_write(0, BASE_BM_VPN + i * 2);
//        mem_write(0, BASE_BM_VPN + i * 2 + 1);
//        mem_write(0, BASE_BM_PPN + i);
    }
}

int read(data_unit *data, v_address address, m_pid_t pid) {
    return -1;
}

int write(data_unit data, v_address address, m_pid_t pid) {
    return -1;
}

int allocate(v_address *address, m_size_t size, m_pid_t pid) {
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
        pt_put(vpn + i, ppns[i], pid);
    }
    return 0;
}

int free(v_address address, m_pid_t pid) {
    return -1;
}

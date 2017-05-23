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

void pt_put(v_address vpn, p_address ppn, m_pid_t pid) {
    unsigned int pt_entry = ppn << 16 + pid;
    mem_write(pt_entry >> 24, BASE_PT + vpn * 4);
    mem_write(pt_entry >> 16, BASE_PT + vpn * 4 + 1);
    mem_write(pt_entry >> 8, BASE_PT + vpn * 4 + 2);
    mem_write(pt_entry, BASE_PT + vpn * 4 + 3);
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
    mem_write(0, BASE_PT + vpn * 4 + 2);
    mem_write(0, BASE_PT + vpn * 4 + 3);
}

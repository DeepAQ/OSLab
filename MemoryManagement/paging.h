#ifndef PAGING_H
#define PAGING_H

#define NUM_PAGE_MEM 32393
#define NUM_PAGE_HDD 131072
#define BASE_BM_VPN 0
#define SIZE_BM_VPN 40867
#define BASE_BM_PPN 10 * 4096
#define SIZE_BM_PPN 20434
#define BASE_VPT 15 * 4096
#define BASE_PPT 255 * 4096
#define BASE_MEM 375 * 4096

v_address find_vpn(m_size_t p_num);

unsigned int vpt_get(v_address vpn);

unsigned int ppt_get(v_address vpn);

void pt_put(v_address vpn, p_address ppn, m_pid_t pid, m_size_t size);

void pt_remove(v_address vpn);

#endif // PAGING_H

#ifndef PAGING_H
#define PAGING_H

#include "type.h"

#define PAGE_SIZE 4096
#define NUM_PAGE_MEM 32392
#define NUM_PAGE_HDD 131072
#define BASE_BM_VPN 0
#define SIZE_BM_VPN 40866
#define BASE_AVL_MEM 40800
#define BASE_AVL_HDD 40802
#define BASE_BM_PPN 10 * PAGE_SIZE
#define SIZE_BM_PPN 20433
#define BASE_VPT 15 * PAGE_SIZE
#define BASE_PPT 255 * PAGE_SIZE
#define BASE_MEM_SWAP 375 * PAGE_SIZE
#define BASE_MEM 376 * PAGE_SIZE

m_size_t get_available_mem();
void set_available_mem(m_size_t size);
m_size_t get_available_hdd();
void set_available_hdd(m_size_t size);

v_address find_vpn(m_size_t p_num);
unsigned int vpt_get(v_address vpn);
unsigned int ppt_get(p_address ppn);

void pt_put(v_address vpn, p_address ppn, m_pid_t pid, m_size_t size, data_unit continuation);
void pt_remove(v_address vpn);
p_address hdd_swap(v_address vpn, unsigned int vpt_entry, unsigned int ppt_entry);

#endif // PAGING_H

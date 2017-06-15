
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
    for (PROCESS* p = proc_table; p < proc_table + NR_TASKS; p++) {
        if (p->sleep_ticks > 0) {
            p->sleep_ticks--;
        }
    }

    p_proc_ready++;
    if (p_proc_ready >= proc_table + NR_TASKS) {
        p_proc_ready = proc_table;
    }
    while (p_proc_ready->sleep_ticks > 0 || p_proc_ready->blocked == 1) {
        p_proc_ready++;
        if (p_proc_ready >= proc_table + NR_TASKS) {
            p_proc_ready = proc_table;
        }
    }
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
    return ticks;
}

PUBLIC void sys_process_sleep(int mill_seconds)
{
    p_proc_ready->sleep_ticks = mill_seconds * HZ / 1000;
    schedule();
}

PUBLIC void sys_disp_str(char* str)
{
    if (disp_pos >= 80 * 25 * 2) {
        memset(0xB8000, 0, 80 * 25 * 2);
        disp_pos = 0;
    }
    int colors[] = {9, 10, 11, 12, 13, 14, 15, 16};
    disp_color_str(str, colors[p_proc_ready->pid % 8]);
    disp_str("");
}

PUBLIC void sys_sem_p(SEMAPHORE* sem)
{
    sem->value--;
    if (sem->value < 0) {
        sem->queue[- sem->value - 1] = p_proc_ready;
        p_proc_ready->blocked = 1;
        schedule();
    }
}

PUBLIC void sys_sem_v(SEMAPHORE* sem)
{
    sem->value++;
    if (sem->value <= 0) {
        sem->queue[0]->blocked = 0;
        for (int i = 0; i < - sem->value; i++) {
            sem->queue[i] = sem->queue[i + 1];
        }
        schedule();
    }
}

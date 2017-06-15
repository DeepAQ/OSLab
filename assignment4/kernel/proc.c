
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
    PROCESS* p;
    int	 greatest_ticks = 0;

    for (p = proc_table; p < proc_table + NR_TASKS; p++) {
        if (p->sleep_ticks > 0) {
            p->sleep_ticks--;
        }
    }

    p_proc_ready++;
    if (p_proc_ready >= proc_table + NR_TASKS) {
        p_proc_ready = proc_table;
    }
    while (p_proc_ready->sleep_ticks > 0) {
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
    disp_str(str);
    if (disp_pos >= 80 * 25 * 2) {
        disp_pos = 0;
    }
}

PUBLIC void sys_sem_p()
{
    // TODO
}

PUBLIC void sys_sem_v()
{
    // TODO
}

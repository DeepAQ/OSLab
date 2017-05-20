
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
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
#include "keyboard.h"

PRIVATE char text[V_MEM_SIZE / 2];

PRIVATE void render();

/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty()
{
    // Init console
    disp_str("Loading console...\n");
    memset(text, 0, V_MEM_SIZE / 2);
    render();
    // Poll keyboard events
    while (1) {
        keyboard_read();
    }
}

PRIVATE void set_cursor(unsigned int position)
{
    disable_int();
    out_byte(CRTC_ADDR_REG, CURSOR_H);
    out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, CURSOR_L);
    out_byte(CRTC_DATA_REG, position & 0xFF);
    enable_int();
}

PRIVATE void set_cursor2(unsigned int position)
{
    disable_int();
    out_byte(CRTC_ADDR_REG, CURSOR_H);
    out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, CURSOR_L);
    out_byte(CRTC_DATA_REG, position & 0xFF);
    enable_int();
}

PRIVATE void render()
{
    memset(V_MEM_BASE, 0, V_MEM_SIZE);
    // TODO render
}

/*======================================================================*
                in_process
 *======================================================================*/
PUBLIC void in_process(u32 key)
{
    char output[2] = {'\0', '\0'};

    if (!(key & FLAG_EXT)) {
        output[0] = key & 0xFF;
        disp_str(output);
        set_cursor(disp_pos / 2);
    }
    else {
        int raw_code = key & MASK_RAW;
        switch(raw_code) {
        case UP:
            if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
                disable_int();
                out_byte(CRTC_ADDR_REG, START_ADDR_H);
                out_byte(CRTC_DATA_REG, ((80*15) >> 8) & 0xFF);
                out_byte(CRTC_ADDR_REG, START_ADDR_L);
                out_byte(CRTC_DATA_REG, (80*15) & 0xFF);
                enable_int();
            }
            break;
        case DOWN:
            if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
                /* Shift+Down, do nothing */
            }
            break;
        default:
            break;
        }
    }
}


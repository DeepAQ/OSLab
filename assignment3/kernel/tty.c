
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

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define TEXT_SIZE V_MEM_SIZE / 2

PRIVATE char text[TEXT_SIZE];
PRIVATE int text_pos;

PRIVATE void render();

/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty()
{
    // Init console
    disp_str("Loading console...\n");
    memset(text, 0, TEXT_SIZE);
    text_pos = 0;
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

PRIVATE void render()
{
    u8 *vmem = V_MEM_BASE;
    // clear vmem
    for (int i = 0; i < V_MEM_SIZE / 2; i++) {
        vmem[i * 2] = 0;
        vmem[i * 2 + 1] = 0x0f;
    }
    // start rendering
    int row = 0, col = 0;
    for (int i = 0; i < TEXT_SIZE; i++) {
        if (text[i] == 0) break;
        switch (text[i]) {
        case '\n':
            row++;
            col = 0;
            break;
        case '\t':
            // TODO
            break;
        default:
            vmem[(row * SCREEN_WIDTH + col) * 2] = text[i];
            vmem[(row * SCREEN_WIDTH + col) * 2 + 1] = 0x0f;
            col++;
            break;
        }
        if (col >= SCREEN_WIDTH) {
            row++;
            col = col % SCREEN_WIDTH;
        }
        row %= SCREEN_HEIGHT;
    }
    set_cursor(row * SCREEN_WIDTH + col);
}

/*======================================================================*
                in_process
 *======================================================================*/
PUBLIC void in_process(u32 key)
{
    if (!(key & FLAG_EXT)) {
        text[text_pos++] = key & 0xFF;
        render();
    }
    else {
        int raw_code = key & MASK_RAW;
        switch(raw_code) {
        case ENTER:
            text[text_pos++] = '\n';
            render();
            break;
        case TAB:
            text[text_pos++] = '\t';
            render();
            break;
        }
    }
}


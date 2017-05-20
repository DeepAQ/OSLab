
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

PRIVATE u8 *vmem = V_MEM_BASE;
PRIVATE char text[TEXT_SIZE];
PRIVATE int text_pos;
PRIVATE int state;
PRIVATE char pattern[TEXT_SIZE];
PRIVATE int pattern_pos;
PRIVATE int cursor_pos;

PRIVATE void render();

/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty()
{
    // Init console
    disp_str("Loading console...\n");
    memset(text, 0, TEXT_SIZE);
    text_pos = state = 0;
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
    cursor_pos = position;
    enable_int();
}

PRIVATE void render()
{
    // clear vmem
    for (int i = 0; i < V_MEM_SIZE / 2; i++) {
        vmem[i * 2] = 0;
        vmem[i * 2 + 1] = 0x0f;
    }
    // start rendering
    int row = 0, col = 0;
    for (int i = 0; i < text_pos; i++) {
        if (text[i] == 0) break;
        switch (text[i]) {
        case '\n':
            row = (row + 1) % SCREEN_HEIGHT;
            col = 0;
            break;
        case '\t':
            col += 4 - (col % 4);
            break;
        default:
            vmem[(row * SCREEN_WIDTH + col) * 2] = text[i];
            if (state == 2 && text[i] == pattern[0]) {
                int j;
                for (j = 1; j < pattern_pos && i + j < text_pos && text[i + j] == pattern[j]; j++);
                if (j == pattern_pos)
                    for (; j > 0; j--) vmem[(row * SCREEN_WIDTH + col + j) * 2 - 1] = 0x0e;
            }
            col++;
            break;
        }
        if (col >= SCREEN_WIDTH) {
            row = (row + 1) % SCREEN_HEIGHT;
            col = col % SCREEN_WIDTH;
        }
    }
    for (int i = 0; state == 2 && i < pattern_pos; i++) {
        vmem[(row * SCREEN_WIDTH + col) * 2] = pattern[i];
        vmem[(row * SCREEN_WIDTH + col) * 2 + 1] = 0x0e;
        col++;
    }
    set_cursor(row * SCREEN_WIDTH + col);
}

/*======================================================================*
                in_process
 *======================================================================*/
PUBLIC void in_process(u32 key)
{
    if (state == 0) {
        if (!(key & FLAG_EXT)) {
            text[text_pos++] = key & 0xFF;
            render();
        } else {
            switch(key & MASK_RAW) {
            case ENTER:
                text[text_pos++] = '\n';
                render();
                break;
            case TAB:
                text[text_pos++] = '\t';
                render();
                break;
            case BACKSPACE:
                if (text_pos > 0) {
                    text[--text_pos] = 0;
                    render();
                }
                break;
            case ESC:
                memset(pattern, 0, TEXT_SIZE);
                pattern_pos = 0;
                state = 1;
            }
        }
        text_pos %= TEXT_SIZE;
    } else if (state == 1) {
        if (!(key & FLAG_EXT)) {
            pattern[pattern_pos++] = key & 0xFF;
            vmem[cursor_pos * 2] = key & 0xFF;
            vmem[cursor_pos * 2 + 1] = 0x0e;
            set_cursor(++cursor_pos);
        } else {
            switch (key & MASK_RAW) {
            case ENTER:
                state = 2;
                render();
                break;
            case ESC:
                state = 0;
                render();
            }
        }
    } else if (state == 2 && (key & FLAG_EXT) && ((key & MASK_RAW) == ESC)) {
        state = 0;
        render();
    }
}

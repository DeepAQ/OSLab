
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
                                                  Monkey Liang, 2017
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
PRIVATE u8 vmem_buf[V_MEM_SIZE];

PRIVATE char text[TEXT_SIZE];
PRIVATE int text_pos;
PRIVATE char state;
PRIVATE char pattern[TEXT_SIZE];
PRIVATE int pattern_pos;
PRIVATE int cursor_pos;
PRIVATE char caps_lock;

PRIVATE void render();

PUBLIC void task_tty()
{
    // Init console
    disp_str("Loading console...\n");
    memset(text, 0, TEXT_SIZE);
    text_pos = state = caps_lock = 0;
    render();
    // Poll keyboard events
    while (1) {
        keyboard_read();
    }
}

PUBLIC void task_clear()
{
    while (1) {
        milli_delay(200000);
        if (state == 0) {
            memset(text, 0, TEXT_SIZE);
            text_pos = 0;
            render();
        }
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
    // clear vmem buffer
    for (int i = 0; i < V_MEM_SIZE / 2; i++) {
        vmem_buf[i * 2] = 0;
        vmem_buf[i * 2 + 1] = 0x0f;
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
            vmem_buf[(row * SCREEN_WIDTH + col) * 2] = text[i];
            if (state == 2 && text[i] == pattern[0]) {
                int j;
                for (j = 1; j < pattern_pos && i + j < text_pos && text[i + j] == pattern[j]; j++);
                if (j == pattern_pos)
                    for (; j > 0; j--) vmem_buf[(row * SCREEN_WIDTH + col + j) * 2 - 1] = 0x0e;
            }
            col++;
        }
        if (col >= SCREEN_WIDTH) {
            row = (row + 1) % SCREEN_HEIGHT;
            col = col % SCREEN_WIDTH;
        }
    }
    for (int i = 0; state > 0 && i < pattern_pos; i++) {
        vmem_buf[(row * SCREEN_WIDTH + col) * 2] = pattern[i];
        vmem_buf[(row * SCREEN_WIDTH + col) * 2 + 1] = 0x0e;
        col++;
    }
    // apply render results to display
    memcpy(vmem, vmem_buf, V_MEM_SIZE);
    set_cursor(row * SCREEN_WIDTH + col);
}

PRIVATE char get_char_from_key(u32 key) {
    char c = key & 0xFF;
    if (caps_lock) {
        if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
        else if (c >= 'a' && c <= 'z') c += 'A' - 'a';
    }
    return c;
}

PUBLIC void in_process(u32 key)
{
    if ((key & FLAG_EXT) && (key & MASK_RAW) == CAPS_LOCK) {
        caps_lock = 1 - caps_lock;
        // set LEDs
        while (in_byte(KB_CMD) & 0x02);
        out_byte(KB_DATA, 0xed);
        while (in_byte(KB_DATA) != 0xfa && in_byte(KB_CMD) & 0x02);
        out_byte(KB_DATA, caps_lock << 2);
        while (in_byte(KB_DATA) != 0xfa);
    } else if (state == 0) {
        if (!(key & FLAG_EXT)) {
            text[text_pos++] = get_char_from_key(key);
        } else {
            switch(key & MASK_RAW) {
            case ENTER:
                text[text_pos++] = '\n';
                break;
            case TAB:
                text[text_pos++] = '\t';
                break;
            case BACKSPACE:
                if (text_pos > 0) text[--text_pos] = 0;
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
            pattern[pattern_pos++] = get_char_from_key(key);
        } else {
            switch (key & MASK_RAW) {
            case ENTER:
                state = 2;
                break;
            case BACKSPACE:
                if (pattern_pos > 0) pattern[--pattern_pos] = 0;
                break;
            case ESC:
                state = 0;
            }
        }
    } else if (state == 2 && (key & FLAG_EXT) && ((key & MASK_RAW) == ESC)) {
        state = 0;
    }
    render();
}

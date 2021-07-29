/*
 * hpsdr_debug.c
 *
 *  Created on: 18 jul. 2021
 *      Author: Emiliano Gonzalez LU3VEA (lu3vea @ gmail . com))
 */

#include "hpsdr_debug.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

static int hpsdr_debug_level = 0;
int hpsdr_debug_id = 1;

void hpsdr_dbg_setlevel(int Level) {
    hpsdr_debug_level = Level;
}

int hpsdr_dbg_getlevel(void) {
    return hpsdr_debug_level;
}

void hpsdr_dbg_printf(int Level, const char *fmt, ...) {
    if (Level <= hpsdr_debug_level) {
        bool debug_id_m = false;
        va_list args;
        va_start(args, fmt);
        if (fmt[0] == '<') {
            --hpsdr_debug_id;
            debug_id_m = true;
            fprintf(stderr, "%*c(%d)", hpsdr_debug_id, ' ', hpsdr_debug_id);
        }
        if (fmt[0] == '>') {
            debug_id_m = true;
            fprintf(stderr, "%*c(%d)", hpsdr_debug_id, ' ', hpsdr_debug_id);
            ++hpsdr_debug_id;
        }
        if (!debug_id_m)
            fprintf(stderr, "%*c", hpsdr_debug_id, ' ');
        vfprintf(stderr, fmt, args);
        va_end(args);
        debug_id_m = false;
    }
}

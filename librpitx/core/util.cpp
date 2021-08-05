/*
 Copyright (C) 2018  Evariste COURJAUD F5OEO

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "util.hpp"

static int librpitx_debug_level = 0;
int librpitx_debug_id = 1;

void librpitx_dbg_setlevel(int Level) {
    librpitx_debug_level = Level;
}

int librpitx_dbg_getlevel() {
    return librpitx_debug_level;
}

void librpitx_dbg_printf(int Level, const char *fmt, ...) {
    if (Level <= librpitx_debug_level) {
        bool debug_id_m = false;
        va_list args;
        va_start(args, fmt);
        if (fmt[0] == '<') {
            --librpitx_debug_id;
            debug_id_m = true;
            fprintf(stderr, "[rpitx] %*c(%d)", librpitx_debug_id, ' ', librpitx_debug_id);
        }
        if (fmt[0] == '>') {
            debug_id_m = true;
            fprintf(stderr, "[rpitx] %*c(%d)", librpitx_debug_id, ' ', librpitx_debug_id);
            ++librpitx_debug_id;
        }
        if (!debug_id_m)
            fprintf(stderr, "[rpitx] %*c", librpitx_debug_id, ' ');
        vfprintf(stderr, fmt, args);
        va_end(args);
        debug_id_m = false;
    }
}

/*
 * hpsdr_debug.h
 *
 *  Created on: 18 jul. 2021
 *      Author: Emiliano Gonzalez LU3VEA (lu3vea @ gmail . com))
 */

#ifndef HPSDR_DEBUG_H_
#define HPSDR_DEBUG_H_

void hpsdr_dbg_setlevel(int Level);
 int hpsdr_dbg_getlevel(void);
void hpsdr_dbg_printf(int Level, const char *fmt, ...);

#endif

/*
	Copcake script interpreter.
    Copyright C 2018  bitrate16 bitrate16@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    at your option any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
/*
 * Idk, i keep it just for fun, lol.
 */

#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <stdio.h>

// _______________
// D E B U G G E R

int Kit_bug(int, const char*);

int Kit_bug(int);

int Kit_error(int, const char*);

int Kit_debug(int, const char*);

int Kit_debug(int, int);

int Kit_debug(int);

// #define DEBUG_ENABLED

#ifdef DEBUG_ENABLED

#define DEBUG(x) Kit_debug(__LINE__, x);

#else

#define DEBUG(x)

#endif

#define DEBUG_IDENT(x) int __idn = x; while (__idn--) putchar(' ');

#endif

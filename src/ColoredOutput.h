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
 * Allows changing color output mode with macro-inflates.
 * Used: https://gist.github.com/chrisopedia/8754917
 */

#ifndef COLORED_OUTPUT_H
#define COLORED_OUTPUT_H

#define CRESET        0

#define CREGULAR      0
#define CBOLD         1
#define CUNDERLINE    2
#define CBACKGROUND   3

#define CBLACK       30
#define CRED         31
#define CGREEN       32
#define CYELLOW      33
#define CBLUE        34
#define CMAGENTA     35
#define CCYAN        36
#define CWHITE       37

#define CHIGHBLACK   90
#define CHIGHRED     91
#define CHIGHGREEN   92
#define CHIGHYELLOW  93
#define CHIGHBLUE    94
#define CHIGHMAGENTA 95
#define CHIGHCYAN    96
#define CHIGHWHITE   97

#define creset                 printf("\x1B[0m");
#define csetcolor(color)       printf("\x1B[%dm", color );
#define csetbold(color)        printf("\x1B[1;%dm", color );
#define csetunderline(color)   printf("\x1B[4;%dm", color );
#define csetbackground(color)  printf("\x1B[%dm", color + 10 );

#define cred                   printf("\x1B[31m");
#define cgreen                 printf("\x1B[32m");
#define cblue                  printf("\x1B[34m");
#define cwhite                 printf("\x1B[37m");

#define chighred               printf("\x1B[91m");
#define chighgreen             printf("\x1B[92m");
#define chighblue              printf("\x1B[94m");
#define chighwhite             printf("\x1B[97m");

#if __linux__ 

#define cbegin

#else
	
#define cbegin     system("");

#endif

#endif
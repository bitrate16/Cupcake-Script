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
 * Utils used on debug state of developing parser/TokenStream/ASTExecuter.
 */


#include <stdio.h>

int Kit_bug(int lineno, const char *message) {
	printf("Bug at %d : %s\n", lineno, message);
	return 0;
};

int Kit_bug(int lineno) {
	printf("Bug at %d\n", lineno);
	return 0;
};

int Kit_error(int lineno, const char *message) {
	printf("Error at %d : %s\n", lineno, message);
	return 0;
};

int Kit_debug(int lineno, const char *message) {
	printf("debug %d : %s\n", lineno, message);
	return 1;
};

int Kit_debug(int lineno, int num) {
	printf("debug %d : %d\n", lineno, num);
	return 1;
};

int Kit_debug(int lineno) {
	printf("debug %d\n", lineno);
	return 1;
};
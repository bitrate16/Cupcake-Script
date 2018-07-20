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
 * Defines volatile state for determine if program change it's state :/
 */

#ifndef EXEC_STATE
#define EXEC_STATE

// set to 1 if program handles SIGINT
extern volatile bool _global_int_state;
// set to 1 if program catches unresolvable error
extern volatile bool _global_error_state;
// set to 0 when execution is stopped
extern volatile bool _global_exec_state;
// Exit code of the program
extern volatile bool _global_exit_code;

void _interpreter_exit(int exit_code);

void _interpreter_exit();

void _interpreter_error();

void _interpreter_int();

#endif
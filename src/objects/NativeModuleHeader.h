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
 * Header defines prototypes for native modules functions
 */

#ifndef NATIVE_MODULE_HEADER_H
#define NATIVE_MODULE_HEADER_H

#include "Scope.h"
#include "VirtualObject.h"

// Prototype for onLoad function
// onLoad(scope, argc, args);
typedef void (*_native_onload_func) (Scope*, int, VirtualObject**);

// Prototype for onUnload function
typedef void (*_native_onunload_func) ();

// Template for function:
// 
// extern "C" void func() {
//     ...
// };

#endif
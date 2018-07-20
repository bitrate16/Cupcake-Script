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
 * Utility file.
 * Header included in main program file. 
 * Helps define all fields/types used by interpreter.
 */

#ifndef DEFAULT_OBJECT_DEFINE_UTIL
#define DEFAULT_OBJECT_DEFINE_UTIL

#include "objects/Scope.h"

extern void defineTypes(Scope *root_scope);

#endif
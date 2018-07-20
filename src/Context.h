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
 * Context used for storing information about 
 * current execution state of the program node.
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include "GarbageCollector.h"

struct Scope;
struct FileUrl;
struct ASTExecuter;

// Structure for storing executable context
struct Context : GC_Object {
	// Root scope to what this contest is attached
	Scope *scope;
	// Absolute path to script directory
	FileUrl *script_dir_path;
	// Absolute path to script file
	FileUrl *script_file_path;
	// Instance of ASTExecuter attached
	ASTExecuter *executer;
	
	Context();
	
	~Context();
};

#endif
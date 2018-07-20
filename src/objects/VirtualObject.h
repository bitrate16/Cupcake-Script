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
 * This class defines core representation 
 * of all objects in the Cupcake VM.
 * Core type VirtualObject can be extended 
 * for used needed functionality.
 */

#ifndef VIRTUAL_OBJECT_H
#define VIRTUAL_OBJECT_H

#include "../TokenNamespace.h"
#include "../string.h"
#include "../GarbageCollector.h"

struct Scope;

struct VirtualObject : GC_Object {
	
	// Count of variables pointing to this object
	// int  pointers     = 0;
	int  type         = NONE;

// Don't look down.
//                GC_chain->deleted_ptr = 1
//               /
//        delete -> called by stack dispose tools
//       /
// Object
//       \
//        GC_dispose -> delete this
//                  \
//                   called by GC_collect/GC_dispose
// You lost.
	
	virtual VirtualObject *get(Scope *scope, string *name);
	
	virtual void put(Scope *scope, string *name, VirtualObject *value);
	
	virtual void remove(Scope *scope, string *name);
	
	virtual bool contains(Scope *scope, string *name);
	
	virtual VirtualObject *call(Scope *scope, int argc, VirtualObject **args);
};

#endif
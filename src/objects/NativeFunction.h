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
 * This class defines representation of 
 * bridge between native functions and Cupcake VM.
 * NativeFunction allows calling functions with 
 * prototype of handler_function.
 */

#ifndef NATIVE_FUNCTION_H
#define NATIVE_FUNCTION_H

#include "Object.h"
#include "Scope.h"
#include "TreeObjectMap.h"

#include "../TokenNamespace.h"
#include "../string.h"

// NativeFunction prototype's prototype	
struct NativeFunctionPrototype : Object {
	NativeFunctionPrototype();
	void finalize(void);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	VirtualObject *call(Scope*, int, VirtualObject**);
	void mark(void);
};

// NativeFunction type's prototype

typedef VirtualObject* (*handler_function) (Scope*, int, VirtualObject**);

struct NativeFunction : VirtualObject {
	handler_function handler;
	NativeFunction(handler_function handler);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	VirtualObject *call(Scope*, int, VirtualObject**);
};

// Called on start. Defines Scope prototype & type. Insert into given scope all default operations & fields
void define_native_function(Scope*);

extern NativeFunctionPrototype *native_function_prototype;

#endif
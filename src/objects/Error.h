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
 * standard throwable Error in Cupcake VM.
 */

#ifndef ERROR_H
#define ERROR_H

#include "Scope.h"
#include "StringType.h"
#include "Object.h"

#include "../string.h"

// Error prototype's prototype	
struct ErrorPrototype : Object {
	ErrorPrototype();
	void finalize(void);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	VirtualObject *call(Scope*, int, VirtualObject**);
	void mark(void);
};

// Error type's prototype
struct Error : Object {
	Array *stacktrace;
	Error(Array *stacktrace, String *message);
	Error(Array *stacktrace);
	void finalize(void);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	VirtualObject *call(Scope*, int, VirtualObject**);
	void mark(void);
	
	static Error *newInstance(Scope*, string message);
	static Error *newInstance(Scope*, string *message);
	static Error *newInstance(Scope*, const char *message);
	
	void print();
};

// Called on start. Defines error prototype & type
void define_error(Scope*);

extern ErrorPrototype *error_prototype;

#endif
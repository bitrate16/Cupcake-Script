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
 * This class defines representation of Cupcake's String type. 
 * Strings uses optimization for functions like substring/trim 
 * by referring them to the parent string object.
 */

#ifndef STRING_TYPE_H
#define STRING_TYPE_H

#include "Object.h"
#include "Scope.h"
#include "TreeObjectMap.h"

#include "../TokenNamespace.h"
#include "../string.h"

// String prototype's prototype	
struct StringPrototype : Object {
	StringPrototype();
	void finalize(void);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	VirtualObject *call(Scope*, int, VirtualObject**);
	void mark(void);
};

// String type's prototype
struct String : VirtualObject {
	// Parent string used for storing
	// value without memory loss.
	// Like:
	// my_str = 'abcdefg';
	// substr = my_str.subString(1, 3);
	// > substr = 'bc'
	// >> substr.parent = my_str
	// >> substr.offset = 1;
	// >> substr.length = 2;
	String *parent;
	int     offset;
	int     length;
	
	// Used for storing self value
	string *value;
	String();
	String(string);
	String(string*);
	String(const char*);
	// parent[offset, offset + length]
	String(String*, int, int);
	// parent[offset, parent.length]
	String(String*, int);
	void finalize(void);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	VirtualObject *call(Scope*, int, VirtualObject**);
	void mark(void);
	
	// Useful methods
	// Returns length of buffer, 
	// attached to this string.
	// Does not include offset:
	// stringLength = source.length - offset
	int         stringLength();
	// Returns offset of attached buffer
	int         stringOffset();
	// Retuns attached buffer string
	string      *stringValue();
	// Parse positive integer from string, -1 else.
	int     parsePositiveInt();
	// Returns character at specified position.
	// -1 if out of bounds.
	int         stringCharAt(int index);
};

// Called on start. Defines string prototype & type
void define_string(Scope*);

extern StringPrototype *string_prototype;

#endif
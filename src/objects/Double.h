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
 * standard Double in Cupcake VM.
 */

#ifndef DOUBLE_H
#define DOUBLE_H

#include "Object.h"
#include "Scope.h"
#include "TreeObjectMap.h"

#include "../TokenNamespace.h"
#include "../string.h"

// Double prototype's prototype	
struct DoublePrototype : Object {
	DoublePrototype();
	void finalize(void);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	VirtualObject *call(Scope*, int, VirtualObject**);
	void mark(void);
};

// Double type's prototype
struct Double : VirtualObject {
	double value;
	Double();
	Double(double);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	VirtualObject *call(Scope*, int, VirtualObject**);
};

// Called on start. Defines double prototype & type
void define_double(Scope*);

extern DoublePrototype *double_prototype;

#endif
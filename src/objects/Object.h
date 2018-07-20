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
 * standard Object in Cupcake VM.
 */

#ifndef OBJECT_H
#define OBJECT_H

#include "Scope.h"
#include "ObjectConverter.h"
#include "TreeObjectMap.h"

#include "../TokenNamespace.h"
#include "../string.h"

// Object prototype's prototype	
struct ObjectPrototype : VirtualObject {
	TreeObjectMap *table;
	ObjectPrototype();
	void finalize(void);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	VirtualObject *call(Scope*, int, VirtualObject**);
	void mark(void);
};

// Object type's prototype
struct Object : VirtualObject {//, IntCastObject, StringCastObject, DoubleCastObject {
	TreeObjectMap *table;
	Object();
	Object(TreeObjectMap *table);
	virtual void finalize(void);
	virtual VirtualObject *get(Scope*, string*);
	virtual void put(Scope*, string*, VirtualObject*);
	virtual void remove(Scope*, string*);
	virtual bool contains(Scope*, string*);
	virtual VirtualObject *call(Scope*, int, VirtualObject**);
	virtual void mark(void);
	virtual string toString();
	virtual long toInt();
	virtual double toDouble();
};

// Called on start. Defines object prototype & type
void define_object(Scope*);

extern ObjectPrototype *object_prototype;

#endif
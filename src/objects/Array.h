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
 * Array.
 */

#ifndef ARRAY_H // Array.
#define ARRAY_H

#include "Object.h" // <-- That's not array.
#include "Scope.h"

#include "../TokenNamespace.h"
#include "../VectorArray.h"
#include "../string.h"

// Object prototype's prototype	
struct ArrayPrototype : Object {
	ArrayPrototype();
	void finalize(void);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	VirtualObject *call(Scope*, int, VirtualObject**);
	void mark(void);
};

// Object type's prototype
struct Array : Object, IntCastObject, StringCastObject, DoubleCastObject {
	VectorArray<VirtualObject> *array; // <-- Array here too.
	Array(); // <-- Array
	Array(VectorArray<VirtualObject>*); // <-- Another Array
	void finalize(void);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	VirtualObject *call(Scope*, int, VirtualObject**);
	void mark(void);
	
	void push(VirtualObject*);
	
	virtual string toString();
	virtual long toInt();
	virtual double toDouble();
};

// Called on start. Defines array prototype & type
void define_array(Scope*);

extern ArrayPrototype *array_prototype;

//                               ,-""   `.
//                             ,'  _   e )`-._           <-- Duck.
//                            /  ,' `-._<.===-'      
//                           /  /
//                          /  ;
//              _.--.__    /   ;
// (`._    _.-""       "--'    |
// <_  `-""                     \
//  <`-                          :
//   (__   <__.                  ;
//     `-.   '-.__.      _.'    /
//        \      `-.__,-'    _,'
//         `._    ,    /__,-'
//            ""._\__,'< <____
//                 | |  `----.`.
//                 | |        \ `.
//                 ; |___      \-``
//                 \   --<
//                  `.`.<
//                    `-'

#endif // Array

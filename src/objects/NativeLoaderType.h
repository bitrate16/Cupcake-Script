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
 * This classes allows loading/unloading dynamic linked modules.
 */

#ifndef NATIVE_LOADER_TYPE_H
#define NATIVE_LOADER_TYPE_H

#include "cstdlib"

#include "Object.h"
#include "Scope.h"
#include "TreeObjectMap.h"
#include "ObjectConverter.h"

#include "../TokenNamespace.h"
#include "../VectorArray.h"
#include "../string.h"

struct NativeModule {
	string file;
	void *module;
};

// Null type's prototype
struct NativeLoader : Object, StringCastObject, IntCastObject, DoubleCastObject {
	NativeLoader();
	
	long toInt();
	double toDouble();
	string toString();
	
	// Retuns string result of load procedure. 
	// Empty string = OK
	string loadModule(Scope*, string, int, VirtualObject**);
	
	void finalize();
};

// Called on start. Defines native_loader object
void define_native_loader(Scope*);

// Pointer to array of loaded modules.
// Initialized once when NativeLoader object is initially created.
extern VectorArray<NativeModule> *loaded_modules;
extern NativeLoader *native_loader;

// Called on program exit.
// Unloading modules on garbage collecting is unsafe by the 
// reason of deleting types that are still in use.
extern void unload_loaded_modules();

#endif
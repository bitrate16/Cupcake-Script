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


#include "Scope.h"
#include "Null.h"
#include "Undefined.h"
#include "NativeFunction.h"
#include "Boolean.h"
#include "Integer.h"
#include "StringType.h"

#include "../string.h"

NativeFunctionPrototype *native_function_prototype = NULL;

// NativeFunction type
NativeFunction::NativeFunction(handler_function handler) {
	this->handler = handler;
	this->type    = NATIVE_FUNCTION;
};

VirtualObject *NativeFunction::get(Scope *scope, string *name) {
	if (*name == "__address")
		return new Integer(*((int*) &handler));
	
	return native_function_prototype->table->get(*name);
};

void NativeFunction::put(Scope *scope, string *name, VirtualObject *value) {};

void NativeFunction::remove(Scope *scope, string *name) {};

bool NativeFunction::contains(Scope *scope, string *name) {
	if (*name == "__address")
		return 1;
	return 0;
};

VirtualObject *NativeFunction::call(Scope *scope, int argc, VirtualObject **args) {
	try {
		if (this->handler)
			return handler(scope, argc, args);
	} catch(...) {
		scope->context->executer->raiseError("Unhandled Exception");
	}
	
	return new Undefined;
};

// NativeFunction prototype	
NativeFunctionPrototype::NativeFunctionPrototype() {		
	// table = new TreeObjectMap;
	type  = NATIVE_FUNCTION_PROTOTYPE;
};

void NativeFunctionPrototype::finalize(void) {
	table->finalize();
};

VirtualObject *NativeFunctionPrototype::get(Scope *scope, string *name) {
	return table->get(*name);
};

void NativeFunctionPrototype::put(Scope *scope, string *name, VirtualObject *value) {
	table->put(*name, value);
};

void NativeFunctionPrototype::remove(Scope *scope, string *name) {
	table->remove(*name);
};

bool NativeFunctionPrototype::contains(Scope *scope, string *name) {
	return table->contains(*name);
};

VirtualObject *NativeFunctionPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	return new Undefined;
};

void NativeFunctionPrototype::mark(void) {
	if (gc_reachable)
		return;
	gc_reachable = 1;
	table->mark();
};

// Operators

static VirtualObject* operator_eq(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 1)
		return new Undefined;
	
	return new Boolean(args[0]->type == NATIVE_FUNCTION && args[1]->type == NATIVE_FUNCTION ? ((NativeFunction*) args[0])->handler == ((NativeFunction*) args[1])->handler : args[0] == args[1]);
};

static VirtualObject* operator_neq(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 1)
		return new Undefined;
	
	return new Boolean(args[0]->type == NATIVE_FUNCTION && args[1]->type == NATIVE_FUNCTION ? ((NativeFunction*) args[0])->handler != ((NativeFunction*) args[1])->handler : args[0] != args[1]);
};

// Called on start. Defines NativeFunction prototype & type
void define_native_function(Scope *scope) {
	native_function_prototype = new NativeFunctionPrototype();
	scope->table->put(string("NativeFunction"), native_function_prototype);
	
	native_function_prototype->table->put(string("__typename"),   new String("NativeFunction"));
	native_function_prototype->table->put(string("__operator=="), new NativeFunction(&operator_eq));
	native_function_prototype->table->put(string("__operator!="), new NativeFunction(&operator_neq));
};


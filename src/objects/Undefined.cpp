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
#include "StringType.h"

#include "../string.h"

UndefinedPrototype *undefined_prototype = NULL;

// Undefined type
Undefined::Undefined() {
	type = UNDEFINED;
};

VirtualObject *Undefined::get(Scope *scope, string *name) {
	VirtualObject *proto_obj = undefined_prototype->table->get(*name);
	if (scope && !proto_obj)
		scope->context->executer->raiseError("undefined reference");
	return proto_obj;
};

void Undefined::put(Scope *scope, string *name, VirtualObject *value) {
	if (scope)
		scope->context->executer->raiseError("undefined reference");
};

void Undefined::remove(Scope *scope, string *name) {
	if (scope)
		scope->context->executer->raiseError("undefined reference");
};

bool Undefined::contains(Scope *scope, string *name) {
	return undefined_prototype->table->contains(*name);
};

VirtualObject *Undefined::call(Scope *scope, int argc, VirtualObject **args) {
	if (scope)
		scope->context->executer->raiseError("undefined reference");
	return NULL;
};

// Undefined prototype	
UndefinedPrototype::UndefinedPrototype() {		
	// table = new TreeObjectMap;
	type  = UNDEFINED_PROTOTYPE;
};

void UndefinedPrototype::finalize(void) {
	table->finalize();
};

VirtualObject *UndefinedPrototype::get(Scope *scope, string *name) {
	return table->get(*name);
};

void UndefinedPrototype::put(Scope *scope, string *name, VirtualObject *value) {
	table->put(*name, value);
};

void UndefinedPrototype::remove(Scope *scope, string *name) {
	table->remove(*name);
};

bool UndefinedPrototype::contains(Scope *scope, string *name) {
	return table->contains(*name);
};

VirtualObject *UndefinedPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	return new Null;
};

void UndefinedPrototype::mark(void) {
	if (gc_reachable)
		return;
	gc_reachable = 1;
	table->mark();
};

// Operators

// ==
static VirtualObject* operator_eq(Scope *scope, int argc, VirtualObject **args) {
	if (argc <= 1)
		return new Undefined;
	
	return new Boolean(args[1]->type == UNDEFINED);
};

// !=
static VirtualObject* operator_neq(Scope *scope, int argc, VirtualObject **args) {
	if (argc <= 1)
		return new Undefined;
	
	return new Boolean(args[1]->type != UNDEFINED);
};

// ||
static VirtualObject* operator_or(Scope *scope, int argc, VirtualObject **args) {
	if (argc <= 1)
		return new Undefined;
	
	return args[1];
};

// Called on start. Defines undefined prototype & type
void define_undefined(Scope *scope) {
	undefined_prototype = new UndefinedPrototype();
	scope->table->put(string("Undefined"), undefined_prototype);
	
	undefined_prototype->table->put(string("__typename"),   new String("Undefined"));
	undefined_prototype->table->put(string("__operator=="), new NativeFunction(&operator_eq));
	undefined_prototype->table->put(string("__operator!="), new NativeFunction(&operator_neq));
	undefined_prototype->table->put(string("__operator||"), new NativeFunction(&operator_or));
};


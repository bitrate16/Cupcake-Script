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

NullPrototype *null_prototype = NULL;

// Null type
Null::Null() {
	type = TNULL;
};

VirtualObject *Null::get(Scope *scope, string *name) {
	VirtualObject *proto_obj = null_prototype->table->get(*name);
	if (scope && !proto_obj)
		scope->context->executer->raiseError("null reference");
	return proto_obj;
};

void Null::put(Scope *scope, string *name, VirtualObject *value) {
	if (scope)
		scope->context->executer->raiseError("null reference");
};

void Null::remove(Scope *scope, string *name) {
	if (scope)
		scope->context->executer->raiseError("null reference");
};

bool Null::contains(Scope *scope, string *name) {
	return null_prototype->table->contains(*name);
};

VirtualObject *Null::call(Scope *scope, int argc, VirtualObject **args) {
	if (scope)
		scope->context->executer->raiseError("null reference");
	return NULL;
};

// Null prototype	
NullPrototype::NullPrototype() {		
	// table = new TreeObjectMap;
	type  = NULL_PROTOTYPE;
};

void NullPrototype::finalize(void) {
	table->finalize();
};

VirtualObject *NullPrototype::get(Scope *scope, string *name) {
	return table->get(*name);
};

void NullPrototype::put(Scope *scope, string *name, VirtualObject *value) {
	table->put(*name, value);
};

void NullPrototype::remove(Scope *scope, string *name) {
	table->remove(*name);
};

bool NullPrototype::contains(Scope *scope, string *name) {
	return table->contains(*name);
};

VirtualObject *NullPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	return new Null;
};

void NullPrototype::mark(void) {
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
	
	return new Boolean(args[1]->type == TNULL);
};

// !=
static VirtualObject* operator_neq(Scope *scope, int argc, VirtualObject **args) {
	if (argc <= 1)
		return new Undefined;
	
	return new Boolean(args[1]->type != TNULL);
};

// ||
static VirtualObject* operator_or(Scope *scope, int argc, VirtualObject **args) {
	if (argc <= 1)
		return new Undefined;
	
	return args[1];
};

// Called on start. Defines null prototype & type
void define_null(Scope *scope) {
	null_prototype = new NullPrototype();
	scope->table->put(string("Null"), null_prototype);
	
	null_prototype->table->put(string("__typename"),   new String("Null"));
	null_prototype->table->put(string("__operator=="), new NativeFunction(&operator_eq));
	null_prototype->table->put(string("__operator!="), new NativeFunction(&operator_neq));
	null_prototype->table->put(string("__operator||"), new NativeFunction(&operator_or));
};
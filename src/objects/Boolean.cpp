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
#include "Integer.h"
#include "Boolean.h"
#include "StringType.h"

#include "ObjectConverter.h"

#include "../string.h"

BooleanPrototype *boolean_prototype;

// Boolean type
Boolean::Boolean() {		
	type  = BOOLEAN;
	value = 0;
};

Boolean::Boolean(bool i) {		
	type  = BOOLEAN;
	value = i;
};

VirtualObject *Boolean::get(Scope *scope, string *name) {
	return boolean_prototype->table->get(*name);
};

void Boolean::put(Scope *scope, string *name, VirtualObject *value) {};

void Boolean::remove(Scope *scope, string *name) {};

bool Boolean::contains(Scope *scope, string *name) {
	return boolean_prototype->table->contains(*name);
};

VirtualObject *Boolean::call(Scope *scope, int argc, VirtualObject **args) {
	return new Boolean(value);
};

// Boolean prototype	
BooleanPrototype::BooleanPrototype() {		
	// table = new TreeObjectMap;
	type  = BOOLEAN_PROTOTYPE;
};

void BooleanPrototype::finalize(void) {
	table->finalize();
};

VirtualObject *BooleanPrototype::get(Scope *scope, string *name) {
	return table->get(*name);
};

void BooleanPrototype::put(Scope *scope, string *name, VirtualObject *value) {
	table->put(*name, value);
};

void BooleanPrototype::remove(Scope *scope, string *name) {
	table->remove(*name);
};

bool BooleanPrototype::contains(Scope *scope, string *name) {
	return table->contains(*name);
};

VirtualObject *BooleanPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	if (argc)
		return new Boolean(objectIntValue(args[0]));
	return new Boolean;
};

void BooleanPrototype::mark(void) {
	if (gc_reachable)
		return;
	gc_reachable = 1;
	table->mark();
};


// Operators

// ==
static VirtualObject* operator_eq(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	if (args[0]->type != BOOLEAN)
		return new Undefined;
	
	Boolean       *a = (Boolean*) args[0];
	VirtualObject *b =            args[1];
	
	return new Boolean(!a->value == !objectIntValue(b));
};

// != / ^
static VirtualObject* operator_neq(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	if (args[0]->type != BOOLEAN)
		return new Undefined;
	
	Boolean       *a = (Boolean*) args[0];
	VirtualObject *b =            args[1];
	
	return new Boolean(!a->value != !objectIntValue(b));
};

// +
static VirtualObject* operator_add(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	if (args[0]->type != BOOLEAN)
		return new Undefined;
	
	Boolean       *a = (Boolean*) args[0];
	VirtualObject *b =            args[1];
	
	return new Boolean((!!a->value + !!objectIntValue(b)) % 2);
};

// -
static VirtualObject* operator_sub(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	if (args[0]->type != BOOLEAN)
		return new Undefined;
	
	Boolean       *a = (Boolean*) args[0];
	VirtualObject *b =            args[1];
	
	return new Boolean((!!a->value - !!objectIntValue(b) + 2) % 2);
};

// && / &
static VirtualObject* operator_and(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	if (args[0]->type != BOOLEAN)
		return new Undefined;
	
	Boolean       *a = (Boolean*) args[0];
	VirtualObject *b =            args[1];
	
	return new Boolean(a->value && objectIntValue(b));
};

// || / |
static VirtualObject* operator_or(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	if (args[0]->type != BOOLEAN)
		return new Undefined;
	
	Boolean       *a = (Boolean*) args[0];
	VirtualObject *b =            args[1];
	
	return new Boolean(a->value || objectIntValue(b));
};

// >
static VirtualObject* operator_gt(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	if (args[0]->type != BOOLEAN)
		return new Undefined;
	
	Boolean       *a = (Boolean*) args[0];
	VirtualObject *b =            args[1];
	
	return new Boolean(a->value && !objectIntValue(b));
};

// <
static VirtualObject* operator_lt(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	if (args[0]->type != BOOLEAN)
		return new Undefined;
	
	Boolean       *a = (Boolean*) args[0];
	VirtualObject *b =            args[1];
	
	return new Boolean(!a->value && objectIntValue(b));
};

// >=
static VirtualObject* operator_ge(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	if (args[0]->type != BOOLEAN)
		return new Undefined;
	
	Boolean       *a = (Boolean*) args[0];
	VirtualObject *b =            args[1];
	
	return new Boolean(!!a->value <= !!objectIntValue(b));
};

// <=
static VirtualObject* operator_le(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	if (args[0]->type != BOOLEAN)
		return new Undefined;
	
	Boolean       *a = (Boolean*) args[0];
	VirtualObject *b =            args[1];
	
	return new Boolean(!!a->value >= !!objectIntValue(b));
};

// ! / ~ / ++ / --
static VirtualObject* operator_not(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	if (args[0]->type != BOOLEAN)
		return new Undefined;
	
	Boolean       *a = (Boolean*) args[0];
	
	return new Boolean(!a->value);
};

// Called on start. Defines boolean prototype & type
void define_boolean(Scope *scope) {
	boolean_prototype = new BooleanPrototype();
	scope->table->put(string("Boolean"), boolean_prototype);
	
	boolean_prototype->table->put(string("__typename"),   new String("Boolean"));
	boolean_prototype->table->put(string("__operator=="), new NativeFunction(&operator_eq));
	boolean_prototype->table->put(string("__operator!="), new NativeFunction(&operator_neq));
	boolean_prototype->table->put(string("__operator>"),  new NativeFunction(&operator_gt));
	boolean_prototype->table->put(string("__operator<"),  new NativeFunction(&operator_le));
	boolean_prototype->table->put(string("__operator>="), new NativeFunction(&operator_ge));
	boolean_prototype->table->put(string("__operator<="), new NativeFunction(&operator_lt));
	boolean_prototype->table->put(string("__operator+"),  new NativeFunction(&operator_add));
	boolean_prototype->table->put(string("__operator-"),  new NativeFunction(&operator_sub));
	boolean_prototype->table->put(string("__operator&&"), new NativeFunction(&operator_and));
	boolean_prototype->table->put(string("__operator||"), new NativeFunction(&operator_or));
	boolean_prototype->table->put(string("__operator&"),  new NativeFunction(&operator_and));
	boolean_prototype->table->put(string("__operator|"),  new NativeFunction(&operator_or));
	boolean_prototype->table->put(string("__operator^"),  new NativeFunction(&operator_neq));
	boolean_prototype->table->put(string("__operator!x"), new NativeFunction(&operator_not));
	boolean_prototype->table->put(string("__operator~x"), new NativeFunction(&operator_not));
	boolean_prototype->table->put(string("__operator++"), new NativeFunction(&operator_not));
	boolean_prototype->table->put(string("__operator--"), new NativeFunction(&operator_not));
	boolean_prototype->table->put(string("SIZE"),         new Integer(sizeof(bool)));
};


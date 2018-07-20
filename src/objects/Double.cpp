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
#include "Double.h"
#include "Null.h"
#include "Undefined.h"
#include "Integer.h"
#include "NativeFunction.h"
#include "Boolean.h"
#include "StringType.h"
#include "ObjectConverter.h"

#include "../string.h"

DoublePrototype *double_prototype = NULL;

// Double type
Double::Double() {		
	type  = DOUBLE;
	value = 0;
};

Double::Double(double d) {		
	type  = DOUBLE;
	value = d;
};

VirtualObject *Double::get(Scope *scope, string *name) {
	VirtualObject *v = double_prototype->table->get(*name);
	
	if (v)
		return v;
	
	return new Null;
};

void Double::put(Scope *scope, string *name, VirtualObject *value) {};

void Double::remove(Scope *scope, string *name) {};

bool Double::contains(Scope *scope, string *name) {
	return double_prototype->table->contains(*name);
};

VirtualObject *Double::call(Scope *scope, int argc, VirtualObject **args) {
	return new Double(value);
};

// Double prototype	
DoublePrototype::DoublePrototype() {		
	// table = new TreeObjectMap;
	type  = DOUBLE_PROTOTYPE;
};

void DoublePrototype::finalize(void) {
	table->finalize();
};

VirtualObject *DoublePrototype::get(Scope *scope, string *name) {
	return table->get(*name);
};

void DoublePrototype::put(Scope *scope, string *name, VirtualObject *value) {
	table->put(*name, value);
};

void DoublePrototype::remove(Scope *scope, string *name) {
	table->remove(*name);
};

bool DoublePrototype::contains(Scope *scope, string *name) {
	return table->contains(*name);
};

VirtualObject *DoublePrototype::call(Scope *scope, int argc, VirtualObject **args) {
	if (argc)
		return new Double(objectDoubleValue(args[0]));
	return new Double;
};

void DoublePrototype::mark(void) {
	if (gc_reachable)
		return;
	gc_reachable = 1;
	table->mark();
};

// Operators

// Operators

// ==
static VirtualObject* operator_eq(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectDoubleValue(args[0]) == objectDoubleValue(args[1]));
};

// !=
static VirtualObject* operator_neq(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectDoubleValue(args[0]) != objectDoubleValue(args[1]));
};

// >
static VirtualObject* operator_gt(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectDoubleValue(args[0]) > objectDoubleValue(args[1]));
};

// <
static VirtualObject* operator_lt(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectDoubleValue(args[0]) < objectDoubleValue(args[1]));
};

// >=
static VirtualObject* operator_ge(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectDoubleValue(args[0]) >= objectDoubleValue(args[1]));
};

// <=
static VirtualObject* operator_le(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectDoubleValue(args[0]) <= objectDoubleValue(args[1]));
};

// +
static VirtualObject* operator_sum(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Double(objectDoubleValue(args[0]) + objectDoubleValue(args[1]));
};

// -
static VirtualObject* operator_sub(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Double(objectDoubleValue(args[0]) - objectDoubleValue(args[1]));
};

// *
static VirtualObject* operator_mul(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Double(objectDoubleValue(args[0]) * objectDoubleValue(args[1]));
};

// /
static VirtualObject* operator_div(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	if (args[0]->type != DOUBLE)
		return new Undefined;
	
	return new Double(objectDoubleValue(args[0]) / objectDoubleValue(args[1]));
};

// <<
static VirtualObject* operator_shl(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Double(objectIntValue(args[0]) << objectIntValue(args[1]));
};

// >>
static VirtualObject* operator_shr(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Double(objectIntValue(args[0]) >> objectIntValue(args[1]));
};

// >>>
static VirtualObject* operator_ushr(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Double((unsigned int) objectIntValue(args[0]) >> objectIntValue(args[1]));
};

// &
static VirtualObject* operator_band(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Double(objectIntValue(args[0]) & objectIntValue(args[1]));
};

// |
static VirtualObject* operator_bor(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Double(objectIntValue(args[0]) | objectIntValue(args[1]));
};

// &
static VirtualObject* operator_and(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Double(objectIntValue(args[0]) && objectIntValue(args[1]));
};

// |
static VirtualObject* operator_or(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Double(objectIntValue(args[0]) || objectIntValue(args[1]));
};

// ^
static VirtualObject* operator_bxor(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Double(objectIntValue(args[0]) ^ objectIntValue(args[1]));
};

// !x
static VirtualObject* operator_not(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Double(!objectDoubleValue(args[0]));
};

// -x
static VirtualObject* operator_neg(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Double(-objectDoubleValue(args[0]));
};

// +x
static VirtualObject* operator_pos(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Double(objectDoubleValue(args[0]));
};

// ++
static VirtualObject* operator_inc(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Double(objectDoubleValue(args[0]) + 1);
};

// --
static VirtualObject* operator_dec(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Double(objectDoubleValue(args[0]) - 1);
};


// Functions
// toString(double)
static VirtualObject* function_tostring(Scope *scope, int argc, VirtualObject **args) {
	if (!argc || args[0]->type != DOUBLE)
		return NULL;
	
	return new String(objectStringValue(args[0]));
};


// Called on start. Defines double prototype & type
void define_double(Scope *scope) {
	double_prototype = new DoublePrototype();
	scope->table->put(string("Double"), double_prototype);
	
	double_prototype->table->put(string("__typename"),    new String("Double"));
	double_prototype->table->put(string("__operator=="),  new NativeFunction(&operator_eq));
	double_prototype->table->put(string("__operator!="),  new NativeFunction(&operator_neq));
	double_prototype->table->put(string("__operator>"),   new NativeFunction(&operator_gt));
	double_prototype->table->put(string("__operator<"),   new NativeFunction(&operator_lt));
	double_prototype->table->put(string("__operator>="),  new NativeFunction(&operator_ge));
	double_prototype->table->put(string("__operator>="),  new NativeFunction(&operator_le));
	double_prototype->table->put(string("__operator+"),   new NativeFunction(&operator_sum));
	double_prototype->table->put(string("__operator-"),   new NativeFunction(&operator_sub));
	double_prototype->table->put(string("__operator*"),   new NativeFunction(&operator_mul));
	double_prototype->table->put(string("__operator/"),   new NativeFunction(&operator_div));
	double_prototype->table->put(string("__operator<<"),  new NativeFunction(&operator_div));
	double_prototype->table->put(string("__operator>>"),  new NativeFunction(&operator_shr));
	double_prototype->table->put(string("__operator>>>"), new NativeFunction(&operator_ushr));
	double_prototype->table->put(string("__operator&"),   new NativeFunction(&operator_band));
	double_prototype->table->put(string("__operator|"),   new NativeFunction(&operator_bor));
	double_prototype->table->put(string("__operator&&"),  new NativeFunction(&operator_and));
	double_prototype->table->put(string("__operator||"),  new NativeFunction(&operator_or));
	double_prototype->table->put(string("__operator^"),   new NativeFunction(&operator_bxor));
	double_prototype->table->put(string("__operator-x"),  new NativeFunction(&operator_neg));
	double_prototype->table->put(string("__operator+x"),  new NativeFunction(&operator_pos));
	double_prototype->table->put(string("__operator!x"),  new NativeFunction(&operator_not));
	double_prototype->table->put(string("__operator++"),  new NativeFunction(&operator_inc));
	double_prototype->table->put(string("__operator--"),  new NativeFunction(&operator_dec));
	
	double_prototype->table->put(string("toString"),      new NativeFunction(&function_tostring));
	double_prototype->table->put(string("SIZE"),          new Integer(sizeof(double)));
};


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
#include "Integer.h"
#include "Double.h"
#include "Null.h"
#include "Undefined.h"
#include "NativeFunction.h"
#include "Boolean.h"
#include "StringType.h"
#include "ObjectConverter.h"

#include "../string.h"
#include "../ptr_wrapper.h"

IntegerPrototype *integer_prototype = NULL;

// Integer type
Integer::Integer() {		
	type  = INTEGER;
	value = 0;
};

Integer::Integer(int i) {		
	type  = INTEGER;
	value = i;
};

VirtualObject *Integer::get(Scope *scope, string *name) {
	return integer_prototype->table->get(*name);
};

void Integer::put(Scope *scope, string *name, VirtualObject *value) {};

void Integer::remove(Scope *scope, string *name) {};

bool Integer::contains(Scope *scope, string *name) {
	return integer_prototype->contains(scope, name);
};

VirtualObject *Integer::call(Scope *scope, int argc, VirtualObject **args) {
	return new Integer(value);
};

// Integer prototype	
IntegerPrototype::IntegerPrototype() {		
	// table = new TreeObjectMap;
	type  = INTEGER_PROTOTYPE;
};

void IntegerPrototype::finalize(void) {
	table->finalize();
};

VirtualObject *IntegerPrototype::get(Scope *scope, string *name) {
	return table->get(*name);
};

void IntegerPrototype::put(Scope *scope, string *name, VirtualObject *value) {
	table->put(*name, value);
};

void IntegerPrototype::remove(Scope *scope, string *name) {
	table->remove(*name);
};

bool IntegerPrototype::contains(Scope *scope, string *name) {
	return table->contains(*name);
};

VirtualObject *IntegerPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	if (argc)
		return new Integer(objectIntValue(args[0]));
	return new Integer;
};

void IntegerPrototype::mark(void) {
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
	
	return new Boolean(objectIntValue(args[0]) == objectIntValue(args[1]));
};

// !=
static VirtualObject* operator_neq(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) != objectIntValue(args[1]));
};

// >
static VirtualObject* operator_gt(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) > objectIntValue(args[1]));
};

// <
static VirtualObject* operator_lt(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) < objectIntValue(args[1]));
};

// >=
static VirtualObject* operator_ge(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) >= objectIntValue(args[1]));
};

// <=
static VirtualObject* operator_le(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) <= objectIntValue(args[1]));
};

// +
static VirtualObject* operator_sum(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]) + objectIntValue(args[1]));
};

// -
static VirtualObject* operator_sub(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]) - objectIntValue(args[1]));
};

// *
static VirtualObject* operator_mul(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	if (args[0]->type == DOUBLE)
		return new Double(objectIntValue(args[0]) * objectDoubleValue(args[1]));
	
	return new Integer(objectIntValue(args[0]) * objectIntValue(args[1]));
};

// /
static VirtualObject* operator_div(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	int a_val = objectIntValue(args[0]);
	
	if (args[1]->type == DOUBLE)
		return new Double(a_val / ((Double*) args[1])->value);
	
	int b_val = objectIntValue(args[1]);
	if (b_val)
		return new Integer(a_val / b_val);
	
	scope->context->executer->raiseError("divide by zero");
	return new Undefined;
};

// %
static VirtualObject* operator_mod(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	if (args[0]->type != INTEGER)
		return new Undefined;
	
	Integer       *a = (Integer*) args[0];
	VirtualObject *b = args[1];
	
	int b_val = objectIntValue(b);
	if (b_val)
		return new Integer(a->value % b_val);
	return new Undefined;
};

// <<
static VirtualObject* operator_shl(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]) << objectIntValue(args[1]));
};

// >>
static VirtualObject* operator_shr(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]) >> objectIntValue(args[1]));
};

// >>>
static VirtualObject* operator_ushr(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Integer((unsigned int) objectIntValue(args[0]) >> objectIntValue(args[1]));
};

// &
static VirtualObject* operator_band(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]) & objectIntValue(args[1]));
};

// |
static VirtualObject* operator_bor(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]) | objectIntValue(args[1]));
};

// &&
static VirtualObject* operator_and(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) && objectIntValue(args[1]));
};

// ||
static VirtualObject* operator_or(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) || objectIntValue(args[1]));
};

// ^
static VirtualObject* operator_bxor(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]) ^ objectIntValue(args[1]));
};

// !x
static VirtualObject* operator_not(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Integer(!objectIntValue(args[0]));
};

// ~x
static VirtualObject* operator_bnot(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Integer(~objectIntValue(args[0]));
};

// -x
static VirtualObject* operator_neg(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Integer(-objectIntValue(args[0]));
};

// +x
static VirtualObject* operator_pos(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]));
};

// ++
static VirtualObject* operator_inc(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]) + 1);
};

// --
static VirtualObject* operator_dec(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]) - 1);
};


static char baseDigit(int digit) {
	if (digit < 10)
		return digit + '0';
	return 'a' + digit - 10;
};

// Functions
// toString(integer[, base])
static VirtualObject* function_tostring(Scope *scope, int argc, VirtualObject **args) {
	if (!argc || args[0]->type != INTEGER)
		return NULL;
	
	int i = ((Integer*) args[0])->value;
	int b = argc == 1 ? 10 : objectIntValue(args[1]);
	
	if (b < 2 || b > 36)
		return new Undefined;
	
	ptr_wrapper wrapper(new string, PTR_NEW);
	string *s = (string*) wrapper.ptr;
	
	*s += string::toString((long long) i, b);
	
	wrapper.deattach();
	return new String(s);
};

static VirtualObject* function_parseInt(Scope *scope, int argc, VirtualObject **args) {
	if (!argc)
		return NULL;
	
	int base = 10;
	if (argc >= 2)
		base = objectIntValue(args[1]);
	
	string s = objectStringValue(args[0]);
	int a = s.toInt(-1, base);
	int b = s.toInt(+1, base);
	
	if (a == b)
		return new Integer(a);
	
	return new Undefined();
};


// Called on start. Defines integer prototype & type
void define_integer(Scope *scope) {
	integer_prototype = new IntegerPrototype();
	scope->table->put(string("Integer"), integer_prototype);
	
	integer_prototype->table->put(string("__typename"),    new String("Integer"));
	integer_prototype->table->put(string("__operator=="),  new NativeFunction(&operator_eq));
	integer_prototype->table->put(string("__operator!="),  new NativeFunction(&operator_neq));
	integer_prototype->table->put(string("__operator>"),   new NativeFunction(&operator_gt));
	integer_prototype->table->put(string("__operator<"),   new NativeFunction(&operator_lt));
	integer_prototype->table->put(string("__operator>="),  new NativeFunction(&operator_ge));
	integer_prototype->table->put(string("__operator>="),  new NativeFunction(&operator_le));
	integer_prototype->table->put(string("__operator+"),   new NativeFunction(&operator_sum));
	integer_prototype->table->put(string("__operator-"),   new NativeFunction(&operator_sub));
	integer_prototype->table->put(string("__operator*"),   new NativeFunction(&operator_mul));
	integer_prototype->table->put(string("__operator/"),   new NativeFunction(&operator_div));
	integer_prototype->table->put(string("__operator%"),   new NativeFunction(&operator_mod));
	integer_prototype->table->put(string("__operator<<"),  new NativeFunction(&operator_div));
	integer_prototype->table->put(string("__operator>>"),  new NativeFunction(&operator_shr));
	integer_prototype->table->put(string("__operator>>>"), new NativeFunction(&operator_ushr));
	integer_prototype->table->put(string("__operator&"),   new NativeFunction(&operator_band));
	integer_prototype->table->put(string("__operator|"),   new NativeFunction(&operator_bor));
	integer_prototype->table->put(string("__operator&&"),  new NativeFunction(&operator_and));
	integer_prototype->table->put(string("__operator||"),  new NativeFunction(&operator_or));
	integer_prototype->table->put(string("__operator^"),   new NativeFunction(&operator_bxor));
	integer_prototype->table->put(string("__operator-x"),  new NativeFunction(&operator_neg));
	integer_prototype->table->put(string("__operator+x"),  new NativeFunction(&operator_pos));
	integer_prototype->table->put(string("__operator!x"),  new NativeFunction(&operator_not));
	integer_prototype->table->put(string("__operator~x"),  new NativeFunction(&operator_bnot));
	integer_prototype->table->put(string("__operator++"),  new NativeFunction(&operator_inc));
	integer_prototype->table->put(string("__operator--"),  new NativeFunction(&operator_dec));
	
	integer_prototype->table->put(string("toString"),      new NativeFunction(&function_tostring));
	integer_prototype->table->put(string("parseInt"),      new NativeFunction(&function_parseInt));
	integer_prototype->table->put(string("SIZE"),          new Integer(sizeof(int)));
};


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


#include "Array.h"
#include "Scope.h"
#include "Null.h"
#include "Undefined.h"
#include "Integer.h"
#include "Double.h"
#include "Boolean.h"
#include "NativeFunction.h"
#include "StringType.h"
#include "ObjectConverter.h"

#include "../string.h"

ArrayPrototype *array_prototype = NULL;

// Array type
Array::Array() {
	array = new VectorArray<VirtualObject>;
	if (array_prototype)
		table->putAll(array_prototype->table);
	type = ARRAY;
};

Array::Array(VectorArray<VirtualObject> *array) {
	this->array = array;
	if (array_prototype)
		table->putAll(array_prototype->table);
	type = ARRAY;
};

void Array::finalize(void) {
	delete array;
	table->finalize();
};

static bool digit(int c) {
	return '0' <= c && c <= '9';
};

static int parsePositiveIndex(string *s) {
	if (s == NULL || s->length == 0)
		return -1;
	
	int n = 0;
	for (int i = 0; i < s->length; ++i) {
		if (n >= 0 && digit(s->buffer[i]))
			n = n * 10 + (s->buffer[i] - '0');
		else
			return -1;
	}
	
	return n;
};

VirtualObject *Array::get(Scope *scope, string *name) {
	int i = parsePositiveIndex(name);
	if (i != -1)
		return array->get(i);
	
	return table->get(*name);
};

void Array::put(Scope *scope, string *name, VirtualObject *value) {
	int i = parsePositiveIndex(name);
	if (i != -1) {
		for (int k = 0; k < i; ++k)
			array->push(NULL);
		
		array->push(value);
		return;
	}
	
	table->put(*name, value);
};

void Array::remove(Scope *scope, string *name) {
	table->remove(*name);
};

bool Array::contains(Scope *scope, string *name) {	
	return table->contains(*name);
};

VirtualObject *Array::call(Scope *scope, int argc, VirtualObject **args) {
	return new Array(array);
};

void Array::mark(void) {
	if (gc_reachable)
		return;
	gc_reachable = 1;
	table->mark();
	
	if (array)
		for (int i = 0; i < array->length; ++i)
			if (array->vector[i])
				array->vector[i]->mark();
};

void Array::push(VirtualObject *o) {
	array->push(o);
};

string Array::toString() {
	Array *a = this;
	string s = "[";
	for (int i = 0; i < a->array->length; ++i) {
		if (a->array->vector[i]->type == ARRAY)
			if (((Array*) a->array->vector[i])->array->length == 0)
				s += "[]";
			else
				s += "[..]";
		else
			s += objectStringValue(a->array->vector[i]);
		
		if (i != a->array->length - 1)
			s += ", ";
	}
	s += "]";
	
	return s;
};

long Array::toInt() {
	return array->length + 1;
};

double Array::toDouble() {
	return array->length + 1;
};


// Array prototype	
ArrayPrototype::ArrayPrototype() {	
	type = ARRAY_PROTOTYPE;
};

void ArrayPrototype::finalize(void) {
	table->finalize();
};

VirtualObject *ArrayPrototype::get(Scope *scope, string *name) {
	return table->get(*name);
};

void ArrayPrototype::put(Scope *scope, string *name, VirtualObject *value) {
	table->put(*name, value);
}; 

void ArrayPrototype::remove(Scope *scope, string *name) {
	table->remove(*name);
};

bool ArrayPrototype::contains(Scope *scope, string *name) {
	return table->contains(*name);
};

VirtualObject *ArrayPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	return new Array;
};

void ArrayPrototype::mark(void) {
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
	
	if (!(args[0] && args[1]))
		return new Boolean;
	
	if (args[0]->type == ARRAY && args[1]->type == ARRAY) {
		Array *a = (Array*) args[0];
		Array *b = (Array*) args[1];
		
		if (a->array->length != b->array->length)
			return new Boolean;
		
		for (int i = 0; i < a->array->length; ++i)
			if (a->array->get(i) != b->array->get(i))
				return new Boolean;
			
		return new Boolean(1);
	}
	return new Boolean;
};

// !=
static VirtualObject* operator_neq(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;	
	
	if (!(args[0] && args[1]))
		return new Boolean;
	
	if (args[0]->type == ARRAY && args[1]->type == ARRAY) {
		Array *a = (Array*) args[0];
		Array *b = (Array*) args[1];
		
		if (a->array->length != b->array->length)
			return new Boolean(1);
		
		for (int i = 0; i < a->array->length; ++i)
			if (a->array->get(i) != b->array->get(i))
				return new Boolean(1);
			
		return new Boolean;
	}
	return new Boolean(1);
};

// +
// Merge two arrays
static VirtualObject* operator_add(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;	
	
	if (!(args[0] && args[1]))
		return NULL;
	
	if (!(args[0]->type == ARRAY && args[1]->type == ARRAY))
		return NULL;
	
	Array *a = (Array*) args[0];
	Array *b = (Array*) args[1];
	
	Array *c = new Array(new VectorArray<VirtualObject>(a->array->length + b->array->length));
	
	for (int i = 0; i < a->array->length; ++i)
		c->array->push(a->array->vector[i]);
	for (int i = 0; i < b->array->length; ++i)
		c->array->push(b->array->vector[i]);
	
	return c;
};

// push(value [, value])
static VirtualObject* function_push(Scope *scope, int argc, VirtualObject **args) {
	// Function called with enclosing scopes:
	// [ProxyScope]
	//  \
	//   [Scope]
	//    \
	//     [this native function]
	
	// Conditions of execution:
	// 1. scope != NULL
	// 2. parent scope != NULL
	// 3. parent scope type is PROXY_SCOPE
	// 4. parent scope object != NULL
	// 5. parent scope object type is ARRAY
	// 5. parent scope object array != NULL
	
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object) && o->type == ARRAY && ((Array*) o)->array))
		return NULL;
	
	Array *a = (Array*) o;
	int  i = 0;
	while (i < argc) 
		a->array->push(args[i++]);
	
	
	return NULL;
};

// pop([count])
// Pops n = [count] values from array & returns 
static VirtualObject* function_pop(Scope *scope, int argc, VirtualObject **args) {
	// Function called with enclosing scopes:
	// [ProxyScope]
	//  \
	//   [Scope]
	//    \
	//     [this native function]
	
	// Conditions of execution:
	// 1. scope != NULL
	// 2. parent scope != NULL
	// 3. parent scope type is PROXY_SCOPE
	// 4. parent scope object != NULL
	// 5. parent scope object type is ARRAY
	// 5. parent scope object array != NULL
	
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object) && o->type == ARRAY && ((Array*) o)->array))
		return NULL;
	
	Array *a = (Array*) o;
	VirtualObject *t = NULL;
	
	int n = (argc ? objectIntValue(args[0]) : 0) + 1;
	int i = 0;
	while (i++ < n && (t = a->array->pop()));
	
	return t;
};

// size()
static VirtualObject* function_size(Scope *scope, int argc, VirtualObject **args) {
	// Function called with enclosing scopes:
	// [ProxyScope]
	//  \
	//   [Scope]
	//    \
	//     [this native function]
	
	// Conditions of execution:
	// 1. scope != NULL
	// 2. parent scope != NULL
	// 3. parent scope type is PROXY_SCOPE
	// 4. parent scope object != NULL
	// 5. parent scope object type is ARRAY
	// 5. parent scope object array != NULL
	
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object) && o->type == ARRAY && ((Array*) o)->array))
		return NULL;
	
	return new Integer(((Array*) o)->array->length);
};

// Called on start. Defines array prototype & type
void define_array(Scope *scope) {
	array_prototype = new ArrayPrototype();
	scope->table->put(string("Array"), array_prototype);
	
	array_prototype->table->put(string("__typename"),   new String("Array"));
	array_prototype->table->put(string("__operator=="), new NativeFunction(&operator_eq));
	array_prototype->table->put(string("__operator!="), new NativeFunction(&operator_eq));
	array_prototype->table->put(string("__operator+"),  new NativeFunction(&operator_add));
	array_prototype->table->put(string("push"),         new NativeFunction(&function_push));	
	array_prototype->table->put(string("pop"),          new NativeFunction(&function_push));	
	array_prototype->table->put(string("size"),         new NativeFunction(&function_size));	
};


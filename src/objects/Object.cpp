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
#include "Object.h"
#include "Array.h"
#include "Double.h"
#include "Null.h"
#include "Undefined.h"
#include "NativeFunction.h"
#include "Boolean.h"
#include "StringType.h"
#include "ObjectConverter.h"

#include "../string.h"
#include "../VectorArray.h"

ObjectPrototype *object_prototype = NULL;

// Object type
Object::Object() {		
	table = new TreeObjectMap;
	if (object_prototype)
		table->putAll(object_prototype->table);
	type  = OBJECT;
};

Object::Object(TreeObjectMap *map) {		
	table = new TreeObjectMap;
	type  = OBJECT;
	table->putAll(map);
};

void Object::finalize(void) {
	table->finalize();
};

VirtualObject *Object::get(Scope *scope, string *name) {
	return table->get(*name);
};

void Object::put(Scope *scope, string *name, VirtualObject *value) {
	table->put(*name, value);
};

void Object::remove(Scope *scope, string *name) {
	table->remove(*name);
};

bool Object::contains(Scope *scope, string *name) {
	return table->contains(*name);
};

VirtualObject *Object::call(Scope *scope, int argc, VirtualObject **args) {
	return new Object(table);
};

void Object::mark(void) {
	if (gc_reachable)
		return;
	gc_reachable = 1;
	table->mark();
};

string Object::toString() {
	if (!table)
		return "[Object]";
	return string("[") + objectStringValue(table->get(string("__typename"))) + "]";
};

long Object::toInt() {
	return table->size + 1;
};

double Object::toDouble() {
	return table->size + 1;
};


// Object prototype	
ObjectPrototype::ObjectPrototype() {	
	// printf("- - - - ObjectPrototype constructor\n");
	table = new TreeObjectMap;
	type  = OBJECT_PROTOTYPE;
};

void ObjectPrototype::finalize(void) {
	table->finalize();
};

VirtualObject *ObjectPrototype::get(Scope *scope, string *name) {
	return table->get(*name);
};

void ObjectPrototype::put(Scope *scope, string *name, VirtualObject *value) {
	table->put(*name, value);
}; 

void ObjectPrototype::remove(Scope *scope, string *name) {
	table->remove(*name);
};

bool ObjectPrototype::contains(Scope *scope, string *name) {
	return table->contains(*name);
};

VirtualObject *ObjectPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	return new Object(table);
};

void ObjectPrototype::mark(void) {
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
	return new Boolean(args[0] == args[1]);
};

// !=
static VirtualObject* operator_neq(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	return new Boolean(args[0] != args[1]);
};

// remove(key [, key])
static VirtualObject* function_remove(Scope *scope, int argc, VirtualObject **args) {
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
	
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	int  i = 0;
	while (i < argc) {
		string s = objectStringValue(args[i++]);
		o->remove(scope, &s);
	}
	
	return NULL;
};

// append(key, value [,key, value])
static VirtualObject* function_append(Scope *scope, int argc, VirtualObject **args) {
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
	
	argc = argc - (argc % 2);
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	int  i = 0;
	while (i < argc) {
		string s = objectStringValue(args[i]);
		o->put(scope, &s, args[i++]);
	}
	
	return NULL;
};

// containsKey(key [, key])
static VirtualObject* function_containskey(Scope *scope, int argc, VirtualObject **args) {
	// Function called with enclosing scopes:
	// [ProxyScope]
	//  \
	//   [Scope]
	//    \
	//     [this native function]
	
	// Conditions of values containment:
	// 1. scope != NULL
	// 2. parent scope != NULL
	// 3. parent scope type is PROXY_SCOPE
	// 4. parent scope object != NULL
	// 5. parent scope object contains all given keys
	
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return new Boolean;
	
	bool b = argc;
	int  i = 0;
	while (i < argc && b) {
		string s = objectStringValue(args[i++]);
		b = b & o->contains(scope, &s);
	}
	
	return new Boolean(b);
};

// keys()
static VirtualObject* function_keys(Scope *scope, int argc, VirtualObject **args) {
	// Function called with enclosing scopes:
	// [ProxyScope]
	//  \
	//   [Scope]
	//    \
	//     [this native function]
	
	// Conditions of values containment:
	// 1. scope != NULL
	// 2. parent scope != NULL
	// 3. parent scope type is PROXY_SCOPE
	// 4. parent scope object != NULL
	// 5. parent scope object contains all given keys
	
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	Object *b = dynamic_cast<Object*>(o);
	
	if (b == nullptr) {
		Scope *s = dynamic_cast<Scope*>(o);
		
		if (s == nullptr)
			return NULL;
		
		Array *a = new Array();
		s->keys(a);
		
		return a;
	}
	
	Array *a = new Array(new VectorArray<VirtualObject>(b->table->size));
	b->table->keys(a);
	
	return a;
};

// Called on start. Defines object prototype & type
void define_object(Scope *scope) {
	object_prototype = new ObjectPrototype();
	scope->table->put(string("Object"), object_prototype);
	
	object_prototype->table->put(string("__typename"),   new String("Object"));
	object_prototype->table->put(string("__operator=="), new NativeFunction(&operator_eq));
	object_prototype->table->put(string("__operator!="), new NativeFunction(&operator_eq));
	object_prototype->table->put(string("containsKey"),  new NativeFunction(&function_containskey));
	object_prototype->table->put(string("remove"),       new NativeFunction(&function_remove));	
	object_prototype->table->put(string("append"),       new NativeFunction(&function_append));	
	object_prototype->table->put(string("concat"),       new NativeFunction(&function_append));	
	object_prototype->table->put(string("keys"),         new NativeFunction(&function_keys));
};


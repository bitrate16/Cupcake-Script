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


#include "Object.h"
#include "VirtualObject.h"
#include "Null.h"
#include "Undefined.h"
#include "Scope.h"
#include "Boolean.h"
#include "StringType.h"
#include "NativeFunction.h"
#include "../ASTExecuter.h"
#include "../GarbageCollector.h"

#include "../string.h"

#include "ObjectConverter.h"

ScopePrototype *scope_prototype = NULL;

// 
// 
// 
// 
//     [Scope]
//      + var A
//        + A = [Scope]:this
// 
//     [Scope]->dispose()
//      + for v in table
//        + v->dispose()
//          \
//           [Scope].A == [Scope] <- Stack overflow

// Scope type
Scope::Scope() {
	type     = SCOPE;
	context  = NULL;
	parent   = NULL;
	table    = NULL;
	priority = 0;
};

Scope::Scope(Scope *parent) {
	table          = new TreeObjectMap;
	type           = SCOPE;
	this->priority = 0;
	setParent(parent);
	
	if (scope_prototype)
		table->putAll(scope_prototype->table);
	if (parent)
		table->put(string("__parent"), parent);
	else
		table->put(string("__parent"), new Null);
};

Scope::Scope(Scope *parent, bool priority) {
	table          = new TreeObjectMap;
	type           = SCOPE;
	this->priority = priority;
	setParent(parent);
	
	if (scope_prototype)
		table->putAll(scope_prototype->table);
	if (parent)
		table->put(string("__parent"), parent);
	else
		table->put(string("__parent"), new Null);
};

void Scope::setParent(Scope *parent) {	
	this->parent = parent;
	if (parent) 
		context = parent->context;
	if (parent)
		table->put(string("__parent"), parent);
	else
		table->put(string("__parent"), new Null);
};

void Scope::finalize(void) {
	table->finalize();
};

VirtualObject *Scope::get(Scope *scope, string *name) {	
	VirtualObject *v = table->get(*name);
	
	if (v)
		return v;
	
	if (parent != NULL) {
		VirtualObject *v = parent->get(scope, name);
		
		if (v != NULL)
			return v;
	}
	
	return NULL;
};

// Without context, scope acts as simple object, 
// so we don't need to handle it's parents / references.
//
// This feature allows returning scope from functions:
// A = {};
// B = 13;
// A.f = function() { return self };
//
// Calling:
// S = A.f();
//
// As the result, attempt to read fields will be like this:
// S.B -> 13
// S.B =  17
// S.B -> 17
// But:
// B   -> 13
void Scope::put(Scope *scope, string *name, VirtualObject *value) {
	table->put(*name, value);
};

// Insert value incliding scopes priority.
// If priority == 1:
// The first scope, marked to be priority 
// scope / the first scope, having value 
// with the same key, will get the new walue.
// Else if priority == 0:
// The first scope with the same key will get new value.
void Scope::putPriority(Scope *scope, string *name, VirtualObject *value, bool priority) {	
	Scope *s = this;
	
	while (s) {
		if (priority && s->priority) {
			s->put(scope, name, value);
			return;
		}
		
		if (s->type == SCOPE) {
			TreeObjectMapEntry *e = s->table->findEntry(*name);
			
			if (e) {
				e->value = value;
				return;
			}
		} else if (s->type == PROXY_SCOPE) {
			VirtualObject *o = ((ProxyScope*) s)->object;
			if (o && o->contains(scope, name)) {
				o->put(scope, name, value);
				return;
			}
		}		
		
		s = s->parent;
	}
	
	table->put(*name, value);
};

// Called whatever executer tries to define
// new variable with var <name> { = <value> };
void Scope::define(string *name, VirtualObject *value) {
	if (table)
		table->put(*name, value);
};

void Scope::define(string name, VirtualObject *value) {
	if (table)
		table->put(name, value);
};

void Scope::remove(Scope *scope, string *name) {
	if (table)
		table->remove(*name);
};

bool Scope::contains(Scope *scope, string *name) {
	if (*name == "__parent")
		return 1;
	
	return table || table->contains(name) || (parent && parent->contains(scope, name));
}; 

VirtualObject *Scope::call(Scope *scope, int argc, VirtualObject **args) {
	return this;
};

void Scope::mark(void) {
	if (parent && !parent->gc_reachable)
		parent->mark();
	
	if (gc_reachable)
		return;
	gc_reachable = 1;
	if (table)
		table->mark();
	
	context->mark();
};

Scope *Scope::enclosingObject() {
	Scope *s = this;
	
	while (s)
		if (s->type == PROXY_SCOPE)
			return s;
		else
			s = s->parent;
	
	return this;
};

void Scope::keys(Array *a) {
	if (table)
		table->keys(a);
};

Scope *Scope::getRoot() {
	return parent == NULL ? this : parent->getRoot();
};


// ProxyScope type
ProxyScope::ProxyScope(Scope *parent, VirtualObject *object) {
	table          = NULL;
	type           = PROXY_SCOPE;
	this->object   = object;
	this->priority = 1;
	setParent(parent);
};

void ProxyScope::setParent(Scope *parent) {	
	this->parent = parent;
	if (parent) 
		context = parent->context;
};

void ProxyScope::finalize(void) {};

VirtualObject *ProxyScope::get(Scope *scope, string *name) {
	if (*name == "__parent")
		return parent;
	if (*name == "__object")
		return object;
	
	if (object && object->contains(scope, name))
		return object->get(scope, name);
	
	if (parent != NULL) {
		VirtualObject *v = parent->get(scope, name);
		
		if (v != NULL)
			return v;
	}
	
	return NULL;
};

void ProxyScope::put(Scope *scope, string *name, VirtualObject *value) {	
	if (object)
		object->put(scope, name, value);	
};

void ProxyScope::define(string *name, VirtualObject *value) {
	if (object)
		object->put(this, name, value);
};

void ProxyScope::define(string name, VirtualObject *value) {
	if (object)
		object->put(this, &name, value);
};

void ProxyScope::remove(Scope *scope, string *name) {
	if (object)
		object->remove(scope, name);
};

bool ProxyScope::contains(Scope *scope, string *name) {
	if (*name == "__parent")
		return 1;
	if (*name == "__object")
		return 1;
	
	return (object && object->contains(scope, name)) || (parent && parent->contains(scope, name));
}; 

VirtualObject *ProxyScope::call(Scope *scope, int argc, VirtualObject **args) {
	return this;
};

void ProxyScope::mark(void) {
	if (parent && !parent->gc_reachable)
		parent->mark();
	
	if (object && !object->gc_reachable)
		object->mark();
	
	gc_reachable = 1;
	
	context->mark();
};

void ProxyScope::keys(Array *a) {
	if (object)
		if (object->type == PROXY_SCOPE)
			((ProxyScope*) object)->keys(a);
		else {
			Object *b = dynamic_cast<Object*>(object);
	
			if (b == nullptr)
				return;
			
			b->table->keys(a);
		}
};

Scope *ProxyScope::getRoot() {
	if (parent == NULL)
		if (object)
			if (object->type == SCOPE)
				return ((Scope*) object)->getRoot();
			else if (object->type == SCOPE)
				return ((ProxyScope*) object)->getRoot();
			else
				return this;
	return parent->getRoot();
};


// Scope prototype	
ScopePrototype::ScopePrototype() {	
	table = new TreeObjectMap;
	type  = SCOPE_PROTOTYPE;
};

void ScopePrototype::finalize(void) {
	table->finalize();
};

VirtualObject *ScopePrototype::get(Scope *scope, string *name) {
	return table->get(*name);
};

void ScopePrototype::put(Scope *scope, string *name, VirtualObject *value) {
	table->put(*name, value);
}; 

void ScopePrototype::remove(Scope *scope, string *name) {
	table->remove(*name);
};

bool ScopePrototype::contains(Scope *scope, string *name) {
	return table->contains(*name);
};

VirtualObject *ScopePrototype::call(Scope *scope, int argc, VirtualObject **args) {
	return new Scope(NULL);
};

void ScopePrototype::mark(void) {
	if (gc_reachable)
		return;
	gc_reachable = 1;
	table->mark();
};


// Called on start. Defines NativeFunction prototype & type
void define_scope(Scope *scope) {
	scope_prototype = new ScopePrototype();
	scope->table->put(string("Scope"), scope_prototype);
	
	if (object_prototype)
		scope_prototype->table->putAll(object_prototype->table);
	scope_prototype->table->put(string("__typename"),   new String("Scope"));
	
	scope->table->putAll(scope_prototype->table);
};


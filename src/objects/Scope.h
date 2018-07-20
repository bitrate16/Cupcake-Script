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
 * This class defines representation of 
 * execution Scope in cupcake VM.
 */

#ifndef SCOPE_H
#define SCOPE_H

#include "VirtualObject.h"
#include "TreeObjectMap.h"

#include "../TokenNamespace.h"
#include "../string.h"
#include "../ASTExecuter.h"
#include "../GarbageCollector.h"

struct Context;	

// Scope prototype's prototype
struct ScopePrototype : VirtualObject {
	TreeObjectMap *table;
	ScopePrototype();
	void finalize(void);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	VirtualObject *call(Scope*, int, VirtualObject**);
	void mark(void);
};

// Scope type's prototype
struct Scope : VirtualObject {
	Context *context;
	Scope    *parent;
	
	TreeObjectMap *table;
	// Marks if this scope has priority 
	// over it's parent in putting value. 
	// That means if assgnment will be called, 
	// value will be inserted into the closest 
	// privilleged scope / scope, that contains
	// field of such name.
	bool priority;
	
	Scope();
	Scope(Scope*);
	Scope(Scope*, bool);
	virtual void setParent(Scope*);
	virtual void finalize(void);
	virtual VirtualObject *get(Scope*, string*);
	virtual void put(Scope*, string*, VirtualObject*);
	virtual void putPriority(Scope*, string*, VirtualObject*, bool);
	virtual void remove(Scope*, string*);
	virtual bool contains(Scope*, string*);
	virtual void define(string*, VirtualObject*);
	virtual void define(string, VirtualObject*);
	virtual VirtualObject *call(Scope*, int, VirtualObject**);
	virtual void mark(void);
	virtual void keys(Array*);
	
	// Retuns closest proxy scope instance / self
	Scope *enclosingObject();
	
	Scope *getRoot();
};

// Wrapper for objects
struct ProxyScope : Scope {	
	// Object, proxied by this scope
	VirtualObject *object;
	
	ProxyScope(Scope*, VirtualObject*, bool);
	ProxyScope(Scope*, VirtualObject*);
	void setParent(Scope*);
	void finalize(void);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	void define(string*, VirtualObject*);
	void define(string, VirtualObject*);
	VirtualObject *call(Scope*, int, VirtualObject**);
	void mark(void);
	void keys(Array*);
	
	Scope *getRoot();
};

void define_scope(Scope *scope);

extern ScopePrototype *scope_prototype;

#endif
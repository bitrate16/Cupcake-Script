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
 * executable source function.
 */

#ifndef CODE_FUNCTION_H
#define CODE_FUNCTION_H

#include "Object.h"
#include "Scope.h"
#include "TreeObjectMap.h"

#include "../TokenNamespace.h"
#include "../ASTNode.h"
#include "../string.h"

// CodeFunction prototype's prototype	
struct CodeFunctionPrototype : Object {
	CodeFunctionPrototype();
	void finalize(void);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	VirtualObject *call(Scope*, int, VirtualObject**);
	void mark(void);
};

// CodeFunction type's prototype

typedef VirtualObject* (*handler_function) (Scope*, int, VirtualObject**);

struct CodeFunction : Object {
	// Node, that refers to this function
	ASTNode        *node;
	// Definition scope of the function
	Scope         *scope;
	// 1 if fucntion was created dynamically
	// var f = Function("return x + y;", 'x', 'y');
	bool       dynamical;
	CodeFunction(Scope *scope, ASTNode *node);
	void finalize(void);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	VirtualObject *call(Scope*, int, VirtualObject**);
	void mark(void);
};

// Called on start. Defines null prototype & type
void define_code_function(Scope*);

extern CodeFunctionPrototype *code_function_prototype;

#endif
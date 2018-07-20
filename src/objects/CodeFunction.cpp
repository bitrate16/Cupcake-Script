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
#include "CodeFunction.h"
#include "NativeFunction.h"
#include "Boolean.h"
#include "Integer.h"
#include "StringType.h"
#include "ObjectConverter.h"

#include "../string.h"
#include "../Parser.h"

CodeFunctionPrototype *code_function_prototype = NULL;

// CodeFunction type
CodeFunction::CodeFunction(Scope *scope, ASTNode *node) {
	this->dynamical = 0;
	this->type      = CODE_FUNCTION;
	this->node      = node;
	this->scope     = scope;
	// table         = new TreeObjectMap;
	table->putAll(code_function_prototype->table);
	table->put(string("__scope"), scope);
};

void CodeFunction::finalize(void) {
	table->finalize();
	
	if (dynamical)
		delete this->node;
};

VirtualObject *CodeFunction::get(Scope *scope, string *name) {
	return table->get(*name);;
};

void CodeFunction::put(Scope *scope, string *name, VirtualObject *value) {
	table->put(*name, value);
};

void CodeFunction::remove(Scope *scope, string *name) {
	table->remove(*name);
};

bool CodeFunction::contains(Scope *scope, string *name) {
	return table->contains(*name);
};

// Apply arguments on the given scope & assign parent of this scope
VirtualObject *CodeFunction::call(Scope *scope, int argc, VirtualObject **args) {
	// scope->parent = this->scope;
	ASTObjectList *l = this->node->objectlist;
	for (int i = 0; i < argc; i++) {
		if (l) {
			scope->put(scope, (string*) l->object, args[i]);
			l = l->next;
		} else
			break;
	};
};

void CodeFunction::mark(void) {
	if (scope)
		scope->mark();
	
	if (gc_reachable)
		return;
	gc_reachable = 1;
	table->mark();
	
	if (this->scope && !this->scope->gc_reachable)
		this->scope->mark();
};


// CodeFunction prototype	
CodeFunctionPrototype::CodeFunctionPrototype() {		
	// table = new TreeObjectMap;
	type  = CODE_FUNCTION_PROTOTYPE;
};

void CodeFunctionPrototype::finalize(void) {
	table->finalize();
};

VirtualObject *CodeFunctionPrototype::get(Scope *scope, string *name) {
	return table->get(*name);
};

void CodeFunctionPrototype::put(Scope *scope, string *name, VirtualObject *value) {
	table->put(*name, value);
};

void CodeFunctionPrototype::remove(Scope *scope, string *name) {
	table->remove(*name);
};

bool CodeFunctionPrototype::contains(Scope *scope, string *name) {
	return table->contains(*name);
};

VirtualObject *CodeFunctionPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	string functionSource = objectStringValue(args[0]);
	
	char *instring = functionSource.toCString();
	FAKESTREAM  fs(instring);
	TokenStream ts;
	Parser       p;
	ts.init(&fs);
	p.init(&ts);
	
	ASTNode *tree = p.parse();
	free(instring);
	
	if (!tree) {
		scope->context->executer->raiseError("Parser error");
		return NULL;
	}
	
	tree->type = BLOCK;
	
	ASTNode *fnode = new ASTNode(-1, FUNCTION);
	fnode->addChild(tree);
	
	for (int i = 1; i < argc; ++i) {
		fnode->addLastObject(new string(objectStringValue(args[i])));
	}
	
	CodeFunction *f = new CodeFunction(scope, fnode);
	f->dynamical = 1;
	
	return f;
};

void CodeFunctionPrototype::mark(void) {
	if (gc_reachable)
		return;
	gc_reachable = 1;
	table->mark();
};

// Called on start. Defines CodeFunction prototype & type
void define_code_function(Scope *scope) {
	code_function_prototype = new CodeFunctionPrototype();
	scope->table->put(string("Function"), code_function_prototype);
	
	code_function_prototype->table->put(string("__typename"),   new String("Function"));
	// ==, != overloaded in Object
};

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
#include "Array.h"
#include "Error.h"
#include "Null.h"
#include "Undefined.h"
#include "NativeFunction.h"
#include "Boolean.h"
#include "StringType.h"
#include "ObjectConverter.h"
#include "TreeObjectMap.h"

#include "../TokenNamespace.h"
#include "../ASTExecuter.h"
#include "../string.h"

ErrorPrototype *error_prototype = NULL;

// Error type
Error::Error(Array *stacktrace, String *message) {		
	type  = ERROR;
	if (error_prototype)
		table->putAll(error_prototype->table);
	this->stacktrace = stacktrace;
	table->put(string("stacktrace"), (VirtualObject*) stacktrace);
	table->put(string("message"),    (VirtualObject*)    message);
};

Error::Error(Array *stacktrace) {		
	type  = ERROR;
	if (error_prototype)
		table->putAll(error_prototype->table);
	this->stacktrace = stacktrace;
	table->put(string("stacktrace"), (VirtualObject*) stacktrace);
};

void Error::finalize(void) {
	table->finalize();
};

VirtualObject *Error::get(Scope *scope, string *name) {
	return table->get(*name);
};

void Error::put(Scope *scope, string *name, VirtualObject *value) {
	table->put(*name, value);
};

void Error::remove(Scope *scope, string *name) {
	table->remove(*name);
};

bool Error::contains(Scope *scope, string *name) {
	return table->contains(*name);
};

VirtualObject *Error::call(Scope *scope, int argc, VirtualObject **args) {
	return NULL;
};

void Error::mark(void) {
	if (gc_reachable)
		return;
	gc_reachable = 1;
	table->mark();
};

static Error *Error::newInstance(Scope *scope, string message) {
	String *omessage = new String(message);
	return error_prototype->call(scope, 1, &omessage);
};

static Error *Error::newInstance(Scope *scope, string *message) {
	String *omessage = new String(message);
	return error_prototype->call(scope, 1, &omessage);
};

static Error *Error::newInstance(Scope *scope, const char *message) {
	String *omessage = new String(message);
	return error_prototype->call(scope, 1, &omessage);
};

void Error::print() {
	string smessage = "message";
	VirtualObject *message = this->get(NULL, &smessage);
	if (message)
		printf("Error: %S\n", objectStringValue(message).toCharSequence());
	else
		printf("Error\n");
	
	for (int i = 0; i < this->stacktrace->array->length; ++i) {
		VirtualObject *traceentry = this->stacktrace->array->get(i);
		if (!traceentry)
			continue;
		
		string lineno = "lineno";
		string file   = "file";
		string name   = "name";
		
		VirtualObject *olineno = traceentry->get(NULL, &lineno);
		VirtualObject *ofile   = traceentry->get(NULL, &file);
		VirtualObject *oname   = traceentry->get(NULL, &name);
		
		if (!olineno) {
			if (oname)
				printf("at %S()\n", objectStringValue(oname).toCharSequence());
			else
				printf("at <%S>\n", objectStringValue(ofile).toCharSequence());
		} else {
			if (oname)
				printf("at %S(%d)\n", objectStringValue(oname).toCharSequence(), objectIntValue(olineno));
			else
				printf("at <%S>[%d]\n", objectStringValue(ofile).toCharSequence(), objectIntValue(olineno));
		}
	}
};


// Error prototype	
ErrorPrototype::ErrorPrototype() {
	type = ERROR_PROTOTYPE;
};

void ErrorPrototype::finalize(void) {
	table->finalize();
};

VirtualObject *ErrorPrototype::get(Scope *scope, string *name) {
	return table->get(*name);
};

void ErrorPrototype::put(Scope *scope, string *name, VirtualObject *value) {
	table->put(*name, value);
};

void ErrorPrototype::remove(Scope *scope, string *name) {
	table->remove(*name);
};

bool ErrorPrototype::contains(Scope *scope, string *name) {
	return table->contains(*name);
};

VirtualObject *ErrorPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	Array *stacktrace = new Array;
	
	/* Old version. Used befure adding feature of copying stackrace
	// Iterate over all contexts
	Scope     *tscope = scope;
	Context      *ctx = NULL;
	ASTExecuter *exec = NULL;
	while (tscope) {
		if (ctx == tscope->context || exec == tscope->context->executer) {
			tscope = tscope->parent;
			continue;
		}
		ctx  = tscope->context;
		exec = ctx->executer;
		
		ASTExecuterStackTraceElement *sttemp     = tscope->context->executer->aststacktrace->head;
		ASTExecuterStackTraceElement *sttempprev = NULL;
		
		while (sttemp) {
			Object *tracenode = new Object;
			stacktrace->array->push(tracenode);
			
			if (sttempprev)
				tracenode->table->put(string("lineno"), new Integer(sttempprev->tracelineno));
			
			if (sttemp->type == ASTESTE_FILE) {
				tracenode->table->put(string("file"), new String(sttemp->tracename));
				sttemp = sttemp->next;
				sttempprev = NULL;
				continue;
			}
			
			tracenode->table->put(string("name"), new String(sttemp->type == ASTESTE_UNDEFINED ? string("<anonymous>") : sttemp->tracename));
			
			sttempprev = sttemp;
			sttemp     = sttemp->next;
		}
		
		if (sttempprev) {
			Object *tracenode = new Object;
			tracenode->table->put(string("lineno"), new Integer(sttempprev->tracelineno));
			tracenode->table->put(string("file"), new String(sttemp->tracename));
			stacktrace->array->push(tracenode);
		}
		
		tscope = tscope->parent;
	}
	*/
	
	
	if (!scope)
		return NULL;

	ASTExecuterStackTraceElement *sttemp     = scope->context->executer->aststacktrace->head;
	ASTExecuterStackTraceElement *sttempprev = NULL;
	
	while (sttemp) {
		Object *tracenode = new Object;
		stacktrace->array->push(tracenode);
		
		if (sttempprev)
			tracenode->table->put(string("lineno"), new Integer(sttempprev->tracelineno));
		
		if (sttemp->type == ASTESTE_FILE) {
			tracenode->table->put(string("file"), new String(sttemp->tracename));
			sttemp = sttemp->next;
			sttempprev = NULL;
			continue;
		}
		
		tracenode->table->put(string("name"), new String(sttemp->type == ASTESTE_UNDEFINED ? string("<anonymous>") : sttemp->tracename));
		
		sttempprev = sttemp;
		sttemp     = sttemp->next;
	}
	
	if (sttempprev) {
		Object *tracenode = new Object;
		tracenode->table->put(string("lineno"), new Integer(sttempprev->tracelineno));
		tracenode->table->put(string("file"), new String(sttemp->tracename));
		stacktrace->array->push(tracenode);
	}
	
	if (argc != 0)
		return new Error(stacktrace, new String(objectStringValue(args[0])));
	return new Error(stacktrace);
};

void ErrorPrototype::mark(void) {
	if (gc_reachable)
		return;
	gc_reachable = 1;
	table->mark();
};

// Operators
// printStackTrace()
static VirtualObject* function_printStackTrace(Scope *scope, int argc, VirtualObject **args) {
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
	// 5. parent scope object type == ERROR
	
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)) && o->type == ERROR)
		return NULL;
	
	
	printf("Incomplete code part at Error.cpp, line: %d\n", __LINE__);
	return NULL;
};

// print()
static VirtualObject* function_print(Scope *scope, int argc, VirtualObject **args) {
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
	// 5. parent scope object type == ERROR
	
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)) && o->type == ERROR)
		return NULL;
	
	
	Error *e = (Error*) o;
	
	string smessage = "message";
	VirtualObject *message = e->get(NULL, &smessage);
	if (message)
		printf("Error: %S\n", objectStringValue(message).toCharSequence());
	else
		printf("Error\n");
	
	for (int i = 0; i < e->stacktrace->array->length; ++i) {
		VirtualObject *traceentry = e->stacktrace->array->get(i);
		if (!traceentry)
			continue;
		
		string lineno = "lineno";
		string file   = "file";
		string name   = "name";
		
		VirtualObject *olineno = traceentry->get(NULL, &lineno);
		VirtualObject *ofile   = traceentry->get(NULL, &file);
		VirtualObject *oname   = traceentry->get(NULL, &name);
		
		if (!olineno) {
			if (oname)
				printf("at %S()\n", objectStringValue(oname).toCharSequence());
			else
				printf("at <%S>\n", objectStringValue(ofile).toCharSequence());
		} else {
			if (oname)
				printf("at %S(%d)\n", objectStringValue(oname).toCharSequence(), objectIntValue(olineno));
			else
				printf("at <%S>[%d]\n", objectStringValue(ofile).toCharSequence(), objectIntValue(olineno));
		}
	}
	
	return NULL;
};

// Called on start. Defines error prototype & type
void define_error(Scope *scope) {
	error_prototype = new ErrorPrototype();
	scope->table->put(string("Error"), error_prototype);
	
	error_prototype->table->put(string("__typename"),      new String("Error"));
	error_prototype->table->put(string("printStackTrace"), new NativeFunction(&function_printStackTrace));
	error_prototype->table->put(string("print"),           new NativeFunction(&function_print));
};


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

#include <cstdlib>
#include <cstdio>
#include <wchar.h>
#include <stdarg.h>
#include <locale.h>

#include "TokenNamespace.h"
#include "TokenStream.h"
#include "Parser.h"
#include "FakeStream.h"

#include "ASTExecuter.h"
#include "DebugUtils.h"
#include "ColoredOutput.h"
#include "TokenNamespace.h"
#include "string.h"
#include "LinkedMap.h"
#include "ptr_wrapper.h"

#include "objects/Object.h"
#include "objects/Array.h"
#include "objects/Scope.h"
#include "objects/Null.h"
#include "objects/Undefined.h"
#include "objects/StringType.h"
#include "objects/Integer.h"
#include "objects/Double.h"
#include "objects/Boolean.h"
#include "objects/NativeFunction.h"
#include "objects/CodeFunction.h"
#include "objects/Error.h"
#include "GarbageCollector.h"
#include "objects/ObjectConverter.h"

#include "exec_state.h"

#include "StackTrace.h"


// ASTExecuterElement
ASTExecuterElement::ASTExecuterElement() {
	attached_node = NULL;
	target        = NULL;
	next          = NULL;
	prev          = NULL;
	scope         = NULL;
	data          = 0;
};

void ASTExecuterElement::printData(void) {
	printf("FLAG_0: %d\n", (data & FLAG_0) >> (FLAGS_OFFSET + 5));
	printf("FLAG_1: %d\n", (data & FLAG_1) >> (FLAGS_OFFSET + 4));
	printf("FLAG_2: %d\n", (data & FLAG_2) >> (FLAGS_OFFSET + 3));
	printf("FLAG_3: %d\n", (data & FLAG_3) >> (FLAGS_OFFSET + 2));
	printf("FLAG_4: %d\n", (data & FLAG_4) >> (FLAGS_OFFSET + 1));
	printf("FLAG_5: %d\n", (data & FLAG_5) >> (FLAGS_OFFSET + 0));
	printf("DATA12: %d\n", (data & DATA10_MASK) >> DATA10_OFFSET);
	printf("USER16: %d\n",  data & USER16_MASK);
	printf("TARGET: %d\n",  target ? 1 : 0);
};


// ASTExecuterStack 
ASTExecuterStack::ASTExecuterStack(int mx_size) {
	top        = NULL;
	head       = NULL;
	bottom     = NULL;
	size       = 0;
	alloc_size = 0;
	max_size   = mx_size;
};

ASTExecuterStack::~ASTExecuterStack() {
	while (top) {
		ASTExecuterElement *tmp = top->next;
		delete top;
		top = tmp;
	}
};

// head->next instead of head
ASTExecuterElement *ASTExecuterStack::pop() {
	if (size == 0)
		return NULL;
	--size;
	head = head->next;
	
	return head;
};

void ASTExecuterStack::collect() {
	while (alloc_size > MIN_AST_STACK_SIZE && top != head) {
		ASTExecuterElement *temp = top->next;
		delete top;
		top = temp;
		top->prev = NULL;
		--alloc_size;
	}
};

int ASTExecuterStack::push() {
	if (head == NULL)
		if (bottom) {
			head = bottom;
			++size;
			return 1;
		} else {
			head   = new ASTExecuterElement;
			top    = head;
			bottom = head;
			++size;
			++alloc_size;
			return 1;
		}
	
	if (head->prev) {
		head = head->prev;
		++size;
		return 1;
	}
		
	if (size >= max_size)
		return 0;
	
	
	head->prev       = new ASTExecuterElement;
	head->prev->next = head;
	head             = head->prev;
	top              = head;
	++size;
	++alloc_size;
	return 1;
};


// ASTObjectElement 
ASTObjectElement::ASTObjectElement() {
	next             = NULL;
	prev             = NULL;
	object           = NULL;
	level            =    0;
};

ASTObjectElement::~ASTObjectElement() {};


// ASTObjectStack 
ASTObjectStack::ASTObjectStack(int mx_size) {
	top        = NULL;
	head       = NULL;
	bottom     = NULL;
	size       = 0;
	alloc_size = 0;
	max_size   = mx_size;
	GC.gc_attach_root(this);
};

ASTObjectStack::~ASTObjectStack() {
	while (top) {
		ASTObjectElement *tmp = top->next;
		delete top;
		top = tmp;
	}
	GC.gc_deattach_root(this);
};

void ASTObjectStack::popTillLevel(int level) {
	while (head && head->level > level) {
		--size;
		head->object =       NULL;
		head         = head->next;
	}
};

void ASTObjectStack::popForLevel(int level) {
	while (head && head->level >= level) {
		--size;
		head->object =       NULL;
		head         = head->next;
	}
};

VirtualObject *ASTObjectStack::pop(int level) {
	if (size == 0 || level < head->level)
		return NULL;
	
	--size;
	ASTObjectElement *e = head;
	head = head->next;
	
	VirtualObject *o = e->object;
	e->object        =      NULL;
	
	return o;
};

void ASTObjectStack::collect() {
	while (alloc_size > MIN_AST_STACK_SIZE && top != head) {
		ASTObjectElement *temp = top->next;
		delete top;
		top = temp;
		top->prev = NULL;
		--alloc_size;
	}
};

void ASTObjectStack::pullUp(int level) {	
	if (size)
		if (level >= head->level) {
			VirtualObject *o = head->object;
			head->object = NULL;
			popTillLevel(level - 1);
			push(o, level - 1);
		} else if (level < head->level) {
			popTillLevel(level - 1);
			push(new Undefined, level - 1);
		}
	else
		push(new Undefined, level - 1);
};

VirtualObject *ASTObjectStack::peek(int level) {
	if (size == 0 || level > head->level)
		return NULL;
	
	return head->object;
};

VirtualObject *ASTObjectStack::peek(int offset, int level) {
	if (size == 0 || offset > size || level > head->level)
		return NULL;
	
	ASTObjectElement *e = head;
	while (offset) {
		e = e->next;
		--offset;
		if (head == NULL || level > head->level)
			return NULL;
	}
	
	return e->object;
};

int ASTObjectStack::push(VirtualObject *o, int level) {
	// printf("ASTObjectStack::push(%d)\n", o);
	// if (o == 12)
	// 	throw;
	// print_stacktrace(stdout);
	popForLevel(level + 1);
	
	if (head == NULL)
		if (bottom) {
			head         = bottom;
			head->object = o;
			head->level  = level;
			++size;
			return 1;
		} else {
			head         = new ASTObjectElement;
			top          = head;
			bottom       = head;
			head->object = o;
			head->level  = level;
			++size;
			++alloc_size;
			return 1;
		}
	
	if (head->prev) {
		head         = head->prev;
		head->object = o;
		head->level  = level;
		++size;
		return 1;
	}
		
	if (size >= max_size)
		return 0;
	
	head->prev       = new ASTObjectElement;
	head->prev->next = head;
	head             = head->prev;
	head->object     = o;
	head->level      = level;
	top              = head;
	++size;
	++alloc_size;
	return 1;
};

void ASTObjectStack::print(void) {
	printf("AST OBJECT STACK:\n");
	printf(" ---------------------------------\n");
	
	if (head) {
		ASTObjectElement *e = bottom;
		
		do {
			printf("| [%04d] ", e->level);
			objectStringValue(e->object).print();
			putchar('\n');
			printf(" ---------------------------------\n");
		
			e = e->prev;
		} while (e && e->next != head);
	} else
		printf("| EMPTY\n ---------------------------------\n");
};

void ASTObjectStack::mark() {
	gc_reachable = 1;
	
	ASTObjectElement *e = head;
	
	if (!e)
		return;
	
	while (1) {
		// printf("%d %d %d\n", e, e->object, !e->object);
		if (e->object && !e->object->gc_reachable)
			e->object->mark();
		
		e = e->next;
		if (!e)
			break;
	}
};

void ASTObjectStack::finalize() {};


// ASTExecuterStackTrace	
ASTExecuterStackTraceElement::ASTExecuterStackTraceElement() {
	this->next        = NULL;
	this->prev        = NULL;
	this->tracelineno = -1;
	this->level       = (unsigned int) (-1) >> 1;
	this->type        = ASTESTE_UNDEFINED;
	this->tracename   = "";
};
	
// -- - ASTExecuterStackTraceElement::~ASTExecuterStackTraceElement() {
// -- - (Г *w*) Г     .`#`.(*O*) < OMAE WA MOU, SHINDEIRU        
// -- -    || \         |  _//_/
// -- -    ||  NANI     |/ //
// -- -   /  \           /  \
// -- - };

// StackTrace stack.
// Used for storing hierarchical function call 
// names & line numbers for further debug & stack 
// trace generation on raising errors.
ASTExecuterStackTrace::ASTExecuterStackTrace(int mx_size) {
	top        = NULL;
	head       = NULL;
	bottom     = NULL;
	pos_size   = 0;
	size       = 0;
	alloc_size = 0;
	max_size   = mx_size;
};

ASTExecuterStackTrace::~ASTExecuterStackTrace() {
	while (top) {
		ASTExecuterStackTraceElement *tmp = top->next;
		delete top;
		top = tmp;
	}
};

int ASTExecuterStackTrace::insert(int lineno, string name) {
	if (alloc_size >= max_size)
		return 0;
	
	auto new_bottom         = new ASTExecuterStackTraceElement;
	new_bottom->type        = ASTESTE_NAME; 
	new_bottom->tracename   = name;
	new_bottom->tracelineno = lineno;
	new_bottom->level       = pos_size - size - 1;
	new_bottom->prev        = bottom;
	if (bottom)
		bottom->next        = new_bottom;
	bottom                  = new_bottom;
	++size;
	++alloc_size;
	return 1;
};

int ASTExecuterStackTrace::insert(int lineno) {
	if (alloc_size >= max_size)
		return 0;
	
	auto new_bottom         = new ASTExecuterStackTraceElement;
	new_bottom->type        = ASTESTE_UNDEFINED; 
	new_bottom->tracelineno = lineno;
	new_bottom->level       = pos_size - size - 1;
	new_bottom->prev        = bottom;
	if (bottom)
		bottom->next        = new_bottom;
	bottom                  = new_bottom;
	++size;
	++alloc_size;
	return 1;
};

int ASTExecuterStackTrace::insert(string file) {
	if (alloc_size >= max_size)
		return 0;
	
	auto new_bottom         = new ASTExecuterStackTraceElement;
	new_bottom->type        = ASTESTE_FILE; 
	new_bottom->tracename   = file;
	new_bottom->tracelineno = -1;
	new_bottom->level       = pos_size - size - 1;
	new_bottom->prev        = bottom;
	if (bottom)
		bottom->next        = new_bottom;
	bottom                  = new_bottom;
	++size;
	++alloc_size;
	return 1;
};

void ASTExecuterStackTrace::desert() {
	if (bottom && bottom->level < 0) {
		auto temp = bottom;
		bottom = bottom->prev;
		bottom->next = NULL;
		--size;
		--alloc_size;
		delete temp;
	}
};

void ASTExecuterStackTrace::normalize() {
	while (bottom && bottom->level < 0) {
		auto temp = bottom;
		bottom = bottom->prev;
		bottom->next = NULL;
		--size;
		--alloc_size;
		delete temp;
	}
};

void ASTExecuterStackTrace::clear() {
	while (top) {
		auto temp = top;
		top = top->next;
	}
	size       = 0;
	alloc_size = 0;
	pos_size   = 0;
};

void ASTExecuterStackTrace::pop(int level) {
	if (pos_size == 0 || level < head->level)
		return;
	// printf("POP = %d %S\n", level, head->tracename.toCharSequence());
	--size;
	--pos_size;
	head->level = (unsigned int) (-1) >> 1;
	head = head->next;
};

void ASTExecuterStackTrace::popForLevel(int level) {
	while (head && head->level >= level) {
		--size;
		--pos_size;
		head->level = (unsigned int) (-1) >> 1;
		head        = head->next;
	}
};

void ASTExecuterStackTrace::collect() {
	while (alloc_size > MIN_AST_STACK_SIZE && top != head && head->level >= 0) {
		ASTExecuterStackTraceElement *temp = top->next;
		delete top;
		top = temp;
		top->prev = NULL;
		--alloc_size;
	}
};

int ASTExecuterStackTrace::push(int level, int lineno, string name) {
	// Override current level value.
	popForLevel(level);
	
	// printf("PUSH::<level><name> = %d %S\n", level, name.toCharSequence());
	
	if (head == NULL)
		if (bottom) {
			head              = bottom;
			head->type        = ASTESTE_NAME; 
			head->tracename   = name;
			head->tracelineno = lineno;
			head->level       = level;
			++size;
			++pos_size;
			return 1;
		} else {
			head              = new ASTExecuterStackTraceElement;
			top               = head;
			bottom            = head;
			head->type        = ASTESTE_NAME; 
			head->tracename   = name;
			head->tracelineno = lineno;
			head->level       = level;
			++size;
			++pos_size;
			++alloc_size;
			return 1;
		}
	
	if (head->prev) {
		head              = head->prev;
		head->type        = ASTESTE_NAME; 
		head->tracename   = name;
		head->tracelineno = lineno;
		head->level       = level;
		++size;
		++pos_size;
		return 1;
	}
		
	if (size >= max_size)
		return 0;
	
	head->prev        = new ASTExecuterStackTraceElement;
	head->prev->next  = head;
	head              = head->prev;
	head->type        = ASTESTE_NAME; 
	head->tracename   = name;
	head->tracelineno = lineno;
	head->level       = level;
	top               = head;
	++size;
	++pos_size;
	++alloc_size;
	return 1;
};

int ASTExecuterStackTrace::push(int level, int lineno) {
	// Override current level value.
	popForLevel(level);
	
	// printf("PUSH::<level> = %d\n", level);
	
	if (head == NULL)
		if (bottom) {
			head              = bottom;
			head->type        = ASTESTE_UNDEFINED; 
			head->tracelineno = lineno;
			head->level       = level;
			++size;
			++pos_size;
			return 1;
		} else {
			head              = new ASTExecuterStackTraceElement;
			top               = head;
			bottom            = head;
			head->type        = ASTESTE_UNDEFINED; 
			head->tracelineno = lineno;
			head->level       = level;
			++size;
			++pos_size;
			++alloc_size;
			return 1;
		}
	
	if (head->prev) {
		head              = head->prev;
		head->type        = ASTESTE_UNDEFINED; 
		head->tracelineno = lineno;
		head->level       = level;
		++size;
		++pos_size;
		return 1;
	}
		
	if (size >= max_size)
		return 0;
	
	head->prev        = new ASTExecuterStackTraceElement;
	head->prev->next  = head;
	head              = head->prev;
	head->type        = ASTESTE_UNDEFINED; 
	head->level       = level;
	top               = head;
	++size;
	++pos_size;
	++alloc_size;
	return 1;
};

int ASTExecuterStackTrace::push(int level, string file) {
	// Override current level value.
	popForLevel(level);
	
	// printf("PUSH::<level><file> = %d %S\n", level, file.toCharSequence());
	
	if (head == NULL)
		if (bottom) {
			head              = bottom;
			head->type        = ASTESTE_FILE; 
			head->tracename   = file;
			head->tracelineno = -1;
			head->level       = level;
			++size;
			++pos_size;
			return 1;
		} else {
			head              = new ASTExecuterStackTraceElement;
			top               = head;
			bottom            = head;
			head->type        = ASTESTE_FILE; 
			head->tracename   = file;
			head->tracelineno = -1;
			head->level       = level;
			++size;
			++pos_size;
			++alloc_size;
			return 1;
		}
	
	if (head->prev) {
		head              = head->prev;
		head->type        = ASTESTE_FILE; 
		head->tracename   = file;
		head->tracelineno = -1;
		head->level       = level;
		++size;
		++pos_size;
		return 1;
	}
		
	if (size >= max_size)
		return 0;
	
	head->prev        = new ASTExecuterStackTraceElement;
	head->prev->next  = head;
	head              = head->prev;
	head->type        = ASTESTE_FILE; 
	head->tracename   = file;
	head->tracelineno = -1;
	head->level       = level;
	top               = head;
	++size;
	++pos_size;
	++alloc_size;
	
	return 1;
};


// ASTExecuter 
ASTExecuter::ASTExecuter() {
	_error        = 0;
	aststack      = new   ASTExecuterStack(AST_EXECUTION_STACK_SIZE);
	astobjstack   = new       ASTObjectStack(AST_OBJECT_STACK_SIZE);
	aststacktrace = new ASTExecuterStackTrace(AST_STACK_TRACE_SIZE);
	error_handler = NULL;
	// Also this instance will not work without main constituent - Duck.
	//      __
	//  ___( o)>
	//  \ <_. )
	//   `---'
	// Trust me, i'm engineer
};

ASTExecuter::~ASTExecuter() {
	delete aststack;
	delete astobjstack;
	delete aststacktrace;
	GC.gc_collect();
};

int ASTExecuter::insertStackTrace(ASTExecuter *executer) {
	if (!executer)
		return 0;
	if (executer->aststacktrace->size + this->aststacktrace->size > this->aststacktrace->max_size)
		return 0;
	
	auto temp = executer->aststacktrace->head;
	while (temp) {
		if (temp->type == ASTESTE_NAME)
			this->aststacktrace->insert(temp->tracelineno, temp->tracename);
		if (temp->type == ASTESTE_UNDEFINED)
			this->aststacktrace->insert(temp->tracelineno);
		if (temp->type == ASTESTE_FILE)
			this->aststacktrace->insert(temp->tracename);
		
		temp = temp->next;
	}
	
	return 1;
};

// Handles unresolvable error while trying to push stack with no space.
void stackoverflow_error(ASTExecuter *executer, int stack_type) {
	if (stack_type == OBJECT_STACK)
		printf("Object stack overflow\n");
	if (stack_type == EXECUTION_STACK)
		printf("Execution stack overflow\n");
	if (stack_type == STACKTRACE_STACK)
		printf("StackTrace stack overflow\n");
	
	// XXX: Raise error
};

void ASTExecuter::execution_error(const char *msg) {
	chighred;
	printf("Execution error: %s\n", msg);
	cwhite;
	
	_error = 1;
};

// Without reference object
// use: nativeCall(caller_scope, func, 3, *obj1, *obj2, *obj3)
//                                        ^      ^      ^ - pointers
void ASTExecuter::nativeCall(Scope *caller_scope, VirtualObject *f, int argc, ...) {
	// Explicit - because function & parameters are passed as is
	// Implicit - because execution with FLAG_0 in implicit mode
	
	if (f == NULL) {
		if (!astobjstack->push(new Undefined, aststack->size)) 
			stackoverflow_error(this, OBJECT_STACK);
		return;
	}
	
	va_list args;
	va_start(args, argc);
	
	if (!aststack->push()) {
		stackoverflow_error(this, OBJECT_STACK);
		return;
	}
	
	
	if (!astobjstack->push(NULL, aststack->size)) {
		stackoverflow_error(this, OBJECT_STACK);
		return;
	}
	if (!astobjstack->push(f, aststack->size)) {
		stackoverflow_error(this, OBJECT_STACK);
		return;
	}
	for (int i = 0; i < argc; ++i)
		astobjstack->push(va_arg(args, VirtualObject*), aststack->size);
	
	va_end(args);
	
	// Call by type, not node
	aststack->head->data |= (NATIVE_CALL << DATA10_OFFSET);
	// Force execution with NULL target
	aststack->head->data |= FLAG_0;
	// Assign execution scope
	aststack->head->scope = caller_scope;
};

// With referennce object
// use: nativeCall(caller_scope, ref, func, 3, *obj1, *obj2, *obj3)
//                                             ^      ^      ^ - pointers
void ASTExecuter::nativeCall(Scope *caller_scope, VirtualObject *r, VirtualObject *f, int argc, ...) {
	// Explicit - because function & parameters are passed as is
	// Implicit - because execution with FLAG_0 in implicit mode
	
	if (f == NULL) {
		if (!astobjstack->push(new Undefined, aststack->size))
			stackoverflow_error(this, OBJECT_STACK);
		return;
	}
	
	va_list args;
	va_start(args, argc);
	
	if (!aststack->push()) {
		stackoverflow_error(this, EXECUTION_STACK);
		return;
	}
	
	if (!astobjstack->push(r, aststack->size)) {
		stackoverflow_error(this, OBJECT_STACK);
		return;
	}
	if (!astobjstack->push(f, aststack->size)) {
		stackoverflow_error(this, OBJECT_STACK);
		return;
	}
	for (int i = 0; i < argc; ++i)
		if (!astobjstack->push(va_arg(args, VirtualObject*), aststack->size)) {
			stackoverflow_error(this, OBJECT_STACK);
			return;
		}
	
	va_end(args);
	
	// Call by type, not node
	aststack->head->data |= (NATIVE_CALL << DATA10_OFFSET);
	// Force execution with NULL target
	aststack->head->data |= FLAG_0;
	// Assign execution scope
	aststack->head->scope = caller_scope;
};

void ASTExecuter::binaryOperatorCall(ASTExecuterElement *element, ASTNode *node, int depth, const char *opmember) {
	// FLAG_1: left calculated
	// FLAG_2: right calculated
	// FLAG_3: result in stack expected
	
	if (element->target == NULL)
		if (element->data & FLAG_1)
			return;
		else {
			element->scope  = element->next ? element->next->scope : NULL;
			element->data  |= FLAG_1;
			element->target = node->left;
		}
	else if (element->data & FLAG_2) {
		// left & Right calculated
		element->data |=  FLAG_0;
		element->data &= ~FLAG_2;
		element->data |=  FLAG_3;
		
		if (!astobjstack->head->next) {
			raiseError("Execution Failture on operator");
			return;
		}
		
		if (!astobjstack->head->next->object) {
			astobjstack->push(NULL, depth);
			return;
		}
		
		string key = string(opmember);
		// Save called name
		if (!aststacktrace->push(depth, node->lineno, key)) {
			stackoverflow_error(this, STACKTRACE_STACK);
			return;
		}
		nativeCall(element->scope, astobjstack->head->next->object->get(element->scope, &key), 2, astobjstack->head->next->object, astobjstack->head->object);
		
	} else if (element->data & FLAG_3) {
		// Result expected
		element->data  &= ~FLAG_0;
		element->target = NULL;
		
		astobjstack->pullUp(depth);
		
	} else {
		// Left calculated
		element->target = element->target->next;
		element->data  |= FLAG_2;
	}
};

void ASTExecuter::binaryLogicalOperatorCall(ASTExecuterElement *element, ASTNode *node, int depth, const char *opmember) {
	// FLAG_1: left calculated
	// FLAG_2: right calculated
	// FLAG_3: result in stack expected
	
	if (element->target == NULL)
		if (element->data & FLAG_1)
			return;
		else {
			element->scope  = element->next ? element->next->scope : NULL;
			element->data  |= FLAG_1;
			element->target = node->left;
		}
	else if (element->data & FLAG_2) {
		// left & Right calculated
		element->data |=  FLAG_0;
		element->data &= ~FLAG_2;
		element->data |=  FLAG_3;
		
		if (!astobjstack->head->next) {
			raiseError("Execution Failture on operator");
			return;
		}
		
		if (!astobjstack->head->next->object) {
			astobjstack->push(NULL, depth);
			return;
		}
		
		string key = string(opmember);
		// Save called name
		if (!aststacktrace->push(depth, node->lineno, key)) {
			stackoverflow_error(this, STACKTRACE_STACK);
			return;
		}
		nativeCall(element->scope, astobjstack->head->next->object->get(element->scope, &key), 2, astobjstack->head->next->object, astobjstack->head->object);
		
	} else if (element->data & FLAG_3) {
		// Result expected
		element->data  &= ~FLAG_0;
		element->target = NULL;
		
		astobjstack->pullUp(depth);
		
	} else {
		// Left calculated
		if (opmember == "__operator||" && objectIntValue(astobjstack->peek(depth))) {
			if (!astobjstack->push(new Boolean(1), depth - 1)){
				stackoverflow_error(this, OBJECT_STACK);
				return;
			}
			element->target = NULL;
		} else if (opmember == "__operator&&" && !objectIntValue(astobjstack->peek(depth))) {
			if (!astobjstack->push(new Boolean(0), depth - 1)) {
				stackoverflow_error(this, OBJECT_STACK);
				return;
			}
			element->target = NULL;
		} else {
			element->target = element->target->next;
			element->data  |= FLAG_2;
		}
	}
};

void ASTExecuter::unaryOperatorCall(ASTExecuterElement *element, ASTNode *node, int depth, const char *opmember) {
	// FLAG_1: left calculated
	// FLAG_2: result in stack expected
	
	if (element->target == NULL)
		if (element->data & FLAG_1)
			return;
		else {
			element->scope  = element->next ? element->next->scope : NULL;
			element->data  |= FLAG_1;
			element->target = node->left;
		}
	else if (element->data & FLAG_2) {
		// Result expected
		element->data  &= ~FLAG_0;
		element->target = NULL;
		
		astobjstack->pullUp(depth);
		
	} else {
		// Left calculated
		element->data  |= FLAG_0;
		element->data  |= FLAG_2;
		
		if (!astobjstack->head->next) {
			raiseError("Execution Failture on operator");
			return;
		}
		
		if (!astobjstack->head->next->object) {
			astobjstack->push(NULL, depth);
			return;
		}
		
		string key = string(opmember);
		// Save called name
		if (!aststacktrace->push(depth, node->lineno, key)) {
			stackoverflow_error(this, STACKTRACE_STACK);
			return;
		}
		nativeCall(element->scope, astobjstack->head->object->get(element->scope, &key), 1, astobjstack->head->object);
	}
};

void ASTExecuter::raiseError(const char *message) {	
	
	// Reach enclosing try/catch
	ASTExecuterElement *enclosing_try = NULL;
	ASTExecuterElement *e             = aststack->head;
	
	while (1) {
		if (!e) 
			break;
		
		if (e->attached_node && e->attached_node->type == TRY && !(e->data & FLAG_2)) {
			enclosing_try = e;
			break;
		}
		
		e = e->next;
	}
	
	if (!enclosing_try) {
		Error *error = Error::newInstance(aststack->head->scope, message);
		GC.gc_lock(error);	
		
		// Clear all stack till level of the ASTROOT
		while (aststack->head && aststack->head->attached_node && aststack->head->attached_node->type != ASTROOT) {
			leave(aststack->head, aststack->head->attached_node, aststack->size);
			aststack->pop();
		}
		
		if (error_handler) {
			error_handler(error);
		} else {
			string Runtime_key = "Runtime";
			VirtualObject *Runtime_object = aststack->head->scope->get(NULL, &Runtime_key);
			
			if (!Runtime_object) {
				error->print();
				_interpreter_exit(1);
				return;
			}
			
			string defaultErrorHandler_key = "defaultErrorHandler";
			VirtualObject *defaultErrorHandler_object = Runtime_object->get(NULL, &defaultErrorHandler_key);
			
			if (!defaultErrorHandler_object) {
				error->print();
				_interpreter_exit(1);
				return;
			}
			
			nativeCall(aststack->head->scope, defaultErrorHandler_object, 1, error);
		}
		
		GC.gc_lock(error);
		return;
	}

	
	// Build error instance
	if (enclosing_try->attached_node->objectlist) { // Have expect (name)
		String *omessage = new String(message);
		
		// Push error into try/catch & jump on catch node
		Error *error = error_prototype->call(NULL, 1, &omessage);
	
		enclosing_try->scope = new Scope(enclosing_try->scope);
		GC.gc_attach_root(enclosing_try->scope);
		enclosing_try->data |= FLAG_3;
		enclosing_try->scope->define((string*) enclosing_try->attached_node->objectlist->object, error);
	}
	
	enclosing_try->data |= FLAG_0 | FLAG_2;
	while (aststack->head != enclosing_try) {
		leave(aststack->head, aststack->head->attached_node, aststack->size);
		aststack->pop();
	}
	
	/*
	chighred;
	while (e) {
		if (e->attached_node) {
			printf("Error at line %d: ", e->attached_node->lineno);
			break;
		}
		e = e->next;
	}
	if (!e)
		printf("Error: ");
	
	printf("%s\n", message);
	creset;
	*/
};

void ASTExecuter::raiseError(string message) {	
	
	// Reach enclosing try/catch
	ASTExecuterElement *enclosing_try = NULL;
	ASTExecuterElement *e             = aststack->head;
	
	while (1) {
		if (!e) 
			break;
		
		if (e->attached_node && e->attached_node->type == TRY && !(e->data & FLAG_2)) {
			enclosing_try = e;
			break;
		}
		
		e = e->next;
	}
	
	if (!enclosing_try) {
		Error *error = Error::newInstance(aststack->head->scope, message);
		GC.gc_lock(error);	
		
		// Clear all stack till level of the ASTROOT
		while (aststack->head && aststack->head->attached_node && aststack->head->attached_node->type != ASTROOT) {
			leave(aststack->head, aststack->head->attached_node, aststack->size);
			aststack->pop();
		}
		
		if (error_handler) {
			error_handler(error);
		} else {
			string Runtime_key = "Runtime";
			VirtualObject *Runtime_object = aststack->head->scope->get(NULL, &Runtime_key);
			
			if (!Runtime_object) {
				error->print();
				_interpreter_exit(1);
				return;
			}
			
			string defaultErrorHandler_key = "defaultErrorHandler";
			VirtualObject *defaultErrorHandler_object = Runtime_object->get(NULL, &defaultErrorHandler_key);
			
			if (!defaultErrorHandler_object) {
				error->print();
				_interpreter_exit(1);
				return;
			}
			
			nativeCall(aststack->head->scope, defaultErrorHandler_object, 1, error);
		}
		
		GC.gc_unlock(error);
		return;
	}

	// Build error instance
	if (enclosing_try->attached_node->objectlist) { // Have expect (name)
		String *omessage = new String(message);
		
		
		// Push error into try/catch & jump on catch node
		Error *error = error_prototype->call(NULL, 1, &omessage);
	
		enclosing_try->scope = new Scope(enclosing_try->scope);
		GC.gc_attach_root(enclosing_try->scope);
		enclosing_try->data |= FLAG_3;
		enclosing_try->scope->define((string*) enclosing_try->attached_node->objectlist->object, error);
	}
	
	enclosing_try->data |= FLAG_0 | FLAG_2;
	while (aststack->head != enclosing_try) {
		leave(aststack->head, aststack->head->attached_node, aststack->size);
		aststack->pop();
	}
	
	/*
	chighred;
	while (e) {
		if (e->attached_node) {
			printf("Error at line %d: ", e->attached_node->lineno);
			break;
		}
		e = e->next;
	}
	if (!e)
		printf("Error: ");
	
	printf("%s\n", message);
	creset;
	*/
};

void ASTExecuter::raiseError(VirtualObject *error) {	
	
	// Reach enclosing try/catch
	ASTExecuterElement *enclosing_try = NULL;
	ASTExecuterElement *e             = aststack->head;
	
	while (1) {
		if (!e) 
			break;
		
		if (e->attached_node && e->attached_node->type == TRY && !(e->data & FLAG_2)) {
			enclosing_try = e;
			break;
		}
		
		e = e->next;
	}
	
	if (!enclosing_try) {	
		GC.gc_lock(error);	
		// Clear all stack till level of the ASTROOT
		while (aststack->head && aststack->head->attached_node && aststack->head->attached_node->type != ASTROOT) {
			leave(aststack->head, aststack->head->attached_node, aststack->size);
			aststack->pop();
		}
		
		if (error_handler) {
			error_handler(error);
		} else {	
			string Runtime_key = "Runtime";
			VirtualObject *Runtime_object = aststack->head->scope->get(NULL, &Runtime_key);
			
			if (!Runtime_object) {
				if (error && error->type == ERROR)
					((Error*) error)->print();
				else
					printf("%S\n", objectStringValue(error).toCharSequence());
				_interpreter_exit(1);
				return;
			}
			
			string defaultErrorHandler_key = "defaultErrorHandler";
			VirtualObject *defaultErrorHandler_object = Runtime_object->get(NULL, &defaultErrorHandler_key);
			
			if (!defaultErrorHandler_object) {
				if (error && error->type == ERROR)
					((Error*) error)->print();
				else
					printf("%S\n", objectStringValue(error).toCharSequence());
				_interpreter_exit(1);
				return;
			}
			
			nativeCall(aststack->head->scope, defaultErrorHandler_object, 1, error);
		}
		
		GC.gc_unlock(error);
		return;
	}
	
	// Build error instance
	if (enclosing_try->attached_node->objectlist) { // Have expect (name)	
		enclosing_try->scope = new Scope(enclosing_try->scope);
		GC.gc_attach_root(enclosing_try->scope);
		enclosing_try->data |= FLAG_3;
		enclosing_try->scope->define((string*) enclosing_try->attached_node->objectlist->object, error);
	}
	
	enclosing_try->data |= FLAG_0 | FLAG_2;
	while (aststack->head != enclosing_try) {
		leave(aststack->head, aststack->head->attached_node, aststack->size);
		aststack->pop();
	}
};

// Checks if object is defined value
static bool defined(VirtualObject *o) {
	return o != NULL && o->type != UNDEFINED && o->type != TNULL;
};

static const char *operator_asign_string(int optoken) {
	switch(optoken) {
		case ASSIGN_ADD:
			return "__operator+";
		case ASSIGN_SUB: 
			return "__operator-";
		case ASSIGN_MUL: 
			return "__operator*";
		case ASSIGN_DIV: 
			return "__operator/";
		case ASSIGN_BITRSH: 
			return "__operator>>";
		case ASSIGN_BITURSH: 
			return "__operator>>>";
		case ASSIGN_BITLSH: 
			return "__operator<<";
		case ASSIGN_BITAND: 
			return "__operator&";
		case ASSIGN_BITOR: 
			return "__operator|";
		case ASSIGN_BITXOR: 
			return "__operator^";
		case ASSIGN_MDIV: 
			return "__operator\\\\";
		case ASSIGN_MOD:
			return "__operator%";
	};
};

// Called every time node is visited
void ASTExecuter::visit(ASTExecuterElement *element, ASTNode *node, int depth) {
	
	// DATA12_SECTION can be used for passing node type without passing node
	if (node == NULL && !((element->data & DATA10_MASK) >> DATA10_OFFSET)) 
		execution_error("NODE = NULL");
	
	// // Reference counting sucks at loops
	// // Now i say: deal with this scheme \/
	
	// Inserting value into stack makes value->point()
	// Removing value from stack makes value->dispose()
	// Else:
	// a = this + 10;
	// a
	//  \
	//   \
	//    +          <------- dispose this, dispose 10
	//   / \
	//  /   \
	// this  10      <------- 
	// this has only 1 pointer: root_scope
	// calling object@this->dispose()
	// will delete root_scope
	
	// a.foo;
	//             |      <------ dispose value of foo field of a
	//             .      <------ returns value of foo field of a
	//            / \
	//           /   \
	//          a     foo
	// foo has only 1 pointer: a.foo
	// calling object@foo->dispose() 
	// will delete foo
	
	
#ifdef AST_VISIT_PRINT
	csetcolor(CYELLOW);
	DEBUG_IDENT(depth - 1)
	printf(tokenToString(node ? node->type : ((element->data & DATA10_MASK) >> DATA10_OFFSET)));
	putchar(10);
	cwhite;
#endif
	
	switch (node ? node->type : ((element->data & DATA10_MASK) >> DATA10_OFFSET)) {
		case ASTROOT: {
			// Clear stack
			astobjstack->popTillLevel(depth);
			// DO sam shet
			if (element->target == NULL)
				if (element->data & FLAG_1)
					return;
				else {
					element->target = node->left;
					element->data  |= FLAG_1;
				}
			else 
				element->target = element->target->next;
			
			if (!aststacktrace->head || aststacktrace->head->level != depth)
				aststacktrace->push(depth, string(element->scope->context->script_file_path->path));
			
			break;
		}
		
		case IMPORTED_SCRIPT: {
			// Clear stack
			astobjstack->popTillLevel(0);
			// DO sam shet #2
			if (element->target == NULL)
				return;
			else 
				element->target = element->target->next;
			
			if (!aststacktrace->head || aststacktrace->head->level != depth)
				aststacktrace->push(depth, string(element->scope->context->script_file_path->path));
			
			break;
		}
		
		case DEFINE: {
			// FLAG_1:      executed
			// USER16_DATA: index of current variable
			
			if (element->target == NULL)
				if (element->data & FLAG_1)
					return;
				else {
					element->scope = element->next ? element->next->scope : NULL;
					element->data |= FLAG_1;
					
					if (element->scope) {
						element->target = node->left;
						
						if (!element->target && node->objectlist) {
							ASTObjectList *aol_type = node->objectlist;
							ASTObjectList *aol_name = node->objectlist->next;
							
							while (1) {
								element->scope->define((string*) aol_name->object, new Undefined);
								
								if (aol_name->next) {
									aol_type = aol_name->next;
									aol_name = aol_type->next;
								} else
									break;
							}
							return;
						}
						
						++element->data;
					} else
						element->target = NULL;
				}
			else {
				if (element->target) {
					element->target = element->target->next;
					++element->data;
				}
				
				if (element->target)
					return;
				
				int n = element->data & USER16_MASK;
				
				// At least one variable
				ASTObjectList *aol_type = node->objectlist;
				ASTObjectList *aol_name = node->objectlist->next;
				
				while (1) {
					int type = *(int*) aol_type->object;
					if (type & 0b1000) {
						// Value exists
						VirtualObject *o = astobjstack->peek(depth);
						o = o ? o : new Undefined;
						element->scope->define((string*) aol_name->object, o);
						astobjstack->pop(depth);
					} else 
						element->scope->define((string*) aol_name->object, new Undefined);
					
					if (aol_name->next) {
						aol_type = aol_name->next;
						aol_name = aol_type->next;
					} else
						break;
				}
			}
			
			break;
		}
		
		case BLOCK: {
			if (element->target == NULL)
				if (element->data & FLAG_1)
					return;
				else {
					if (!element->next) {
						raiseError("Execution break. Scope == NULL.");
						element->data  |= FLAG_1;
						element->target = NULL;
					}
					
					element->scope  = new Scope(element->next ? element->next->scope : NULL);
					GC.gc_attach_root(element->scope);
					
					element->target = node->left;
					element->data  |= FLAG_1;
				}
			else 
				element->target = element->target->next;
			
			break;
		}
		
		case IF: {
			// FLAG_1: condition calculated
			// FLAG_2: executed
			
			if (element->data & FLAG_1) {
				if (element->data & FLAG_2) 
					element->target = NULL;
				else {
					if (objectIntValue(astobjstack->peek(depth))) {
						astobjstack->popForLevel(depth);
						element->target = element->target->next;
					} else 
						element->target = element->target->next->next;
					
					element->data  |= FLAG_2;
				}
			} else {
				element->scope  = element->next ? element->next->scope : NULL;
				element->target = node->left;
				element->data  |= FLAG_1;
			}
			
			break;
		}
		
		case WHILE: {
			// FLAG_1: condition calculated
			
			// Loop terminator
			if (element->data & FLAG_5) {
				element->data  &= ~FLAG_0;
				element->target = NULL;
				return;
			}	
			
			// Continue terminator
			if (element->data & FLAG_0) 
				element->data &= ~FLAG_0;
			
			if (element->data & FLAG_1) {				
				if (objectIntValue(astobjstack->peek(depth))) {
					astobjstack->popForLevel(depth);
					
					// Try to collect garbage
					if (node->right->type != BLOCK)
						GC.gc_collect();
					
					element->target = node->right;
					// Force to recalculate the flag
					element->data  &= ~FLAG_1;
				} else
					element->target = NULL;
			} else {
				element->scope  = element->next ? element->next->scope : NULL;
				element->target = node->left;
				element->data  |= FLAG_1;
			}
			
			break;
		}
		
		case FOR: {
			// FLAG_1: initialization calculated
			// FLAG_2: condition calculated
			// FLAG_3: block executed
			// FLAG_4: increment executed
			
			// Loop terminator
			if (element->data & FLAG_5) {
				element->data  &= ~FLAG_0;
				element->target = NULL;
				return;
			}
			
			// Continue terminator
			if (element->data & FLAG_0) 
				element->data &= ~FLAG_0;
			
			if (element->target == NULL) {
				element->data  |= FLAG_1;
				element->target = node->left;
				element->scope  = new Scope(element->next ? element->next->scope : NULL);
				GC.gc_attach_root(element->scope);
				return;
			}
			
			if (element->data & FLAG_1) {
				// Initialization calculated
				element->data  &= ~FLAG_1;
				element->data  |=  FLAG_2;
				// Jump on condition
				element->target = node->left->next;
			} else if (element->data & FLAG_2) {
				// Condition calculated
				VirtualObject *o = astobjstack->peek(depth);
				if (!objectIntValue(o)) {
					element->target = NULL;
					return;
				}
				element->data  &= ~FLAG_2;
				element->data  |=  FLAG_3;
				// Jump on block
				element->target = node->left->next->next->next;
			} else if (element->data & FLAG_3) {
				// Block executed
				element->data  &= ~FLAG_3;
				element->data  |=  FLAG_4;
				
				// Try to collect garbage
				astobjstack->popForLevel(depth);
				if (node->left->next->next->next->type != BLOCK)
					GC.gc_collect();
				
				// Jump on increment
				element->target = node->left->next->next;
			} else if (element->data & FLAG_4) {
				// Increment executed
				element->data  &= ~FLAG_4;
				element->data  |=  FLAG_1;
				// Jump on condition
				element->target = node->left->next;
			}
			
			break;
		}
		
		case BREAK: {
			ASTExecuterElement *e             = element;
			int level                         = depth;
			ASTExecuterElement *enclosing_try = NULL;
			
			while (1) {
				if (!e || (e->attached_node && e->attached_node->type == CALL) || ((e->data & DATA10_MASK) >> DATA10_OFFSET) == NATIVE_CALL) {
					// Error: no loop block
					// XXX: Throw error on closest Try/Catch block
					
					// 1. Collect StackTrace as array of linenumber + partial source
					// 2. Build an Error object instance
					// 3. Jump to catch block & assign argument to new scope
					// 4 -> Parser :: CATCH node
							// 5 -> Parser :: Fix RAISE
					return;
				}
				if (e->attached_node && e->attached_node->type == TRY) 
					enclosing_try = e;
				else if (e->attached_node && (e->attached_node->type == WHILE || e->attached_node->type == FOR)) {
					int i = 0;
					while (e != aststack->head) {
						leave(aststack->head, aststack->head->attached_node, depth - i++);
						aststack->pop();
					}
					
					// Reached loop
					aststack->head->data |= FLAG_0;
					aststack->head->data |= FLAG_5;
					return;
				}
				
				e = e->next;
				--level;
			}
			
			break;
		};
		
		case CONTINUE: {
				
			ASTExecuterElement *e             = element;
			int level                         = depth;
			ASTExecuterElement *enclosing_try = NULL;
			
			while (1) {
				if (!e || (e->attached_node && e->attached_node->type == CALL) || ((e->data & DATA10_MASK) >> DATA10_OFFSET) == NATIVE_CALL) {
					// Error: no loop block
					// XXX: Throw error on closest Try/Catch block
					
					// 1. Collect StackTrace as array of linenumber + partial source
					// 2. Build an Error object instance
					// 3. Jump to catch block & assign argument to new scope
					// 4 -> Parser :: CATCH node
							// 5 -> Parser :: Fix RAISE
					return;
				}
				if (e->attached_node && e->attached_node->type == TRY) 
					enclosing_try = e;
				else if ((e->attached_node && e->attached_node->type == WHILE) || (e->attached_node && e->attached_node->type == FOR)) {
					int i = 0;
					while (e != aststack->head) {
						leave(aststack->head, aststack->head->attached_node, depth - i++);
						aststack->pop();
					}
					
					// Reached loop
					aststack->head->data |= FLAG_0;
					return;
				}
				
				e = e->next;
				--level;
			}
			
			break;
		};
		
		case EXPRESSION: {
			// FLAG_1: executed
			
			if (element->target == NULL)
				if (element->data & FLAG_1)
					return;
				else {
					element->scope  = element->next ? element->next->scope : NULL;
					element->data  |= FLAG_1;
					element->target = node->left;
				}
			else {
				element->target = NULL;
				astobjstack->pop(depth);
			}
			break;
		}
		
		case EMPTY:
			break;
		
		case TRY: {
			// FLAG_1: Executed
			// FLAG_2: Catch node
			// FLAG_3: Attached new Scope for catch node
			
			// Catch thrown on execution
			if (element->data & FLAG_0) {
				element->data  &= ~FLAG_0;
				element->data  &= ~FLAG_2;
				element->target = node->right;
				return;
			}
			
			// Statement was executed correctly
			if (element->data & FLAG_1) {
				element->target = NULL;
				return;
			}
			
			element->scope  = element->next ? element->next->scope : NULL;
			element->target = node->left;
			element->data  |= FLAG_1;
			
			break;
		};
		
		case RETURN: {
			// FLAG_1: value calculated
			
			if (element->target == NULL) 
				if (element->data & FLAG_1)
					return;
				else {
					element->scope  = element->next ? element->next->scope : NULL;
					element->data  |= FLAG_1;
					if (!node->left) {
						ASTExecuterElement *e             = element;
						int level                         = depth;
						ASTExecuterElement *enclosing_try = NULL;
						
						while (1) {
							if (!e) {
								// Error: no function block
								// XXX: Throw error on closest Try/Catch block
								
								// 1. Collect StackTrace as array of linenumber + partial source
								// 2. Build an Error object instance
								// 3. Jump to catch block & assign argument to new scope
								// 4 -> Parser :: CATCH node
											// 5 -> Parser :: Fix RAISE
											// XXX: INCLUDED_SCRIPT type
								return;
							}
							if (e->attached_node && e->attached_node->type == TRY) 
								enclosing_try = e;
							else if ((e->attached_node && e->attached_node->type == CALL) || ((e->data & DATA10_MASK) >> DATA10_OFFSET) == NATIVE_CALL) {
								// Reached function body, return value via stack
								int i = 0;
								while (e != aststack->head) {
									leave(aststack->head, aststack->head->attached_node, depth - i++);
									aststack->pop();
								}
								
								// Override jump loop
								aststack->head->data |= FLAG_0;
								if (!astobjstack->push(new Undefined, level)){
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
								return;
							}
							
							e = e->next;
							--level;
						}
					} else
						element->target = node->left;
				}
			else {
				element->target = NULL;
				
				ASTExecuterElement *e             = element;
				int level                         = depth;
				ASTExecuterElement *enclosing_try = NULL;
				
				while (1) {
					if (!e) {
						// Error: no function block
						// XXX: Throw error on closest Try/Catch block
						
						// 1. Collect StackTrace as array of linenumber + partial source
						// 2. Build an Error object instance
						// 3. Jump to catch block & assign argument to new scope
						// 4 -> Parser :: CATCH node
								// 5 -> Parser :: Fix RAISE
						return;
					}
					if (e->attached_node && e->attached_node->type == TRY) 
						enclosing_try = e;
					else if ((e->attached_node && e->attached_node->type == CALL) || ((e->data & DATA10_MASK) >> DATA10_OFFSET) == NATIVE_CALL) {
						// Reached function body, return value via stack
						VirtualObject *ret = astobjstack->peek(depth);
						ret = defined(ret) ? ret : ret == NULL ? new Undefined : ret;
						if (!astobjstack->push(ret, level)){
							stackoverflow_error(this, OBJECT_STACK);
							return;
						}
						int i = 0;
						while (e != aststack->head) {
							leave(aststack->head, aststack->head->attached_node, depth - i++);
							aststack->pop();
						}
						
						// Override jump loop
						aststack->head->data |= FLAG_0;
						return;
					}
					
					e = e->next;
					--level;
				}
			}
			
			break;
		};
		
		case RAISE: {
			// FLAG_1: value calculated
			
			if (element->target == NULL) 
				if (element->data & FLAG_1)
					return;
				else {
					element->scope  = element->next ? element->next->scope : NULL;
					element->data  |= FLAG_1;
					if (!node->left) {
						ASTExecuterElement *e             = element;
						int level                         = depth;
						ASTExecuterElement *enclosing_try = NULL;
						
						raiseError(new Undefined);
					} else
						element->target = node->left;
				}
			else {
				element->target = NULL;
				
				raiseError(astobjstack->peek(depth));
			}
			
			break;
		};
		
		// Objects
		
		case INTEGER: {
			if (!astobjstack->push(new Integer(*((int*) node->objectlist->object)), depth - 1)) 
				stackoverflow_error(this, OBJECT_STACK);
			break;
		}
		
		case BOOLEAN: {
			if (!astobjstack->push(new Boolean(*((bool*) node->objectlist->object)), depth - 1)) 
				stackoverflow_error(this, OBJECT_STACK);
			break;
		}
		
		case TNULL: {
			if (!astobjstack->push(new Null, depth - 1)) 
				stackoverflow_error(this, OBJECT_STACK);
			break;
		}
		
		case UNDEFINED: {
			if (!astobjstack->push(new Undefined, depth - 1)) 
				stackoverflow_error(this, OBJECT_STACK);
			break;
		}
		
		case NAME: {
			if (element->next && element->next->scope) {
				VirtualObject *o = element->next->scope->get(element->scope, (string*) node->objectlist->object);
				if (o) {
					if (!astobjstack->push(o, depth - 1)) 
						stackoverflow_error(this, OBJECT_STACK);
				} else
					if (!astobjstack->push(new Undefined, depth - 1)) 
						stackoverflow_error(this, OBJECT_STACK);
			} else
				if (!astobjstack->push(new Undefined, depth - 1)) 
					stackoverflow_error(this, OBJECT_STACK);
			break;
		}
		
		case SELF: {
			// Refers to current Scope
			if (element->next && element->next->scope) {
				if (!astobjstack->push(element->next->scope, depth - 1)) 
					stackoverflow_error(this, OBJECT_STACK);
			} else
				if (!astobjstack->push(new Undefined, depth - 1)) 
					stackoverflow_error(this, OBJECT_STACK);
			break;
		};
		
		case THIS: {
			// Refers to the closest ProxyScope object / current Scope
			Scope *scope = element->next ? element->next->scope : NULL;
			
			while (scope) {
				if (scope->type == PROXY_SCOPE) 
					break;
				scope = scope->parent;
			}
			
			if (scope) {
				if (!astobjstack->push(((ProxyScope*) scope)->object, depth - 1)) 
					stackoverflow_error(this, OBJECT_STACK);
			} else if (element->next && element->next->scope) {
				if (!astobjstack->push(element->next->scope, depth - 1)) 
					stackoverflow_error(this, OBJECT_STACK);
			} else
				if (!astobjstack->push(new Undefined, depth - 1)) 
					stackoverflow_error(this, OBJECT_STACK);
			break;
		};
		
		case STRING: {
			if (!astobjstack->push(new String(new string((string*) node->objectlist->object)), depth - 1)) 
				stackoverflow_error(this, OBJECT_STACK);
			break;
		}
	
		case DOUBLE: {
			if (!astobjstack->push(new Double(*((double*) node->objectlist->object)), depth - 1)) 
				stackoverflow_error(this, OBJECT_STACK);
			break;
		}
	
		case OBJECT: {
			// FLAG_1: executed
			// FLAG_2: values calculated
			
			if (element->target == NULL)
				if (element->data & FLAG_1)
					return;
				else {
					element->data  |= FLAG_1;
					element->scope  = element->next ? element->next->scope : NULL;
					element->target = node->left;
					
					if (!element->target)
						if (!astobjstack->push(new Object, depth - 1)) {
							stackoverflow_error(this, OBJECT_STACK);
							return;
						}
				}
			else {
				if (!element->target->next) {
					element->target     = NULL;
					Object *o           = new Object;
					ASTObjectList *l    = node->objectlist;
					ASTObjectElement *e = astobjstack->head;
					
					while (l) {
						if (!e || e->level != depth) {
							o->put(element->scope, (string*) l->object, new Undefined);
							return;
						} else {
							o->put(element->scope, (string*) l->object, e->object);
							l = l->next;
							e = e->next;
						}
					}
					
					if (!astobjstack->push(o, depth - 1)) {
						stackoverflow_error(this, OBJECT_STACK);
						return;
					}
					return;
				}
				element->target = element->target->next;
			}
			break;
		}
	
		case ARRAY: {
			// FLAG_1: executed
			// FLAG_2: values calculated
			
			if (element->target == NULL)
				if (element->data & FLAG_1)
					return;
				else {
					element->data  |= FLAG_1;
					element->scope  = element->next ? element->next->scope : NULL;
					element->target = node->left;
					
					if (!element->target)
						astobjstack->push(new Array, depth - 1);
				}
			else {
				if (!element->target->next) {
					element->target     = NULL;
					Array *a            = new Array(NULL);
					ASTObjectElement *e = astobjstack->head;
					int n = *(int*) node->objectlist->object;
					
					a->array = new VectorArray<VirtualObject>(n);
					a->array->length = n;
					
					for (int i = 0; i < n; ++i) {
						if (!e || e->level != depth) 
							a->array->vector[i] = NULL;
						else {
							a->array->vector[i] = e->object;
							e = e->next;
						}
					}
					
					if (!astobjstack->push(a, depth - 1)) 
						stackoverflow_error(this, OBJECT_STACK);
					return;
				}
				element->target = element->target->next;
			}
			break;
		}
	
		case FUNCTION: {
			if (!astobjstack->push(new CodeFunction(element->next ? element->next->scope : NULL, node), depth - 1))
				stackoverflow_error(this, OBJECT_STACK);
			break;
		}
	
	
	
		// 
		// 
		// 
		// 
		// 
		// 
		// 
		//            Fucntion call expects reference object of this function
		//                   Making object as reference Scope using Proxy Scope
		// 
		// 
		// 
		// 
		// 
	
	
		// Operators
		case CALL: {
			// Before calling function, try to get function reference object.
			
			// Called function scope tructure:
			// [Definition Scope]
			//  \
			//   [Object proxy Scope]
			//    \
			//     [Function Scope]
			//      \
			//       [Function]
			
			//  OBJECT STACK:
			//  --------------
			// |  reference      <-- *r
			//  --------------
			// |  function       <-- *f
			//  --------------
			// |    arg0         <-- *e
			//  --------------
			// |    arg1
			//  --------------
			// |    ....
			//  --------------
			// |    argK         <-- available arguments in stack [n]
			//  --------------
			// |  undefined
			//  --------------
			// |  undefined
			//  --------------
			// |  undefined      <-- addictional arguments [expected_argc]
			//  --------------
			
			// reference:
			// NULL           - [enclosing Scope]
			// object:member  - reference[member]
			// object:field   - reference.field
			
			// FLAG_3: reference & function calculated
			
			if (element->data & FLAG_3) {
				// FLAG_1: arguments entered on calculation
				// FLAG_4: executed
				// FLAG_5: code function executed
				
				if (element->data & FLAG_5) {
					element->data &= ~FLAG_0;
					element->data &= ~FLAG_3;
					element->data &= ~FLAG_5;
					// Push value up
					astobjstack->pullUp(depth);
					
					element->target = NULL;
					
					return;
					
				} else if (element->target == NULL)
					if (element->data & FLAG_4)
						return;
					else {
						element->data  &= ~FLAG_0;
						element->data  &= ~FLAG_1;
						element->scope  = element->next ? element->next->scope : NULL;
						element->data  |= FLAG_4;
						element->target = node->left->next;
					}
				
				if (element->data & FLAG_1)
					element->target = element->target->next;
				else 
					element->data |= FLAG_1;
				
				if (element->target) {
					++element->data;
					return;
				}
				
				int n = 0;
				int expected_argc   = element->data & USER16_MASK;
				ASTObjectElement *e = astobjstack->head;
				
				// astobjstack->print();
				
				//  OBJECT STACK:
				//  --------------
				// |  reference      <-- *r
				//  --------------
				// |  function       <-- *f
				//  --------------
				// |    arg0         <-- *e
				//  --------------
				// |    arg1
				//  --------------
				// |    ....
				//  --------------
				// |    argK         <-- available arguments in stack [n]
				//  --------------
				// |  undefined
				//  --------------
				// |  undefined
				//  --------------
				// |  undefined      <-- addictional arguments [expected_argc]
				//  --------------
				
				while (e->next && e->next->level == depth) {
					++n;
					e = e->next;
				}
				
				for (int i = n; i < expected_argc; ++i)
					if (!astobjstack->push(new Undefined, depth)) {
						stackoverflow_error(this, OBJECT_STACK);
						return;
					}
				
				ptr_wrapper wrapper(expected_argc ? (VirtualObject**) malloc(expected_argc * sizeof(VirtualObject*)) : NULL, PTR_ALLOC);
				VirtualObject **args = (VirtualObject**) wrapper.ptr;
				
				VirtualObject *r     = e->object;
				e = e->prev;
				VirtualObject *f     = e->object;
				
				for (int i = 0; i < expected_argc; ++i) {
					args[i] = (e = e->prev)->object;
					if (!args[i])
						args[i] = new Undefined;
				}
				
				if (f) {
					if (f->type == UNDEFINED || f->type == TNULL) {
						if (!astobjstack->push(new Undefined, depth - 1)) {
							stackoverflow_error(this, OBJECT_STACK);
							return;
						}
						
					} else if (f->type == CODE_FUNCTION) {
						if (r)
							element->scope = new Scope(new ProxyScope(((CodeFunction*) f)->scope, r));
						else
							element->scope = new Scope(((CodeFunction*) f)->scope);
						
						GC.gc_attach_root(element->scope);
						
						element->data  |= FLAG_5;
						
						element->target = ((CodeFunction*) f)->node->left;
						
						// Append all arguments into __arguments Array
						Array *__arguments = new Array();
						for (int i = 0; i < expected_argc; ++i)
							__arguments->array->push(args[i]);
						string key = "__arguments";
						element->scope->define(&key, __arguments);
						
						f->call(element->scope, expected_argc, args);
					} else {
						if (r)
							element->scope = new Scope(new ProxyScope(element->next ? element->next->scope : NULL, r));
						else {
							if (!element->next) {
								raiseError("Execution break. Scope == NULL.");
								element->data  &= FLAG6_MASK;
								element->target = NULL;
							}
							
							element->scope = new Scope(element->next ? element->next->scope : NULL);
						}
						
						GC.gc_attach_root(element->scope);
						
						element->data  |= FLAG_5;
						
						element->target = NULL;
						
						// Append all arguments into __arguments Array
						Array *__arguments = new Array();
						for (int i = 0; i < expected_argc; ++i)
							__arguments->array->push(args[i]);
						string key = "__arguments";
						element->scope->define(&key, __arguments);
						
						f = f->call(element->scope, expected_argc, args);
						if (!astobjstack->push(f, depth - 1)) {
							stackoverflow_error(this, OBJECT_STACK);
							return;
						}
					}
				} else {
					astobjstack->popForLevel(depth);
					if (!astobjstack->push(new Undefined, depth - 1)) {
						stackoverflow_error(this, OBJECT_STACK);
						return;
					}
				}
				
				free(args);
				wrapper.type = PTR_UNDEF;
				
			} else {
				// Save called as anonymous
				if (!aststacktrace->push(depth, node->lineno)) {
					stackoverflow_error(this, STACKTRACE_STACK);
					return;
				}
				
				switch (node->left->type) {
					case NAME: {
						// Save called name
						if (!aststacktrace->push(depth, node->left->lineno, *(string*) node->left->objectlist->object)) {
							stackoverflow_error(this, STACKTRACE_STACK);
							return;
						}
				
						if (!astobjstack->push(NULL, depth)) {
							stackoverflow_error(this, OBJECT_STACK);
							return;
						}
						
						if (element->next && element->next->scope) {
							VirtualObject *o = element->next->scope->get(element->scope, (string*) node->left->objectlist->object);
							if (o) {
								if (!astobjstack->push(o, depth)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
							} else 
								if (!astobjstack->push(new Undefined, depth)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
						} else
							if (!astobjstack->push(new Undefined, depth)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
						}
						
						element->target = NULL;
						element->data  |= FLAG_0;
						element->data  |= FLAG_3;
						
						break;
					};
					
					case MEMBER: {
						// FLAG_1: left calculated
						// FLAG_2: right calculated
						
						if (element->target == NULL)
							if (element->data & FLAG_1)
								return;
							else {
								element->scope  = element->next ? element->next->scope : NULL;
								element->data  |= FLAG_1;
								element->target = node->left->left;
							}
						else if (element->data & FLAG_2) {
							// left & Right calculated
							element->data  &= ~FLAG_2;
							element->target = NULL;
							
							string key               = objectStringValue(astobjstack->peek(depth));
							VirtualObject *reference = astobjstack->peek(1, depth);
							
							// Save called name
							if (!aststacktrace->push(depth, node->left->right->lineno, key)) {
								stackoverflow_error(this, STACKTRACE_STACK);
								return;
							}
							//  foo
							//       [
							//          "bar"   <-- lineno of this
							//       ]
							
							if (!defined(reference)) {
								if (!astobjstack->push(NULL, depth)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
								if (!astobjstack->push(new Undefined, depth)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
								element->target = NULL;
								element->data  |= FLAG_0;
								element->data  |= FLAG_3;
								return;
							}
							
							VirtualObject *o         = reference->get(element->scope, &key);
							
							astobjstack->popForLevel(depth);
							if (!astobjstack->push(reference, depth)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							
							if (defined(o))
								if (!astobjstack->push(o, depth)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
							else
								if (!astobjstack->push(new Undefined, depth)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
							
							element->target = NULL;
							element->data  |= FLAG_0;
							element->data  |= FLAG_3;
							
						} else {
							// Left calculated
							element->target = element->target->next;
							element->data  |= FLAG_2;
						}
						break;
					};
					
					case FIELD: {
						// FLAG_1: executed
						
						if (element->target == NULL)
							if (element->data & FLAG_1)
								return;
							else {
								element->scope  = element->next ? element->next->scope : NULL;
								element->data  |= FLAG_1;
								element->target = node->left->left;
							}
						else {
							// Left calculated
							element->target = NULL;
								
							VirtualObject *reference = astobjstack->peek(depth);
							
							if (!defined(reference)) {
								if (!astobjstack->push(NULL, depth)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
								if (!astobjstack->push(new Undefined, depth)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
								element->target = NULL;
								element->data  |= FLAG_0;
								element->data  |= FLAG_3;
								return;
							}
							
							VirtualObject *o = reference->get(element->scope, (string*) node->left->objectlist->object);
							
							// Save called name
							if (!aststacktrace->push(depth, node->left->lineno, *(string*) node->left->objectlist->object)) {
								stackoverflow_error(this, STACKTRACE_STACK);
								return;
							}
							
							astobjstack->popForLevel(depth);
							if (!astobjstack->push(reference, depth)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							
							if (defined(o)) {
								if (!astobjstack->push(o, depth)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
							} else
								if (!astobjstack->push(new Undefined, depth)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
							
							element->target = NULL;
							element->data  |= FLAG_0;
							element->data  |= FLAG_3;
						}
						break;
					};

					default: {
						if (element->target == NULL)
							if (element->data & FLAG_1)
								return;
							else {
								element->scope  = element->next ? element->next->scope : NULL;
								element->data  |= FLAG_1;
								element->target = node->left;
							}
						else {
							VirtualObject *o = astobjstack->peek(depth);
							
							astobjstack->popForLevel(depth);
							if (!astobjstack->push(NULL, depth)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							
							if (defined(o)) {
								if (!astobjstack->push(o, depth)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
							} else
								if (!astobjstack->push(new Undefined, depth)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
							
							element->target = NULL;
							element->data  |= FLAG_0;
							element->data  |= FLAG_3;
						}
						break;
					};
				};
			}
			
			break;
		}
		
		case NATIVE_CALL: {
			// FLAG_1: executed
			// FLAG_2: code function executed
			
			if (element->data & FLAG_2) {
				element->data &= ~FLAG_0;
				element->data &= ~FLAG_2;
				// Push value up
				astobjstack->pullUp(depth);
				
				element->target = NULL;
			} else if (element->target == NULL)
				if (element->data & FLAG_1)
					return;
				else {
					// Save function as anonymous if no name is set
					if (!aststacktrace->head || aststacktrace->head->level != depth)
						if (!aststacktrace->push(depth, aststacktrace->head ? aststacktrace->head->tracelineno : -1)) {
							stackoverflow_error(this, STACKTRACE_STACK);
							return;
						}
					
					// Reset FLAG_0
					aststack->head->data &= ~FLAG_0;
					element->data        |= FLAG_1;
					
					
					// astobjstack->print();
					
					//  OBJECT STACK:
					//  --------------
					// |  reference      <-- *r
					//  --------------
					// |  function       <-- *f
					//  --------------
					// |    arg0         <-- *e
					//  --------------
					// |    arg1
					//  --------------
					// |    ....
					//  --------------
					// |    argN         <-- available arguments in stack [n]
					//  --------------
					
					int n = 0;
					ASTObjectElement *e = astobjstack->head;
					while (e->next && e->next->level == depth) {
						++n;
						e = e->next;
					}
					
					ptr_wrapper wrapper(n ? (VirtualObject**) malloc(n * sizeof(VirtualObject*)) : NULL, PTR_ALLOC);
					VirtualObject **args = (VirtualObject**) wrapper.ptr;
				
					VirtualObject *r     = e->object;
					e = e->prev;
					VirtualObject *f     = e->object;
					--n;
					
					for (int i = 0; i < n; ++i) {
						args[i] = (e = e->prev)->object;
						if (!args[i])
							args[i] = new Undefined;
					}
						
					if (f) {
						if (f->type == UNDEFINED || f->type == TNULL) {
							astobjstack->popForLevel(depth);
							if (!astobjstack->push(new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
						} else if (f->type == CODE_FUNCTION) {
							if (r)
								element->scope = new Scope(new ProxyScope(((CodeFunction*) f)->scope, r));
							else
								element->scope = new Scope(((CodeFunction*) f)->scope);
							
							GC.gc_attach_root(element->scope);
							
							element->data  |= FLAG_2;
							
							element->target = ((CodeFunction*) f)->node->left;
						
							// Append all arguments into __arguments Array
							Array *__arguments = new Array();
							for (int i = 0; i < n; ++i)
								__arguments->array->push(args[i]);
							string key = "__arguments";
							element->scope->define(&key, __arguments);
							
							f->call(element->scope, n, args);
						} else {
							if (r)
								element->scope = new Scope(new ProxyScope(element->scope, r));
							else {
								if (!element->next) {
									raiseError("Execution break. Scope == NULL.");
									element->data  &= FLAG6_MASK;
									element->target = NULL;
								}
								
								element->scope = new Scope(element->scope);
							}
							
							GC.gc_attach_root(element->scope);
							
							element->data  |= FLAG_2;
							
							element->target = NULL;
						
							// Append all arguments into __arguments Array
							Array *__arguments = new Array();
							for (int i = 0; i < n; ++i)
								__arguments->array->push(args[i]);
							string key = "__arguments";
							element->scope->define(&key, __arguments);
							
							f = f->call(element->scope, n, args);
							if (!astobjstack->push(f, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
						}
					} else {
						astobjstack->popForLevel(depth);
						if (!astobjstack->push(new Undefined, depth - 1)) {
							stackoverflow_error(this, OBJECT_STACK);
							return;
						}
					}
					
					free(args);
					wrapper.type = PTR_UNDEF;
				}
			else // Unreachable
				element->target = NULL;
			
			break;
		}

		case CONDITION: {
			// FLAG_1: condition calculated
			// FLAG_2: block executed
			
			if (element->target == NULL)
				if (element->data & FLAG_1)
					return;
				else {
					element->scope  = element->next ? element->next->scope : NULL;
					element->data  |= FLAG_1;
					element->target = node->left;
				}
			else if (element->data & FLAG_2) {
				// Push value up
				astobjstack->pullUp(depth);
				
				element->target = NULL;
			} else {
				element->data  |= FLAG_2;
				
				if (objectIntValue(astobjstack->peek(depth)))
					element->target = node->left->next;
				else
					element->target = node->right;
			}
			break;
		}

		//
		//
		//
		// OPERATOR_OP passes tho arguments to the function: 1 - operated object, 2 - argument
		//
		//
		//

		case EQ: {
			binaryOperatorCall(element, node, depth, "__operator==");
			break;
		}

		case NEQ: {
			binaryOperatorCall(element, node, depth, "__operator!=");
			break;
		}
		
		case OR: {
			binaryLogicalOperatorCall(element, node, depth, "__operator||");
			break;
		}

		case AND: {
			binaryLogicalOperatorCall(element, node, depth, "__operator&&");
			break;
		}

		case BITOR: {
			binaryOperatorCall(element, node, depth, "__operator|");
			break;
		}

		case BITAND: {
			binaryOperatorCall(element, node, depth, "__operator&");
			break;
		}

		case BITXOR: {
			binaryOperatorCall(element, node, depth, "__operator^");
			break;
		}

		case GT: {
			binaryOperatorCall(element, node, depth, "__operator>");
			break;
		}

		case GE: {
			binaryOperatorCall(element, node, depth, "__operator>=");
			break;
		}

		case LT: {
			binaryOperatorCall(element, node, depth, "__operator<");
			break;
		}

		case LE: {
			binaryOperatorCall(element, node, depth, "__operator<=");
			break;
		}

		case BITRSH: {
			binaryOperatorCall(element, node, depth, "__operator>>");
			break;
		}

		case BITLSH: {
			binaryOperatorCall(element, node, depth, "__operator<<");
			break;
		}

		case BITURSH: {
			binaryOperatorCall(element, node, depth, "__operator>>>");
			break;
		}

		case MDIV: {
			binaryOperatorCall(element, node, depth, "__operator\\\\");
			break;
		}

		case MOD: {
			binaryOperatorCall(element, node, depth, "__operator%");
			break;
		}

		case HASH: {
			binaryOperatorCall(element, node, depth, "__operator#");
			break;
		}

		case PATH: {
			binaryOperatorCall(element, node, depth, "__operator\\");
			break;
		}

		case PUSH: {
			binaryOperatorCall(element, node, depth, "__operator->");
			break;
		}

		case LAMBDA: {
			binaryOperatorCall(element, node, depth, "__operator=>");
			break;
		}

		case PLUS: {
			binaryOperatorCall(element, node, depth, "__operator+");
			break;
		}

		case MINUS: {
			binaryOperatorCall(element, node, depth, "__operator-");
			break;
		}

		case MUL: {
			binaryOperatorCall(element, node, depth, "__operator*");
			break;
		}

		case DIV: {
			binaryOperatorCall(element, node, depth, "__operator/");
			break;
		}
		
		case ASSIGN: {
			switch (node->left->type) {
				case NAME: {
					// FLAG_1: value calculated
					
					if (element->target == NULL)
						if (element->data & FLAG_1)
							return;
						else {
							element->scope  = element->next ? element->next->scope : NULL;
							element->data  |= FLAG_1;
							element->target = node->right;
						}
					else {
						element->target = NULL;
						VirtualObject *o = astobjstack->peek(depth);
						
						if (o == NULL) {
							o = new Undefined;
							element->next->scope->putPriority(element->scope, (string*) (node->left->objectlist->object), o, 0);
							if (!astobjstack->push(o, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
						} else {
							element->next->scope->putPriority(element->scope, (string*) (node->left->objectlist->object), o, 0);
							astobjstack->pullUp(depth);
						}
					}
					break;
				}
				
				case MEMBER: {
					// FLAG_1: left->left  calculated
					// FLAG_2: left->right calculated
					// FLAG_3: right       calculated
					
					if (element->target == NULL)
						if (element->data & FLAG_1)
							return;
						else {
							element->scope  = element->next ? element->next->scope : NULL;
							element->data  |= FLAG_1;
							element->target = node->left->left;
						}
					else if (element->data & FLAG_2) {
						// left & Right calculated
						element->data  &= ~FLAG_2;
						element->data  |=  FLAG_3;
						element->target = node->right;
						
					} else if (element->data & FLAG_3) {
						// left & Right & value calculated
						element->data  &= ~FLAG_3;
						element->target = NULL;
						
						string key               = objectStringValue(astobjstack->peek(1, depth));
						VirtualObject *reference = astobjstack->peek(2, depth);
						VirtualObject *value     = astobjstack->peek(depth);
						
						if (defined(reference)) {
							reference->put(element->scope, &key, value);
							astobjstack->pullUp(depth);
						} else
							if (!astobjstack->push(new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
						
					} else {
						// Left calculated
						element->target = element->target->next;
						element->data  |= FLAG_2;
					}
					break;
				}
				
				case FIELD: {
					// FLAG_1: reference calculated
					// FLAG_2: value calculated						
					
					if (element->target == NULL)
						if (element->data & FLAG_1)
							return;
						else {
							element->scope  = element->next ? element->next->scope : NULL;
							element->data  |= FLAG_1;
							element->target = node->left->left;
						}
					else if (element->data & FLAG_2) {
						element->target = NULL;
						
						VirtualObject *reference = astobjstack->peek(1, depth);
						VirtualObject *value     = astobjstack->peek(depth);
						
						if (defined(reference)) {
							reference->put(element->scope, (string*) node->left->objectlist->object, value);
							astobjstack->pullUp(depth);
						} else
							if (!astobjstack->push(new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
					} else {
						// Left calculated
						element->target = node->right;
						element->data  |= FLAG_2;
					}
					break;
				}
			}
			
			break;
		}
		
		case ASSIGN_ADD:
		case ASSIGN_SUB: 
		case ASSIGN_MUL: 
		case ASSIGN_DIV: 
		case ASSIGN_BITRSH: 
		case ASSIGN_BITURSH: 
		case ASSIGN_BITLSH: 
		case ASSIGN_BITAND: 
		case ASSIGN_BITOR: 
		case ASSIGN_BITXOR: 
		case ASSIGN_MDIV: 
		case ASSIGN_MOD: {
			switch (node->left->type) {
				case NAME: {
					// FLAG_1: excuted
					// FLAG_2: value calculated
					// FLAG_3: expected call result
					
					if (element->data & FLAG_3) {
						element->data  &= ~FLAG_0;
						element->data  &= ~FLAG_3;
						element->target = NULL;
						VirtualObject *result = astobjstack->peek(depth);
						
						if (!defined(result)) {
							if (!astobjstack->push(result = new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							element->scope->putPriority(element->scope, (string*) node->left->objectlist->object, result, 0);
						} else {
							if (!astobjstack->push(result, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							element->scope->putPriority(element->scope, (string*) node->left->objectlist->object, result, 0);
						}
						
					} else if (element->data & FLAG_2) {
						// Value calculated
						element->data  &= ~FLAG_2;
						element->target = NULL;
						
						VirtualObject *o = element->scope->get(element->scope, (string*) node->left->objectlist->object);
						VirtualObject *v = astobjstack->peek(depth);
						
						if (o == NULL) {
							if (!astobjstack->push(o = new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							element->scope->putPriority(element->scope, (string*) node->left->objectlist->object, o, 0);
							return;
						} else {
							// call operator OP=
							string key = operator_asign_string(node->type);
							nativeCall(element->scope, o->get(element->scope, &key), 2, o, v);
							// Expect result
							element->data  |= FLAG_0;
							element->data  |= FLAG_3;
						}
					} else if (element->data & FLAG_1)
						return;
					else {
						element->scope  = element->next ? element->next->scope : NULL;
						element->data  |= FLAG_1;
						if (!element->scope) {
							if (!astobjstack->push(new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							return;
						}
						element->data  |= FLAG_2;
						element->target = node->right;
					} 
					break;
				}
				
				case MEMBER: {
					// FLAG_1: reference  calculated
					// FLAG_2: key        calculated
					// FLAG_3: value      calculated
					// FLAG_4: result     calculated
					
					if (element->data & FLAG_4) {
						element->data  &= ~FLAG_0;
						element->data  &= ~FLAG_4;
						element->target = NULL;
						
						VirtualObject *result    = astobjstack->peek(depth);
						string key               = objectStringValue(astobjstack->peek(2, depth));
						VirtualObject *reference = astobjstack->peek(3, depth);
						
						if (defined(reference)) {
							if (!defined(result)) {
								if (!astobjstack->push(result = new Undefined, depth - 1)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
								reference->put(element->scope, &key, result);
							} else {
								if (!astobjstack->push(result, depth - 1)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
								reference->put(element->scope, &key, result);
							}
						} else 
							if (!astobjstack->push(result ? result : new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
						
					} else if (element->target == NULL)
						if (element->data & FLAG_1)
							return;
						else {
							element->scope  = element->next ? element->next->scope : NULL;
							element->data  |= FLAG_1;
							if (!element->scope) {
								if (!astobjstack->push(new Undefined, depth - 1)) 
									stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							element->target = node->left->left;
						}
					else if (element->data & FLAG_3) {
						// left & Right & Value calculated
						element->data  &= ~FLAG_3;
						
						VirtualObject *value     = astobjstack->peek(depth);
						VirtualObject *okey      = astobjstack->peek(1, depth);
						string         key       = objectStringValue(okey);
						VirtualObject *reference = astobjstack->peek(2, depth);
						
						if (defined(reference)) {
							VirtualObject *o = reference->get(element->scope, &key);
							
							if (o == NULL) {
								if (!astobjstack->push(o = new Undefined, depth - 1)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
								reference->put(element->scope, &key, o);
								element->target = NULL;
								return;
							}
							
							element->data   |=  FLAG_0;
							element->data   |=  FLAG_4;
							string key       = operator_asign_string(node->type);
							nativeCall(element->scope, o->get(element->scope, &key), 2, o, value);
						} else {
							element->target = NULL;
							if (!astobjstack->push(new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
						}
						
					} else if (element->data & FLAG_2) {
						// Left & Right calculated
						element->data  &= ~FLAG_2;
						element->data  |=  FLAG_3;
						element->target = node->right;					
					} else {
						// Left calculated
						element->target = node->left->right;
						element->data  |= FLAG_2;
					}
					break;
				}
				
				case FIELD: {
					// FLAG_1: reference calculated
					// FLAG_2: value     calculated
					// FLAG_3: result    calculated			
					
					if (element->data & FLAG_3) {
						element->data   &= ~FLAG_0;
						element->data   &= ~FLAG_3;
						element->target  = NULL;
						
						VirtualObject *reference = astobjstack->peek(2, depth);
						VirtualObject *result    = astobjstack->peek(depth);
						
						if (defined(result)) {
							if (!astobjstack->push(result, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							reference->put(element->scope, (string*) node->left->objectlist->object, result);
						} else {
							if (!astobjstack->push(result = result ? result : new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							reference->put(element->scope, (string*) node->left->objectlist->object, result);
						}
						
					} else if (element->target == NULL)
						if (element->data & FLAG_1)
							return;
						else {
							element->scope  = element->next ? element->next->scope : NULL;
							element->data  |= FLAG_1;
							element->target = node->left->left;
						}
					else if (element->data & FLAG_2) {
						// Left & value calculated
						element->data   &= ~FLAG_2;
						element->target  = NULL;
						
						element->data           |= FLAG_3;
						VirtualObject *reference = astobjstack->peek(1, depth);
						
						if (defined(reference)) {
							element->data   |=  FLAG_0;
							element->data   |=  FLAG_3;
							VirtualObject *o = reference->get(element->scope, (string*) node->left->objectlist->object);
							VirtualObject *v = astobjstack->peek(depth);
							string key = operator_asign_string(node->type);
							nativeCall(element->scope, o->get(element->scope, &key), 2, o, v);
						} else {
							element->target = NULL;
							if (!astobjstack->push(new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
						}
						
					} else {
						// Left calculated
						element->data  |= FLAG_2;
						element->target = node->right;
					}
					break;
				}
			}
			break;
		}

		//
		//
		//
		//

		case NEG: {
			unaryOperatorCall(element, node, depth, "__operator-x");
			break;
		}

		case POS: {
			unaryOperatorCall(element, node, depth, "__operator+x");
			break;
		}

		case NOT: {
			unaryOperatorCall(element, node, depth, "__operator!x");
			break;
		}

		case BITNOT: {
			unaryOperatorCall(element, node, depth, "__operator~x");
			break;
		}


		case POS_INC:
		case POS_DEC: {
			switch (node->left->type) {
				case NAME: {
					// FLAG_1: excuted
					// FLAG_2: expected call result
					
					if (element->data & FLAG_2) {
						element->data  &= ~FLAG_0;
						element->data  &= ~FLAG_2;
						
						VirtualObject *result = astobjstack->peek(depth);
						
						if (result == NULL) {
							element->scope->putPriority(element->scope, (string*) node->left->objectlist->object, new Undefined, 0);
							return;
						} else {
							element->scope->putPriority(element->scope, (string*) node->left->objectlist->object, result, 0);
							return;
						}
					} else if (element->data & FLAG_1)
						return;
					else {
						element->scope  = element->next ? element->next->scope : NULL;
						element->data  |= FLAG_1;
						if (!element->scope) {
							if (!astobjstack->push(new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							return;
						}
						
						VirtualObject *o = element->scope->get(element->scope, (string*) node->left->objectlist->object);
						
						if (o == NULL) {
							if (!astobjstack->push(o = new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							element->scope->putPriority(element->scope, (string*) node->left->objectlist->object, o, 0);
							return;
						} else {
							// PRE-INC/DEC
							// Save result
							if (!astobjstack->push(o, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							// Push current & call operator X++/X--
							string key = node->type == POS_INC ? "__operator++" : "__operator--";
							nativeCall(element->scope, o->get(element->scope, &key), 1, o);
							// Expect result
							element->data  |= FLAG_0;
							element->data  |= FLAG_2;
						}
					}
					break;
				}
				
				case MEMBER: {
					// FLAG_1: reference  calculated
					// FLAG_2: key        calculated
					// FLAG_3: result     calculated
					
					if (element->target == NULL)
						if (element->data & FLAG_1)
							return;
						else {
							element->scope  = element->next ? element->next->scope : NULL;
							element->data  |= FLAG_1;
							element->target = node->left->left;
						}
					else if (element->data & FLAG_3) {
						element->data  &= ~FLAG_0;
						element->data  &= ~FLAG_3;
						element->target = NULL;
						
						VirtualObject *result    = astobjstack->peek(depth);
						VirtualObject *o         = astobjstack->peek(1, depth);
						string key               = objectStringValue(astobjstack->peek(2, depth));
						VirtualObject *reference = astobjstack->peek(3, depth);
						
						if (defined(reference)) {
							if (!result) 
								reference->put(element->scope, &key, new Undefined);
							else 
								reference->put(element->scope, &key, result);
						}
						
						if (o == NULL)
							if (!astobjstack->push(new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
						else
							if (!astobjstack->push(o, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
					} else if (element->data & FLAG_2) {
						// left & Right calculated
						element->data  &= ~FLAG_2;
						
						string key               = objectStringValue(astobjstack->peek(depth));
						VirtualObject *reference = astobjstack->peek(1, depth);
						
						if (defined(reference)) {
							VirtualObject *o = reference->get(element->scope, &key);
							if (!defined(o)) {
								if (!astobjstack->push(o = new Undefined, depth - 1)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
								reference->put(element->scope, &key, o);
								return;
							}
							element->data   |=  FLAG_0;
							element->data   |=  FLAG_3;
							if (!astobjstack->push(o, depth)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							string key = node->type == POS_INC ? "__operator++" : "__operator--";
							nativeCall(element->scope, o->get(element->scope, &key), 1, o);
						} else {
							element->target = NULL;
							if (!astobjstack->push(new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
						}
						
					} else {
						// Left calculated
						element->target = element->target->next;
						element->data  |= FLAG_2;
					}
					break;
				}
				
				case FIELD: {
					// FLAG_1: reference calculated
					// FLAG_2: result    calculated						
					
					if (element->target == NULL)
						if (element->data & FLAG_1)
							return;
						else {
							element->scope  = element->next ? element->next->scope : NULL;
							element->data  |= FLAG_1;
							element->target = node->left->left;
						}
					else if (element->data & FLAG_2) {
						element->data   &= ~FLAG_0;
						element->data   &= ~FLAG_2;
						element->target  = NULL;
						
						VirtualObject *reference = astobjstack->peek(2, depth);
						VirtualObject *value     = astobjstack->peek(1, depth);
						VirtualObject *result    = astobjstack->peek(depth);
						
						if (defined(result)) {
							reference->put(element->scope, (string*) node->left->objectlist->object, result);
							if (!astobjstack->push(value, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
						} else {
							if (!value) {
								if (!astobjstack->push(new Undefined, depth - 1)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
							} else
								if (!astobjstack->push(value, depth - 1)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
						}
						
					} else {
						// Left calculated
						element->data           |= FLAG_2;
						VirtualObject *reference = astobjstack->peek(depth);
						
						if (defined(reference)) {
							VirtualObject *o = reference->get(element->scope, (string*) node->left->objectlist->object);
							if (!defined(o)) {
								if (!astobjstack->push(o = o ? o : new Undefined, depth - 1)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
								reference->put(element->scope, (string*) node->left->objectlist->object, o);
								return;
							}
							element->data   |=  FLAG_0;
							element->data   |=  FLAG_2;
							if (!astobjstack->push(o, depth)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							string key = node->type == POS_INC ? "__operator++" : "__operator--";
							nativeCall(element->scope, o->get(element->scope, &key), 1, o);
						} else {
							element->target = NULL;
							if (!astobjstack->push(new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
						}
					}
					break;
				}
			}
			break;
		};
		
		case PRE_INC:
		case PRE_DEC: {
			switch (node->left->type) {
				case NAME: {
					// FLAG_1: excuted
					// FLAG_2: expected call result
					
					if (element->data & FLAG_2) {
						element->data  &= ~FLAG_0;
						element->data  &= ~FLAG_2;
						
						VirtualObject *result = astobjstack->peek(depth);
						
						if (result == NULL) {
							if (!astobjstack->push(result = new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							element->scope->putPriority(element->scope, (string*) node->left->objectlist->object, result, 0);
						} else {
							if (!astobjstack->push(result, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							element->scope->putPriority(element->scope, (string*) node->left->objectlist->object, result, 0);
						}
						
					} else if (element->data & FLAG_1)
						return;
					else {
						element->scope  = element->next ? element->next->scope : NULL;
						element->data  |= FLAG_1;
						if (!element->scope) {
							if (!astobjstack->push(new Undefined, depth - 1)) 
								stackoverflow_error(this, OBJECT_STACK);
							return;
						}
						
						VirtualObject *o = element->scope->get(element->scope, (string*) node->left->objectlist->object);
						
						if (!defined(o)) {
							if (!astobjstack->push(o = o ? o : new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							element->scope->putPriority(element->scope, (string*) node->left->objectlist->object, o, 0);
							return;
						} else {
							// POST-INC/DEC
							// call operator ++X/--X--
							string key = node->type == PRE_INC ? "__operator++" : "__operator--";
							nativeCall(element->scope, o->get(element->scope, &key), 1, o);
							// Expect result
							element->data  |= FLAG_0;
							element->data  |= FLAG_2;
						}
					}
					break;
				}
				
				case MEMBER: {
					// FLAG_1: reference  calculated
					// FLAG_2: key        calculated
					// FLAG_3: result     calculated
					
					if (element->target == NULL)
						if (element->data & FLAG_1)
							return;
						else {
							element->scope  = element->next ? element->next->scope : NULL;
							element->data  |= FLAG_1;
							element->target = node->left->left;
						}
					else if (element->data & FLAG_3) {
						element->data  &= ~FLAG_0;
						element->data  &= ~FLAG_3;
						element->target = NULL;
						
						VirtualObject *result    = astobjstack->peek(depth);
						string key               = objectStringValue(astobjstack->peek(1, depth));
						VirtualObject *reference = astobjstack->peek(2, depth);
						
						if (defined(reference)) {
							if (!defined(result)) {
								if (!astobjstack->push(result = new Undefined, depth - 1)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
								reference->put(element->scope, &key, result);
							} else {
								if (!astobjstack->push(result, depth - 1)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
								reference->put(element->scope, &key, result);
							}
						} else 
							if (!astobjstack->push(result ? result : new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
						
					} else if (element->data & FLAG_2) {
						// left & Right calculated
						element->data  &= ~FLAG_2;
						
						VirtualObject *okey      = astobjstack->peek(depth);
						string         key       = objectStringValue(okey);
						VirtualObject *reference = astobjstack->peek(1, depth);
						
						if (defined(reference) && defined(okey)) {
							VirtualObject *o = reference->get(element->scope, &key);
							
							if (!defined(o)) {
								if (!astobjstack->push(o = o ? o : new Undefined, depth - 1)) {
									stackoverflow_error(this, OBJECT_STACK);
									return;
								}
								reference->put(element->scope, &key, o);
								element->target = NULL;
								return;
							}
							element->data   |=  FLAG_0;
							element->data   |=  FLAG_3;
							string key       = node->type == PRE_INC ? "__operator++" : "__operator--";
							nativeCall(element->scope, o->get(element->scope, &key), 1, o);
						} else {
							element->target = NULL;
							if (!astobjstack->push(new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
						}
						
					} else {
						// Left calculated
						element->target = element->target->next;
						element->data  |= FLAG_2;
					}
					break;
				}
				
				case FIELD: {
					// FLAG_1: reference calculated
					// FLAG_2: result    calculated						
					
					if (element->target == NULL)
						if (element->data & FLAG_1)
							return;
						else {
							element->scope  = element->next ? element->next->scope : NULL;
							element->data  |= FLAG_1;
							element->target = node->left->left;
						}
					else if (element->data & FLAG_2) {
						element->data   &= ~FLAG_0;
						element->data   &= ~FLAG_2;
						element->target  = NULL;
						
						VirtualObject *reference = astobjstack->peek(1, depth);
						VirtualObject *result    = astobjstack->peek(depth);
						
						if (defined(result)) {
							if (!astobjstack->push(result, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							reference->put(element->scope, (string*) node->left->objectlist->object, result);
						} else {
							if (!astobjstack->push(result = result ? result : new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
							reference->put(element->scope, (string*) node->left->objectlist->object, result);
						}
						
					} else {
						// Left calculated
						element->data           |= FLAG_2;
						VirtualObject *reference = astobjstack->peek(depth);
						
						if (defined(reference)) {
							element->data   |=  FLAG_0;
							element->data   |=  FLAG_2;
							VirtualObject *o = reference->get(element->scope, (string*) node->left->objectlist->object);
							string key = node->type == PRE_INC ? "__operator++" : "__operator--";
							nativeCall(element->scope, o->get(element->scope, &key), 1, o);
						} else {
							element->target = NULL;
							if (!astobjstack->push(new Undefined, depth - 1)) {
								stackoverflow_error(this, OBJECT_STACK);
								return;
							}
						}
					}
					break;
				}
			}
			break;
		};

		case MEMBER: {
			// FLAG_1: left calculated
			// FLAG_2: right calculated
			
			if (element->target == NULL)
				if (element->data & FLAG_1)
					return;
				else {
					element->scope  = element->next ? element->next->scope : NULL;
					element->data  |= FLAG_1;
					element->target = node->left;
				}
			else if (element->data & FLAG_2) {
				// left & Right calculated
				element->data  &= ~FLAG_2;
				element->target = NULL;
				
				string key               = objectStringValue(astobjstack->peek(depth));
				VirtualObject *reference = astobjstack->peek(1, depth);
				
				if (!defined(reference)) {
					if (!astobjstack->push(new Undefined, depth - 1)) 
						stackoverflow_error(this, OBJECT_STACK);
					return;
				}
				
				VirtualObject *o         = reference->get(element->scope, &key);
				
				if (defined(o)) {
					if (!astobjstack->push(o, depth - 1)) 
						stackoverflow_error(this, OBJECT_STACK);
				} else
					if (!astobjstack->push(new Undefined, depth - 1)) 
						stackoverflow_error(this, OBJECT_STACK);
				
			} else {
				// Left calculated
				element->target = element->target->next;
				element->data  |= FLAG_2;
			}
			break;
		}
		
		case FIELD: {
			// FLAG_1: executed
			
			if (element->target == NULL)
				if (element->data & FLAG_1)
					return;
				else {
					element->scope  = element->next ? element->next->scope : NULL;
					element->data  |= FLAG_1;
					element->target = node->left;
				}
			else {
				// Left calculated
				element->target = NULL;
					
				VirtualObject *o = astobjstack->head->object->get(element->scope, ((string*) node->objectlist->object));
				
				if (defined(o)) {
					if (!astobjstack->push(o, depth - 1)) 
						stackoverflow_error(this, OBJECT_STACK);
				} else
					if (!astobjstack->push(new Undefined, depth - 1)) 
						stackoverflow_error(this, OBJECT_STACK);
			}
			break;
		}
	};
};

// Called when node is never more been visited
void ASTExecuter::leave(ASTExecuterElement *element, ASTNode *node, int depth) {
	
	// DATA12_SECTION can be used for passing node type without passing node
	if (node == NULL && !element->data) 
		execution_error("NODE = NULL");
	
#ifdef AST_VISIT_PRINT
	csetcolor(CRED);
	DEBUG_IDENT(depth - 1)
	printf(tokenToString(node ? node->type : ((element->data & DATA10_MASK) >> DATA10_OFFSET)));
	putchar(10);
	cwhite;
#endif
	
	// Dispose scope
	switch (node ? node->type : ((element->data & DATA10_MASK) >> DATA10_OFFSET)) {
		case ASTROOT: 
			break;
			
		case TRY:
			if (element->data & FLAG_3) {
				GC.gc_deattach_root(element->scope);
				GC.gc_collect();
			}
			break;
			
		case FOR:
			if (element->scope)
				GC.gc_deattach_root(element->scope);
			GC.gc_collect();
			break;
			
		case WHILE:
			GC.gc_collect();
			break;
			
		case BLOCK:
			if (element->scope) 
				GC.gc_deattach_root(element->scope);
			GC.gc_collect();
			break;
			
		case CALL:
			if (element->data & FLAG_5 && element->scope) 
				GC.gc_deattach_root(element->scope);
			GC.gc_collect();
			break;
			
		case NATIVE_CALL:
			if (element->data & FLAG_2 && element->scope) 
				GC.gc_deattach_root(element->scope);
			GC.gc_collect();
			break;
		
		case EMPTY:
			break;
		
		case IMPORTED_SCRIPT:
			GC.gc_deattach_root(element->scope);
			delete element->scope->context;
			delete node;
			GC.gc_collect();
			break;
		
		// case EXECUTED_SCRIPT:
		// 	break;
	}
	
	// printf("node type: %d %s\n", node ? node->type : ((element->data & DATA10_MASK) >> DATA10_OFFSET), tokenToString(node ? node->type : ((element->data & DATA10_MASK) >> DATA10_OFFSET)));
	// Dispose returned value 
	astobjstack->popForLevel(depth);
	aststacktrace->popForLevel(depth);
	
	element->attached_node = NULL;
	element->target        = NULL;
	element->scope         = NULL;
	element->data          = 0;
	
	aststacktrace->collect();
	astobjstack->collect();
	aststack->collect();
};

void ASTExecuter::begin(Context *context, ASTNode *root) {
	aststack->push();
	aststack->head->attached_node = root;
	aststack->head->scope         = context->scope;
	context->executer             = this;
	ASTNode *node                 = root;
	
	while (!_error && (node != NULL || (aststack->head && (aststack->head->data & FLAG_0)))) {
		if (!_global_exec_state)
			break;
		
		try {
			visit(aststack->head, aststack->head ? aststack->head->attached_node : NULL, aststack->size);
		} catch(...) {
			raiseError("Unhandled Exception");
		}
		if (_error)
			break;
		
		// Check for target == NULL && FLAG_0 == 0
		if (aststack->head->data & FLAG_0)
			continue;
		
		if (aststack->head->target == NULL) {
			leave(aststack->head, aststack->head ? aststack->head->attached_node : NULL, aststack->size);
			
			ASTExecuterElement *e = aststack->pop();
			
			node = e ? e->attached_node : NULL;
		} else {
			node = aststack->head->target;
			
			if (!aststack->push()) {
				stackoverflow_error(this, EXECUTION_STACK);
				break;
			}
			aststack->head->attached_node = node;
		}
	}
	
	// Leave all nodes for correct suspend
	while (aststack->head) {
		leave(aststack->head, aststack->head ? aststack->head->attached_node : NULL, aststack->size);
		aststack->pop();
	}
	
	// </execution>
};

ExecuterResult ASTExecuter::beginFunction(Scope *scope, VirtualObject *reference, VirtualObject *function, int argc, ...) {
	// Parent scope is set to NULL.
	// This scope is used in scope's sandwich during function call.
	// [Definition Scope]
	//  \
	//   [Enclosing object proxy Scope]
	//    \
	//     [Function Scope]
	
	if (!function || function->type == UNDEFINED || function->type == TNULL) {
		ExecuterResult r;
		r.object = NULL;
		r.type   = ERT_UNDEF;
	}
	
	// New alternate root scope for overwritting access to the global scope.
	Scope  *new_scope = new ProxyScope(NULL, scope);
	Context *new_context = new Context;
	new_context->script_dir_path  = new FileUrl(scope ? scope->context->script_dir_path->path : "/");
	new_context->script_file_path = new FileUrl("virtual");
	new_context->executer = this;
	new_context->scope    = new_scope;
	new_scope->context    = new_context;
	
	// Set up error handling for this executer.
	VirtualObject *result_error = NULL;
	bool result_error_flag      = 0;
	VirtualObject **result_error_ptr = &result_error;
	bool *result_error_flag_ptr = &result_error_flag;
	
	auto la = [=] (VirtualObject *error) { *result_error_ptr = error; *result_error_flag_ptr = 1; };
	static auto static_variable = la;
	this->error_handler = [](VirtualObject *error) { return static_variable(error); };
	
	if (function->type != CODE_FUNCTION) {
		Scope *clscope;
		// Function scope.
		if (reference)
			clscope = new Scope(new ProxyScope(new_scope, reference));
		else 
			clscope = new Scope(new_scope);
		
		
		GC.gc_attach_root(new_scope);
		GC.gc_attach_root(clscope);
		
		VirtualObject *f = function->call(clscope, argc, (VirtualObject**) (&argc + sizeof(int*)));
		
		GC.gc_deattach_root(clscope);
		GC.gc_deattach_root(new_scope);
		
		GC.gc_lock(f);
		GC.gc_lock(result_error);
		GC.gc_collect();
		GC.gc_unlock(result_error);
		GC.gc_unlock(f);
		
		ExecuterResult r;
		if (result_error_flag) {
			r.object = result_error;
			r.type   = ERT_ERROR;
		} else {
			r.object = f;
			r.type   = ERT_OBJECT;
		}
		
		return r;
	}
	
	GC.gc_attach_root(new_scope);
	
	aststack->push();
	aststack->head->attached_node = NULL;
	aststack->head->data         |= (FUNCTIONROOT << DATA10_OFFSET);
	aststack->head->scope         = new_scope;
	// Passing (...) to (...)
	va_list vargs;
    va_start(vargs, argc);
	nativeCall(new_scope, reference, function, argc, vargs);
	va_end(vargs);
	
	ASTNode *node = aststack->head->attached_node;
	
	VirtualObject *ret_object = NULL;
	
	while (!result_error_flag && !_error && (node != NULL || (aststack->head && (aststack->head->data & FLAG_0)))) {
		if (!_global_exec_state)
			break;
		
		try {
			visit(aststack->head, aststack->head ? aststack->head->attached_node : NULL, aststack->size);
		} catch(...) {
			raiseError("Unhandled Exception");
		}
		if (_error)
			break;
		
		// Check for target == NULL && FLAG_0 == 0
		if (aststack->head->data & FLAG_0)
			continue;
		
		if (aststack->head->target == NULL) {
			if (((aststack->head->data & DATA10_MASK) >> DATA10_OFFSET) == FUNCTIONROOT) {
				// Returned value
				ret_object = astobjstack->peek(aststack->size);
				GC.gc_lock(ret_object);
				break;
			}
			leave(aststack->head, aststack->head ? aststack->head->attached_node : NULL, aststack->size);
			
			ASTExecuterElement *e = aststack->pop();
			
			node = e ? e->attached_node : NULL;
		} else {
			node = aststack->head->target;
			
			if (!aststack->push()) {
				stackoverflow_error(this, EXECUTION_STACK);
				break;
			}
			aststack->head->attached_node = node;
		}
	}
	
	GC.gc_lock(result_error);
	
	// Leave all nodes for correct suspend
	while (aststack->head) {
		if (((aststack->head->data & DATA10_MASK) >> DATA10_OFFSET) == FUNCTIONROOT) {
			// Returned value
			ret_object = astobjstack->peek(aststack->size);
			GC.gc_lock(ret_object);
		}
		leave(aststack->head, aststack->head ? aststack->head->attached_node : NULL, aststack->size);
		aststack->pop();
	}
	
	astobjstack->popForLevel(0);
	
	GC.gc_collect();
	
	GC.gc_unlock(ret_object);
	GC.gc_unlock(result_error);
	GC.gc_deattach_root(new_scope);
	
	ExecuterResult r;
	if (result_error_flag) {
		r.object = result_error;
		r.type   = ERT_ERROR;
	} else {
		r.object = ret_object;
		r.type   = ERT_OBJECT;
	}
	
	// XXX: Модификация Stack Trace. 
	// ASTExecuter.AppendSTackTrace - добавляет стактрейс переданного executer'а к днищу уже имеющегося.
	// В случае с несколькими потоками - просто копировать стактрейс в точке вызова.
	// В случае с анонимным вызовом функции beginFunction - дополнительный параметр - имя 
	// функции. Функция выполняется в виртуальном контексте, а потому ей не нужно имя файла и расположение.
	// Вместо прохода по контекстам - метки с именем файлов.
	
	
	return r;
};
	

// Поздравляю, ты достиг дна.
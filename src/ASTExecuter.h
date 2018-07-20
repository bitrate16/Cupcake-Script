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
 * Defines AST exeuter class.
 * Executer.begin takes as argument the node, 
 * which is tha start of non-recursive visitng using stack.
 * Executer allows passing interrupts such as return/continue/break/raise.
 * Executer is not thread-safe and only assigned with the current thread.
 */

// XXX: *Stack::collect() -> collect only for size less in power of two. (size < (log2[alloc_size] >> 1))
 
#ifndef AST_EXECUTER_H
#define AST_EXECUTER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "DebugUtils.h"
#include "ColoredOutput.h"
#include "TokenNamespace.h"
#include "string.h"
#include "LinkedMap.h"

#include "Context.h"
#include "objects/Scope.h"
#include "ASTNode.h"
#include "GarbageCollector.h"
#include "FileUrl.h"

// data:
// bbbb|bbbbbbbbbbbb|bbbbbbbb|bbbbbbbb
// ^result flags     |        |
//      ^result type |        |
//                   ^user block #1
//                            ^user block #2

#define FLAG_CLEAR     0b00000011111111111111111111111111
#define FLAG6_MASK     0b11111100000000000000000000000000
#define DATA10_MASK    0b00000011111111110000000000000000
#define USER16_MASK    0b00000000000000001111111111111111
#define FLAG_0         0b10000000000000000000000000000000
#define FLAG_1         0b01000000000000000000000000000000
#define FLAG_2         0b00100000000000000000000000000000
#define FLAG_3         0b00010000000000000000000000000000
#define FLAG_4         0b00001000000000000000000000000000
#define FLAG_5         0b00000100000000000000000000000000

// FLAG_0: used for forcing reexecution of the same node even if target is NULL / implicit mode
// FLAG_1: used for handling multiple executions of single node
// FLAG_2, FLAG_3, FLAG_4, FLAG_5: i dunno lol

#define DATA10_OFFSET  16
#define FLAGS_OFFSET   26

#define AST_EXECUTION_STACK_SIZE 16384
#define AST_OBJECT_STACK_SIZE    16384
#define AST_STACK_TRACE_SIZE     16384

#define MIN_AST_STACK_SIZE      16

#define EXECUTION_STACK  0
#define OBJECT_STACK     1
#define STACKTRACE_STACK 2

// #define AST_VISIT_PRINT
// #define VARIABLES_PRINT


struct ASTExecuterElement {
	
	// Result of visit
	int                 data;
	// Next node to jump
	ASTNode          *target;
	// Parent node of this element
	ASTNode   *attached_node;
	// Attached (enclosing) Scope
	Scope             *scope;
	// Next -> root
	ASTExecuterElement *next;
	// Prev -> top
	ASTExecuterElement *prev;
	
	ASTExecuterElement();
	
	void printData(void);
};

// ExecuterStack stack. 
// Used for performing non-recursive iterations over AST.
struct ASTExecuterStack {
	
	int   max_size;
	int       size;
	int alloc_size;
	
	// Points to end of stack
	ASTExecuterElement    *top;
	// Points to the current element
	ASTExecuterElement   *head;
	// Points to the beginning of stack
	ASTExecuterElement *bottom;
	
	ASTExecuterStack(int mx_size);
	
	~ASTExecuterStack();
	
	// head->next instead of head
	ASTExecuterElement *pop();
	
	// Collect unused frames
	void collect();
	
	int push();
};


struct ASTObjectElement {
	// Next -> root
	ASTObjectElement               *next;
	// Prov -> top
	ASTObjectElement               *prev;
	// Assigned object
	VirtualObject                *object;
	// Level, assigned to this object
	int                            level;
	
	ASTObjectElement();
	
	~ASTObjectElement();
};

// ObjectStack stack.
// Used for storing objects in temporary storage during execution.
// Acts as hardware stack in assemly, allows permitting function 
// calls & value returns without direct execution interrupt.
// 
// Mark this struct to extend GC_Object, 
// so we can save all values, located in stack, 
// while perfoming GC.gc_collect
struct ASTObjectStack : GC_Object {
	
	int   max_size;
	int       size;
	int alloc_size;
	
	// Points to end of stack
	ASTObjectElement    *top;
	// Points to the current element
	ASTObjectElement   *head;
	// Points to the beginning of stack
	ASTObjectElement *bottom;
	
	ASTObjectStack(int mx_size);
	
	~ASTObjectStack();
	
	// Pop all objects till object with given initiance level (>)
	// Used for fast pop at emergency exit or fast clear stack from external data on incorrect node termination
	// Level - is the depth of the ASTExecutionElement
	void popTillLevel(int level);
	
	// Pop all objects till object with given initiance level (>=)
	// Used for fast pop at emergency exit or fast clear stack from external data on incorrect node termination
	// Level - is the depth of the ASTExecutionElement
	void popForLevel(int level);
	
	// Pop & dispose()
	// Pop only if given level >= current
	VirtualObject *pop(int level);
	
	// Collect unused frames
	void collect();
	
	// moves head level up (or null of head->level < level)
	void pullUp(int level);
	
	VirtualObject *peek(int level);
	
	VirtualObject *peek(int offset, int level);
	
	// Push & point
	int push(VirtualObject *o, int level);
	
	void print(void);
	
	void mark();
	
	void finalize();
};


enum ASTExecuterStackTraceElementType {
	// <anonymous>
	ASTESTE_UNDEFINED,
	// <name>
	ASTESTE_NAME,
	// <file>
	ASTESTE_FILE
};

struct ASTExecuterStackTraceElement {
	// Next -> root
	ASTExecuterStackTraceElement    *next;
	// Prov -> top
	ASTExecuterStackTraceElement    *prev;
	// Assigned trace name
	string                      tracename;
	// Assigned trace lineno
	int                       tracelineno;
	// Level, assigned to this trace record
	int                             level;
	// Defins current storage type. If not set, name should be <anunymous>
	ASTExecuterStackTraceElementType type;
	
	ASTExecuterStackTraceElement();
	
	// ~ASTExecuterStackTraceElement();
};

// StackTrace stack.
// Used for storing hierarchical function call 
// names & line numbers for further debug & stack 
// trace generation on raising errors.
struct ASTExecuterStackTrace {
	
	// ^ +index
	// | ---------------------- |
	// |                        |  
	// |   POSITIVE SECTION     | push, pop, popTillLevel, popForLevel
	// |                        |
	// 0 ---------------------- |
	// |                        |
	// |   NEGATIVE SECTION     | insert, desert, normalize
	// |                        |
	// | ---------------------- |
	// v -index
	
	// Number of max allowed alloc frames
	int   max_size;
	// Number of used frames
	int       size;
	// Number of positive used frames
	int   pos_size;
	// Number of allocated frames
	int alloc_size;
	
	// Points to end of stack
	ASTExecuterStackTraceElement    *top;
	// Points to the current element
	ASTExecuterStackTraceElement   *head;
	// Points to the beginning of stack
	ASTExecuterStackTraceElement *bottom;
	
	ASTExecuterStackTrace(int mx_size);
	
	~ASTExecuterStackTrace();
	
	
	// Inserts stacktrace frame in negative section
	int insert(int lineno, string name);
	
	int insert(string file);
	
	int insert(int lineno);
	
	// Removes single stacktrace frame from the bottom
	void desert();
	
	// Removes all stacktrace frames from the bottom
	void normalize();
	
	// Removes all StackTrace frames
	void clear();
	
	void pop(int level);
	
	void popForLevel(int level);
	
	void collect();
	
	int push(int level, int lineno, string name);
	
	int push(int level, string file);
	
	int push(int level, int lineno);
};


enum ExecuterResultType {
	ERT_ERROR,
	ERT_OBJECT,
	// Returned when NULL, null of undefined is passed as pointer to function.
	ERT_UNDEF
};

// Result of Executer.begin on object as function.
struct ExecuterResult {
	VirtualObject   *object;
	ExecuterResultType type;
};

// Type of error handler function
typedef void (*error_handler_function_type) (VirtualObject*); 

// Lorem ipsum dolor sit amet, consectetur adipiscing elit, 
// sed do eiusmod tempor incididunt ut labore et dolore magna 
// aliqua. Ut enim ad minim veniam, quis nostrud exercitation 
// ullamco laboris nisi ut aliquip ex ea commodo consequat. 
// Duis aute irure dolor in reprehenderit in voluptate velit 
// esse cillum dolore eu fugiat nulla pariatur. Excepteur sint 
// occaecat cupidatat non proident, sunt in culpa qui officia 
// deserunt mollit anim id est laborum.
struct ASTExecuter {	
	bool                               _error;
	ASTExecuterStack                *aststack;
	ASTObjectStack               *astobjstack;
	ASTExecuterStackTrace      *aststacktrace;
	error_handler_function_type error_handler;
	
	ASTExecuter();
	
	~ASTExecuter();

	// Copies ASTStackTrace from given ASTExecuter into this executer.
	int insertStackTrace(ASTExecuter *executer);
	
	// Returs file relative to current executer directory
	// Or absolute path if file does not exist
	FILE *getRelativeFile(string *filename);
	
	void execution_error(const char *msg);
	
	// use: nativeCall(caller_scope, func, 3, *obj1, *obj2, *obj3)
	//                                        ^      ^      ^ - pointers
	void nativeCall(Scope *scope, VirtualObject *f, int argc, ...);
	
	// use: nativeCall(caller_scope, ref, func, 3, *obj1, *obj2, *obj3)
	//                                             ^      ^      ^ - pointers
	void nativeCall(Scope *scope, VirtualObject *r, VirtualObject *f, int argc, ...);
	
	// OverHandlers for operators execution calls
	
	void binaryOperatorCall(ASTExecuterElement *element, ASTNode *node, int depth, const char *opmember);
	
	void binaryLogicalOperatorCall(ASTExecuterElement *element, ASTNode *node, int depth, const char *opmember);
	
	void unaryOperatorCall(ASTExecuterElement *element, ASTNode *node, int depth, const char *opmember);
	
	// </OverHandlers>
	
	void raiseError(const char *message);
	
	void raiseError(string message);
	
	void raiseError(VirtualObject *error);
	
	// Called every time node is visited
	void visit(ASTExecuterElement *element, ASTNode *node, int depth);
	
	// Called when node is never more been visited
	void leave(ASTExecuterElement *element, ASTNode *node, int depth);
	
	void begin(Context *context, ASTNode *root);
	
	// Performs call of the function in EMPTY executer. 
	// Returns result of the execution.
	// Scope expected as Non NULL object
	ExecuterResult beginFunction(Scope *scope, VirtualObject *reference, VirtualObject *function, int argc, ...);
};

#endif

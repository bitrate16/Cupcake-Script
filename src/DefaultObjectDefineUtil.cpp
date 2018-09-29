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

#include "DefaultObjectDefineUtil.h"

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
#include "objects/NativeLoaderType.h"

#include "objects/ObjectConverter.h"

#include <cstdio>
#include "string.h"
#include "ptr_wrapper.h"
#include "Parser.h"

#include "exec_state.h"


static VirtualObject* function_default(Scope *scope, int argc, VirtualObject **args) {
	return NULL;
};


// - - - - - - - - - G C


// Define functions for operating with GarbageCollector.
static VirtualObject* function_GC_numobjects(Scope *scope, int argc, VirtualObject **args) {	
	return new Integer(GC.gc_size);
};

static VirtualObject* function_GC_numroots(Scope *scope, int argc, VirtualObject **args) {	
	return new Integer(GC.gc_roots_size);
};

static VirtualObject* function_GC_collect(Scope *scope, int argc, VirtualObject **args) {	
	GC.gc_collect();
	return NULL;
};

static void define_GC(Scope *scope) {
	Object *GC_Obj = new Object;
	GC_Obj->table->put(string("numObjects"), new NativeFunction(&function_GC_numobjects));
	GC_Obj->table->put(string("numRoots"),   new NativeFunction(&function_GC_numroots));
	GC_Obj->table->put(string("collect"),    new NativeFunction(&function_GC_collect));
	
	scope->table->put(string("GC"), GC_Obj);
};


// - - - - - - - - - R U N T I M E


// includeFile(filename[, Object:context])
// This fucntion is macro, so it will NOT return any values.
// This function is NOT expression-safe.
static VirtualObject* function_Runtime_importFile(Scope *scope, int argc, VirtualObject **args) {	
	if (argc == 0 || !scope || !scope->context->executer)
		return NULL;
	
	// Parse given file into ASTNode sub-tree 
	// and attach it on execution by replacing 
	// type AST_ROOT with type INCLUDED_SCRIPT
	
	string _filename = objectStringValue(args[0]);
	
	FileUrl *env_file_path = NULL;
	char *cfilename   = _filename.toCString();
	FileUrl *filename = new FileUrl(cfilename);
	free(cfilename);
	
	FILE *f = filename->open("r");
	
	if (!f) {
		FileUrl *extendedfilepath = new FileUrl(scope->context->script_dir_path, filename);
		delete filename;
		f = extendedfilepath->open("r");
		
		if (!f) {
			delete extendedfilepath;
			scope->context->executer->raiseError("File not found");
			return NULL;
		} else
			env_file_path = extendedfilepath;
	} else
		env_file_path = filename;
	
	FileUrl *temp = env_file_path->getRealPath();
	delete env_file_path;
	env_file_path = temp;
	
	FAKESTREAM  fs(f);
	TokenStream ts;
	Parser       p;
	ts.init(&fs);
	p.init(&ts);
	
	ASTNode *tree = p.parse();
	
	if (!tree) {
		scope->context->executer->raiseError("Parser error");
		return NULL;
	}
	
	tree->type = IMPORTED_SCRIPT;
	
	ASTExecuter *executer       = scope->context->executer;
	Context     *base_context   = scope->context;
	Context     *new_context    = new Context();
	Scope       *new_scope      = NULL;
	ASTExecuterElement *element = NULL;
	
	new_context->script_file_path = env_file_path;
	new_context->script_dir_path  = env_file_path->getDirectory();
	new_context->executer         = executer;
	
	if (!executer->aststack->push()) {
		executer->raiseError("Execution stack overflow");
		delete new_context;
		return NULL;
	}
	
	element = executer->aststack->head;
	element->attached_node = tree;
	
	// Create new Scope for execution
	// Try to get enclosing scope for call
	// [Enclosing Scope]     <--
	//  \
	//   [ProxyScope] (optional)
	//    \
	//     [Function Scope]
	
	if (!scope->parent)
		return NULL; // Error: no scope
	
	if (scope->parent->type == PROXY_SCOPE)
		if (!scope->parent->parent) {
			executer->raiseError("Null scope");
			delete new_context;
			return NULL;
		} else 
			if (argc > 1 && args[1]) 
				new_scope = new ProxyScope(scope->parent->parent, args[1]);
			else
				new_scope = new ProxyScope(scope->parent->parent, NULL);
	else 
		if (argc > 1 && args[1]) 
			new_scope = new ProxyScope(scope->parent->parent, args[1]);
		else
			new_scope = new ProxyScope(scope->parent->parent, NULL);
	
	element->scope     = new_context->scope = new_scope;
	new_scope->context = new_context;
	new_context->scope = new_scope;
	GC.gc_attach_root(new_scope);
	
	element->target = tree->left;
	executer->aststacktrace->push(executer->aststack->size, string(new_context->script_file_path->path));
	
	return NULL;
};

static VirtualObject* function_Runtime_exit(Scope *scope, int argc, VirtualObject **args) {	
	if (argc == 0)
		_interpreter_exit();
	else
		_interpreter_exit(objectIntValue(args[0]));
	
	return NULL;
};

static VirtualObject* function_Runtime_system(Scope *scope, int argc, VirtualObject **args) {
	// printf("function_system@call()\n");
	if (argc == 0)
		return NULL;
	
	string s = objectStringValue(args[0]);
	ptr_wrapper wrapper(s.toCString(), PTR_ALLOC);
	system((char*) wrapper.ptr);
	return NULL;
};

static VirtualObject* function_Runtime_defaultErrorHandler(Scope *scope, int argc, VirtualObject **args) {
	if (argc) {
		if (args[0] && args[0]->type == ERROR) {
			Error *e = (Error*) args[0];
			e->print();
		} else
			printf("%S\n", objectStringValue(args[0]).toCharSequence());
	}
	
	_interpreter_exit(1);
	
	return NULL;
};

static void define_Runtime(Scope *scope) {
	Object *Runtime_Obj = new Object;
	Runtime_Obj->table->put(string("importFile"),          new NativeFunction(&function_Runtime_importFile));
	Runtime_Obj->table->put(string("defaultErrorHandler"), new NativeFunction(&function_Runtime_defaultErrorHandler));
	Runtime_Obj->table->put(string("defaultExitListener"), new NativeFunction(&function_default));
	Runtime_Obj->table->put(string("exit"),                new NativeFunction(&function_Runtime_exit));
	Runtime_Obj->table->put(string("system"),              new NativeFunction(&function_Runtime_system));
	
	scope->table->put(string("Runtime"), Runtime_Obj);
};


// - - - - - - - - - C O N T E X T

// Returns path associated with that context
static VirtualObject* function_Context_getPath(Scope *scope, int argc, VirtualObject **args) {
	return new String(scope->context->script_dir_path->path);
};

// Returns file associated with that context
static VirtualObject* function_Context_getFile(Scope *scope, int argc, VirtualObject **args) {
	return new String(scope->context->script_file_path->path);
};

void define_Context(Scope *scope) {
	Object *Context_Obj = new Object;
	Context_Obj->table->put(string("getPath"), new NativeFunction(&function_Context_getPath));
	Context_Obj->table->put(string("getFile"), new NativeFunction(&function_Context_getFile));
	
	scope->table->put(string("Context"), Context_Obj);
};


// - - - - - - - - - S T D I O


// Read line of input
static VirtualObject* function_stdio_readln(Scope *scope, int argc, VirtualObject **args) {
	// printf("readln@call()\n");
	
	string *s = new string();
	wint_t c;
	
	while (true) {
		c = fgetwc(stdin);
		
		if (c == WEOF)
			break;
		
		if (c == '\n')
			break;
		
		*s += c;
	}
	
	return new String(s);
};

// Read single character
static VirtualObject* function_stdio_read(Scope *scope, int argc, VirtualObject **args) {
	// printf("read@call()\n");
	
	string s;
	wint_t c = getwchar();
	if (c == EOF)
		return new Undefined;
	s += (wchar_t) c;
	
	return new String(new string(s));
};

inline static bool c_digit(wint_t c) {
	return '0' <= c && c <= '9';
};

// Read integer from input
static VirtualObject* function_stdio_readInt(Scope *scope, int argc, VirtualObject **args) {
	// printf("readInt@call()\n");
	
	int i;
	int result = wscanf(L"%d",&i);
	if (result)
		return new Integer(i);
	
	return new Undefined();
};

// Read double from input
static VirtualObject* function_stdio_readDouble(Scope *scope, int argc, VirtualObject **args) {
	// printf("readDouble@call()\n");
	
	double d;
	int result = wscanf(L"%lf", &d);
	if (result == -1)
		return new Double(d);
	
	return new Undefined();
};

static VirtualObject* function_stdio_println(Scope *scope, int argc, VirtualObject **args) {
	// printf("println@call()\n");
	if (argc == 0) {
		printf("\n");
		return new Integer;
	}
	
	string s = objectStringValue(args[0]);
	s.print();
	printf("\n");
	return new Integer(s.length);
};

static VirtualObject* function_stdio_print(Scope *scope, int argc, VirtualObject **args) {
	// printf("print@call()\n");
	if (argc == 0)
		return new Integer;
	
	string s = objectStringValue(args[0]);
	s.print();
	return new Integer(s.length);
};

static void define_stdio(Scope *scope) {
	Object *stdio_Obj = new Object;
	stdio_Obj->table->put(string("print"),      new NativeFunction(&function_stdio_print));
	stdio_Obj->table->put(string("println"),    new NativeFunction(&function_stdio_println));
	stdio_Obj->table->put(string("readln"),     new NativeFunction(&function_stdio_readln));
	stdio_Obj->table->put(string("readInt"),    new NativeFunction(&function_stdio_readInt));
	stdio_Obj->table->put(string("readDouble"), new NativeFunction(&function_stdio_readDouble));
	stdio_Obj->table->put(string("read"),       new NativeFunction(&function_stdio_read));
	
	scope->table->put(string("stdio"), stdio_Obj);
};


// - - - - - - - - - P L A T F O R M

static void define_Platform(Scope *scope) {
	Object *Platform_Obj = new Object;
	Platform_Obj->table->put(string("os_name"),      
#if   defined_WIN32
		new String("windows")
#elif defined unix
		new String("unix")
#elif defined __APPLE__
		new String("apple")
#elif defined __linux__
		new String("linux")
#elif defined __FreeBSD__
		new String("FreeBSD")
#else
		new String("undefined")
#endif
	);
	
	scope->define(string("Platform"), Platform_Obj);
};


// Define all global functions & fields.
static void define_global_fields(Scope *scope) {	
	define_GC(scope);
	define_Runtime(scope);
	define_Context(scope);
	define_stdio(scope);
	define_Platform(scope);
};


// Define all types & global fields
void defineTypes(Scope *root_scope) {
	define_object(root_scope);
	define_array(root_scope);
	define_scope(root_scope);
	define_null(root_scope);
	define_undefined(root_scope);
	define_string(root_scope);
	define_integer(root_scope);
	define_double(root_scope);
	define_boolean(root_scope);
	define_native_function(root_scope);
	define_code_function(root_scope);
	define_error(root_scope);
	define_native_loader(root_scope);
	
	define_global_fields(root_scope);
};


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
 * Main program file. 
 * Includes all needed headers. 
 * 
 * P.S. All ducks migrate from the 
 * resources listed below and are 
 * kept in comfortable living conditions.
 * https://textart.io/art/tag/duck/1
 */

#include <cstdlib>
#include <cstdio>
#include <locale.h>

// Handle interrupts
#include <signal.h>
#include <unistd.h>

// #define PRINT_CODE_ENABLED

#include "DebugUtils.h"

#include "TokenNamespace.h"
#include "string.h"
#include "RawToken.h"
#include "Utils.h"
#include "TokenStream.h"
#include "ASTNode.h"
#include "Parser.h"
#include "ASTPrinter.h"
#include "ColoredOutput.h"
#include "GarbageCollector.h"
#include "ASTExecuter.h"
#include "DefaultObjectDefineUtil.h"

#include "objects/NativeLoaderType.h"

#include "exec_state.h"


// _______
// M A I N 

TokenStream    *tstream = NULL;
Parser          *parser = NULL;
ASTNode           *root = NULL;
FAKESTREAM    *instream = NULL;

ASTExecuter   *executer = NULL;
Context *global_context = NULL;


// Handler for system signals
void signal_callback_handler(int signum) {
	_interpreter_int();
};


// 3 in 1. Only for 0.95$ now!
// Passing raw arguments with offset
void compactMain(const char *path, int argc, char **argv) {		
	tstream = new TokenStream;
	parser  = new Parser;
	
	tstream->init(instream);
	parser->init(tstream);
	
	// Parse code
	root = parser->parse(); // <- BUG
	
	delete instream;
	instream = NULL;
	
	cgreen;
	DEBUG("DELETE PARSER")
	delete parser;
	parser = NULL;
	DEBUG("DELETE TSTREAM")
	delete tstream;
	tstream = NULL;
	cwhite;

	if (!root || _global_int_state)
		return 0;
	
	// Print source if needed
#ifdef PRINT_CODE_ENABLED
	printAST(root);
	putchar(10);
#endif	

	// Define global context & variables & other stuff
	global_context                 = new Context();
	global_context->scope          = new Scope(NULL);
	global_context->scope->context = global_context;
	GC.gc_attach_root(global_context->scope);
	defineTypes(global_context->scope);
	
	global_context->script_file_path = FileUrl::getRealPath(path == "/" ? "virtual" : path);
	global_context->script_dir_path  = path == "/" ? path : global_context->script_file_path->getDirectory();
	
	/* ERROR HERE
	// Append arguments
	Array *arguments_array = new Array();
	for (int i = 0; i < argc; ++i)
		arguments_array->array->push(new String(new string(argv[i])));
	global_context->scope->table->put(string("arguments"), arguments_array);
	*/
	
	// Create executer & run code
	executer = new ASTExecuter;
	executer->begin(global_context, root);
	
	// Collect garbage
	GC.gc_deattach_root(global_context->scope);
	
	// Deallocate memory
	cgreen;
	DEBUG("DELETE ROOT")
	delete root;
	root = NULL;
	DEBUG("DELETE EXECUTER")
	cwhite;
	delete executer;
	executer = NULL;
	// delete global_context;
	global_context = NULL;
	
	GC.gc_dispose();
	_global_exec_state = 0;
	
	// Unload loaded modules to free memory.
	unload_loaded_modules();
};

int main(int argc, char **argv) {
	setlocale(LC_ALL,    "");
	setlocale(LC_CTYPE , "");
	
	signal(SIGINT,  signal_callback_handler);
	signal(SIGTERM, signal_callback_handler);
	
	if (argc == 1) {
		printf("Empty args.\nUse -h for help.\n");
		return 0;
	}
	
	if (strcmp(argv[1], "-f"))
		if (argc >= 3) {
			cbegin;
			
			// Initialize InputStream
			FILE *fin = fopen(argv[2], "r");
			if (!fin) {
				printf("File %s not found.\n", argv[2]);
				return 0;
			}
			
			instream = new FAKESTREAM(fin);
			
			compactMain(argv[2]);
		} else 
			printf("No file specified.\n");
	else if (strcmp(argv[1], "-s"))
		if (argc >= 3) {
			cbegin;
			instream = new FAKESTREAM(argv[2]);
			
			compactMain("/", argc - 3, argv + 3 * sizeof(char*));
		} else 
			printf("No source specified.\n");
	else if (strcmp(argv[1], "-i")) {
		cbegin;
		instream = new FAKESTREAM();
			
		compactMain("/", argc, argv);
	} else if (strcmp(argv[1], "-h")) {
		printf("Help:\n");
		printf(":: -f <file path>: execute script from file.\n");
		printf(":: -s <file path>: execute script from command line arguments.\n");
		printf(":: -i <file path>: execute script from STDIN.\n");
		printf(":: <file path>:    execute script from file.\n");
	} else
		if (argc >= 2) {
			cbegin;
			
			// Initialize InputStream
			FILE *fin = fopen(argv[1], "r");
			if (!fin) {
				printf("File %s not found.\n", argv[1]);
				return 0;
			}
			
			instream = new FAKESTREAM(fin);
			
			compactMain(argv[1], argc - 2, argv + 2 * sizeof(char*));
		} else 
			printf("No file specified.\n");
	
	return 0;
};

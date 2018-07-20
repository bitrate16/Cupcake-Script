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
 * Example module for demonstrating NativeLoader.
 * Defines new Hello function in global conext.
 */

// Example use for build:
// cd <project>
// bash buildso.sh src/modules/TestModule.cpp bin/TestModule.so

// Example use:
// stdio.println('Load result: ' + NativeLoader.load('../bin/TestModule.so'));
// helloFromModule();

#include "stdio.h"

#include "../objects/Scope.h"
#include "../objects/NativeFunction.h"

#ifdef __cplusplus
extern "C" {
#endif

// Test function. Say hello to my little friend.
VirtualObject *function_hello(Scope *scope, int argc, VirtualObject **args) {
	printf("Hello from TestModule\n");
	
	return NULL;
};

// Called on module being loaded by
// ck: NativeLoader.load('TestModule.so');
void onLoad(Scope *scope, int argc, VirtualObject **args) {
	printf("Module has been loaded!\n");
	
	string key = "sayHelloFromModule";
	// Append native function to the root scope
	scope->getRoot()->define(&key, new NativeFunction(&function_hello));
};

// Called on module being unloaded on program exit
void onUnload() {
	printf("Module has been unloaded!\nGoodbye!\n");
};

#ifdef __cplusplus
}
#endif
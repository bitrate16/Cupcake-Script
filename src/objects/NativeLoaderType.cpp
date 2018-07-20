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

#include <dlfcn.h>
#include <typeinfo>

#include "NativeLoaderType.h"

#include "Scope.h"
#include "NativeFunction.h"
#include "Integer.h"
#include "StringType.h"

#include "NativeModuleHeader.h"

#include "../string.h"
#include "../FileUrl.h"

VectorArray<NativeModule> *loaded_modules = NULL;
NativeLoader              *native_loader  = NULL;

void unload_loaded_modules() {
	for (int i = 0; i < loaded_modules->length; ++i) {
		void *module = loaded_modules->vector[i]->module;
		void *sym = dlsym(module, "onUnload");
		void *error = dlerror();
		if (error) {
			dlclose(module);
			delete loaded_modules->vector[i];
			continue;
		}
		
		try {
			_native_onunload_func onunload_f = (_native_onunload_func) sym;
			
			if (!onunload_f) {
				dlclose(module);
				delete loaded_modules->vector[i];
				continue;
			}
			
			onunload_f();
		} catch(...) {}
		
		dlclose(module);
		delete loaded_modules->vector[i];
	}
	
	delete loaded_modules;
};

// NativeLoader type
NativeLoader::NativeLoader() {
	// There could be only one Mickle Vincent
	if (native_loader != NULL && native_loader != this) {
		delete this;
		return;
	}
	
	loaded_modules = new VectorArray<NativeModule>();
};

long NativeLoader::toInt() {
	return loaded_modules->length + 1;
};

double NativeLoader::toDouble() {
	return loaded_modules->length + 1;
};

string NativeLoader::toString() {
	return string("[NativeLoader]");
};

string NativeLoader::loadModule(Scope *scope, string filename, int argc, VirtualObject **args) {
	char *file = filename.toCString();
	void *module = dlopen(file, RTLD_NOW | RTLD_GLOBAL);
	free(file);
	if (!module)
		return string((char*) dlerror());
	
	void *sym = dlsym(module, "onLoad");
	char *error = dlerror();
	if (error) {
		string errorst(error);
		dlclose(module);
		return errorst;
	}
	
	try {
        _native_onload_func onload_f = (_native_onload_func) sym;
		
		if (!onload_f) {
			dlclose(module);
			return string("Incorrect onLoad prototype");
		}
		
		onload_f(scope, argc, args);
		
		NativeModule *nmodule = new NativeModule();
		nmodule->file = filename;
		nmodule->module = module;
		
		loaded_modules->push(nmodule);
    } catch(...) {
		dlclose(module);
        return string("Can not cast");
    }
	
	return string();
};

void NativeLoader::finalize() {
	delete table;
};


// Functions

// load(filepath[, args])
static VirtualObject* function_load(Scope *scope, int argc, VirtualObject **args) {	
	if (!argc)
		return new String("No file");
	
	char *temp;
	FileUrl url(temp = objectStringValue(args[0]).toCString());
	free(temp);
	
	FileUrl relative_url = FileUrl(scope->context->script_dir_path, &url);
	
	if (relative_url.exists() && relative_url.isFile())
		return new String(native_loader->loadModule(scope, string(relative_url.path), argc - 1, args + sizeof(VirtualObject*)));
	
	return new String(native_loader->loadModule(scope, string(url.path), argc - 1, args + sizeof(VirtualObject*)));
};

// amount()
static VirtualObject* function_amount(Scope *scope, int argc, VirtualObject **args) {	
	return new Integer(loaded_modules->length);
};

// Called on start. Defines null prototype & type
void define_native_loader(Scope *scope) {
	native_loader = new NativeLoader();
	scope->table->put(string("NativeLoader"), native_loader);
	
	native_loader->table->put(string("__typename"), new String("NativeLoader"));
	native_loader->table->put(string("load"),       new NativeFunction(&function_load));
	native_loader->table->put(string("amount"),     new NativeFunction(&function_amount));
};


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

#include "Files.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cstdio>
#include <wchar.h>

#include "StreamApi.h"

#include "../objects/NativeFunction.h"
#include "../objects/Undefined.h"
#include "../objects/Boolean.h"
#include "../objects/Null.h"
#include "../objects/Integer.h"
#include "../objects/Double.h"
#include "../objects/StringType.h"
#include "../objects/Array.h"
#include "../objects/ObjectConverter.h"

LongPrototype *long_prototype = NULL;

// Long type
Long::Long() {		
	type  = LONGTYPE;
	value = 0;
};

Long::Long(long i) {		
	type  = LONGTYPE;
	value = i;
};

VirtualObject *Long::get(Scope *scope, string *name) {
	return long_prototype->table->get(*name);
};

void Long::put(Scope *scope, string *name, VirtualObject *value) {};

void Long::remove(Scope *scope, string *name) {};

bool Long::contains(Scope *scope, string *name) {
	return long_prototype->contains(scope, name);
};

VirtualObject *Long::call(Scope *scope, int argc, VirtualObject **args) {
	return new Long(value);
};

long Long::toInt() {
	return value;
};

double Long::toDouble() {
	return value;
};

string Long::toString() {
	return string::toString((long long) value, 10);
};


// Long prototype	
LongPrototype::LongPrototype() {		
	// table = new TreeObjectMap;
	type  = LONGTYPE_PROTOTYPE;
};

void LongPrototype::finalize(void) {
	table->finalize();
};

VirtualObject *LongPrototype::get(Scope *scope, string *name) {
	return table->get(*name);
};

void LongPrototype::put(Scope *scope, string *name, VirtualObject *value) {
	table->put(*name, value);
};

void LongPrototype::remove(Scope *scope, string *name) {
	table->remove(*name);
};

bool LongPrototype::contains(Scope *scope, string *name) {
	return table->contains(*name);
};

VirtualObject *LongPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	if (argc)
		if (args[0]->type == LONGTYPE)
			return new Long(((Long*) args[0])->value);
		else
			return new Long(objectIntValue(args[0]));
	return new Long;
};

void LongPrototype::mark(void) {
	if (gc_reachable)
		return;
	gc_reachable = 1;
	table->mark();
};

// Operators

// ==
static VirtualObject* long_operator_eq(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) == objectIntValue(args[1]));
};

// !=
static VirtualObject* long_operator_neq(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) != objectIntValue(args[1]));
};

// >
static VirtualObject* long_operator_gt(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) > objectIntValue(args[1]));
};

// <
static VirtualObject* long_operator_lt(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) < objectIntValue(args[1]));
};

// >=
static VirtualObject* long_operator_ge(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) >= objectIntValue(args[1]));
};

// <=
static VirtualObject* long_operator_le(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) <= objectIntValue(args[1]));
};

// +
static VirtualObject* long_operator_sum(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Long(objectIntValue(args[0]) + objectIntValue(args[1]));
};

// -
static VirtualObject* long_operator_sub(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Long(objectIntValue(args[0]) - objectIntValue(args[1]));
};

// *
static VirtualObject* long_operator_mul(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	if (args[0]->type == DOUBLE)
		return new Double(objectIntValue(args[0]) * objectDoubleValue(args[1]));
	
	return new Long(objectIntValue(args[0]) * objectIntValue(args[1]));
};

// /
static VirtualObject* long_operator_div(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	int a_val = objectIntValue(args[0]);
	
	if (args[1]->type == DOUBLE)
		return new Double(a_val / ((Double*) args[1])->value);
	
	int b_val = objectIntValue(args[1]);
	if (b_val)
		return new Long(a_val / b_val);
	return new Undefined;
};

// %
static VirtualObject* long_operator_mod(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	if (args[0]->type != LONGTYPE)
		return new Undefined;
	
	Long       *a = (Long*) args[0];
	VirtualObject *b = args[1];
	
	int b_val = objectIntValue(b);
	if (b_val)
		return new Long(a->value % b_val);
	return new Undefined;
};

// <<
static VirtualObject* long_operator_shl(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Long(objectIntValue(args[0]) << objectIntValue(args[1]));
};

// >>
static VirtualObject* long_operator_shr(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Long(objectIntValue(args[0]) >> objectIntValue(args[1]));
};

// >>>
static VirtualObject* long_operator_ushr(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Long((unsigned int) objectIntValue(args[0]) >> objectIntValue(args[1]));
};

// &
static VirtualObject* long_operator_band(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Long(objectIntValue(args[0]) & objectIntValue(args[1]));
};

// |
static VirtualObject* long_operator_bor(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Long(objectIntValue(args[0]) | objectIntValue(args[1]));
};

// &&
static VirtualObject* long_operator_and(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) && objectIntValue(args[1]));
};

// ||
static VirtualObject* long_operator_or(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) || objectIntValue(args[1]));
};

// ^
static VirtualObject* long_operator_bxor(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Long(objectIntValue(args[0]) ^ objectIntValue(args[1]));
};

// !x
static VirtualObject* long_operator_not(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Long(!objectIntValue(args[0]));
};

// ~x
static VirtualObject* long_operator_bnot(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Long(~objectIntValue(args[0]));
};

// -x
static VirtualObject* long_operator_neg(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Long(-objectIntValue(args[0]));
};

// +x
static VirtualObject* long_operator_pos(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Long(objectIntValue(args[0]));
};

// ++
static VirtualObject* long_operator_inc(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Long(objectIntValue(args[0]) + 1);
};

// --
static VirtualObject* long_operator_dec(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Long(objectIntValue(args[0]) - 1);
};

// Functions
// toString(long[, base])
static VirtualObject* long_function_tostring(Scope *scope, int argc, VirtualObject **args) {
	if (!argc || args[0]->type != LONGTYPE)
		return NULL;
	
	int i = ((Long*) args[0])->value;
	int b = argc == 1 ? 10 : objectIntValue(args[1]);
	
	if (b < 2 || b > 36)
		return new Undefined;
	
	string *s = new string();
	
	*s += string::toString((long long) i, b);
	
	return new String(s);
};


// Called on start. Defines long prototype & type
void define_long(Scope *scope) {
	long_prototype = new LongPrototype();
	Scope *rootScope = scope->getRoot();
	rootScope->define(string("Long"), long_prototype);
	
	long_prototype->table->put(string("__typename"),    new String("Long"));
	long_prototype->table->put(string("__operator=="),  new NativeFunction(&long_operator_eq));
	long_prototype->table->put(string("__operator!="),  new NativeFunction(&long_operator_neq));
	long_prototype->table->put(string("__operator>"),   new NativeFunction(&long_operator_gt));
	long_prototype->table->put(string("__operator<"),   new NativeFunction(&long_operator_lt));
	long_prototype->table->put(string("__operator>="),  new NativeFunction(&long_operator_ge));
	long_prototype->table->put(string("__operator>="),  new NativeFunction(&long_operator_le));
	long_prototype->table->put(string("__operator+"),   new NativeFunction(&long_operator_sum));
	long_prototype->table->put(string("__operator-"),   new NativeFunction(&long_operator_sub));
	long_prototype->table->put(string("__operator*"),   new NativeFunction(&long_operator_mul));
	long_prototype->table->put(string("__operator/"),   new NativeFunction(&long_operator_div));
	long_prototype->table->put(string("__operator%"),   new NativeFunction(&long_operator_mod));
	long_prototype->table->put(string("__operator<<"),  new NativeFunction(&long_operator_div));
	long_prototype->table->put(string("__operator>>"),  new NativeFunction(&long_operator_shr));
	long_prototype->table->put(string("__operator>>>"), new NativeFunction(&long_operator_ushr));
	long_prototype->table->put(string("__operator&"),   new NativeFunction(&long_operator_band));
	long_prototype->table->put(string("__operator|"),   new NativeFunction(&long_operator_bor));
	long_prototype->table->put(string("__operator&&"),  new NativeFunction(&long_operator_and));
	long_prototype->table->put(string("__operator||"),  new NativeFunction(&long_operator_or));
	long_prototype->table->put(string("__operator^"),   new NativeFunction(&long_operator_bxor));
	long_prototype->table->put(string("__operator-x"),  new NativeFunction(&long_operator_neg));
	long_prototype->table->put(string("__operator+x"),  new NativeFunction(&long_operator_pos));
	long_prototype->table->put(string("__operator!x"),  new NativeFunction(&long_operator_not));
	long_prototype->table->put(string("__operator~x"),  new NativeFunction(&long_operator_bnot));
	long_prototype->table->put(string("__operator++"),  new NativeFunction(&long_operator_inc));
	long_prototype->table->put(string("__operator--"),  new NativeFunction(&long_operator_dec));
	
	long_prototype->table->put(string("toString"),      new NativeFunction(&long_function_tostring));
	long_prototype->table->put(string("SIZE"),          new Integer(sizeof(long)));
};


// - - - - - - - - - - - - - - - - - - - - B Y T E _ T Y P E

CharPrototype *char_prototype = NULL;

// Char type
Char::Char() {		
	type  = CHAR_TYPE;
	value = '\0';
};

Char::Char(char i) {		
	type  = CHAR_TYPE;
	value = i;
};

Char::Char(wchar_t i) {		
	type  = CHAR_TYPE;
	value = i;
};

Char::Char(int i) {		
	type  = CHAR_TYPE;
	value = i;
};

Char::Char(long i) {		
	type  = CHAR_TYPE;
	value = i;
};

VirtualObject *Char::get(Scope *scope, string *name) {
	return char_prototype->table->get(*name);
};

void Char::put(Scope *scope, string *name, VirtualObject *value) {};

void Char::remove(Scope *scope, string *name) {};

bool Char::contains(Scope *scope, string *name) {
	return char_prototype->contains(scope, name);
};

VirtualObject *Char::call(Scope *scope, int argc, VirtualObject **args) {
	return new Char(value);
};

long Char::toInt() {
	return value;
};

double Char::toDouble() {
	return value;
};

string Char::toString() {
	return string("") + value;
};


// Char prototype	
CharPrototype::CharPrototype() {		
	type  = CHAR_TYPE_PROTOTYPE;
};

VirtualObject *CharPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	if (argc) {
		string ts = objectStringValue(args[0]);	
		if (ts.length == 1)
			return new Char(ts.buffer[0]);
		return new Char(objectIntValue(args[0]));
	}
	return new Char;
};


// Operators

// ==
static VirtualObject* operator_eq(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) == objectIntValue(args[1]));
};

// !=
static VirtualObject* operator_neq(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) != objectIntValue(args[1]));
};

// >
static VirtualObject* operator_gt(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) > objectIntValue(args[1]));
};

// <
static VirtualObject* operator_lt(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) < objectIntValue(args[1]));
};

// >=
static VirtualObject* operator_ge(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) >= objectIntValue(args[1]));
};

// <=
static VirtualObject* operator_le(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) <= objectIntValue(args[1]));
};

// +
static VirtualObject* operator_sum(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Char(objectIntValue(args[0]) + objectIntValue(args[1]));
};

// -
static VirtualObject* operator_sub(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Char(objectIntValue(args[0]) + objectIntValue(args[1]));
};

// *
static VirtualObject* operator_mul(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	if (args[0]->type == DOUBLE)
		return new Double(objectIntValue(args[0]) * objectDoubleValue(args[1]));
	
	return new Integer(objectIntValue(args[0]) * objectIntValue(args[1]));
};

// /
static VirtualObject* operator_div(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	int a_val = objectIntValue(args[0]);
	
	if (args[1]->type == DOUBLE)
		return new Double(a_val / ((Double*) args[1])->value);
	
	int b_val = objectIntValue(args[1]);
	if (b_val)
		return new Integer(a_val / b_val);
	return new Undefined;
};

// %
static VirtualObject* operator_mod(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	if (args[0]->type != INTEGER)
		return new Undefined;
	
	Char       *a = (Char*) args[0];
	VirtualObject *b = args[1];
	
	int b_val = objectIntValue(b);
	if (b_val)
		return new Integer(a->value % b_val);
	return new Undefined;
};

// <<
static VirtualObject* operator_shl(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]) << objectIntValue(args[1]));
};

// >>
static VirtualObject* operator_shr(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]) >> objectIntValue(args[1]));
};

// >>>
static VirtualObject* operator_ushr(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Integer((unsigned int) objectIntValue(args[0]) >> objectIntValue(args[1]));
};

// &
static VirtualObject* operator_band(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]) & objectIntValue(args[1]));
};

// |
static VirtualObject* operator_bor(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]) | objectIntValue(args[1]));
};

// &&
static VirtualObject* operator_and(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) && objectIntValue(args[1]));
};

// ||
static VirtualObject* operator_or(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(objectIntValue(args[0]) || objectIntValue(args[1]));
};

// ^
static VirtualObject* operator_bxor(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]) ^ objectIntValue(args[1]));
};

// !x
static VirtualObject* operator_not(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Integer(!objectIntValue(args[0]));
};

// ~x
static VirtualObject* operator_bnot(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Integer(~objectIntValue(args[0]));
};

// -x
static VirtualObject* operator_neg(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Integer(-objectIntValue(args[0]));
};

// +x
static VirtualObject* operator_pos(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]));
};

// ++
static VirtualObject* operator_inc(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Char(objectIntValue(args[0]) + 1);
};

// --
static VirtualObject* operator_dec(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Char(objectIntValue(args[0]) - 1);
};


// Functions
// toString(integer[, base])
static VirtualObject* function_tostring(Scope *scope, int argc, VirtualObject **args) {
	if (!argc || args[0]->type != CHAR_TYPE)
		return NULL;
	
	int i = ((Char*) args[0])->value;
	return new String(string("") + (wchar_t) i);
};


// Called on start. Defines char prototype & type
void define_char(Scope *scope) {
	Scope *rootScope = scope->getRoot();
	char_prototype = new CharPrototype();
	
	rootScope->define(string("Char"), char_prototype);
	
	char_prototype->table->put(string("__typename"),    new String("Char"));
	char_prototype->table->put(string("__operator=="),  new NativeFunction(&operator_eq));
	char_prototype->table->put(string("__operator!="),  new NativeFunction(&operator_neq));
	char_prototype->table->put(string("__operator>"),   new NativeFunction(&operator_gt));
	char_prototype->table->put(string("__operator<"),   new NativeFunction(&operator_lt));
	char_prototype->table->put(string("__operator>="),  new NativeFunction(&operator_ge));
	char_prototype->table->put(string("__operator>="),  new NativeFunction(&operator_le));
	char_prototype->table->put(string("__operator+"),   new NativeFunction(&operator_sum));
	char_prototype->table->put(string("__operator-"),   new NativeFunction(&operator_sub));
	char_prototype->table->put(string("__operator*"),   new NativeFunction(&operator_mul));
	char_prototype->table->put(string("__operator/"),   new NativeFunction(&operator_div));
	char_prototype->table->put(string("__operator%"),   new NativeFunction(&operator_mod));
	char_prototype->table->put(string("__operator<<"),  new NativeFunction(&operator_div));
	char_prototype->table->put(string("__operator>>"),  new NativeFunction(&operator_shr));
	char_prototype->table->put(string("__operator>>>"), new NativeFunction(&operator_ushr));
	char_prototype->table->put(string("__operator&"),   new NativeFunction(&operator_band));
	char_prototype->table->put(string("__operator|"),   new NativeFunction(&operator_bor));
	char_prototype->table->put(string("__operator&&"),  new NativeFunction(&operator_and));
	char_prototype->table->put(string("__operator||"),  new NativeFunction(&operator_or));
	char_prototype->table->put(string("__operator^"),   new NativeFunction(&operator_bxor));
	char_prototype->table->put(string("__operator-x"),  new NativeFunction(&operator_neg));
	char_prototype->table->put(string("__operator+x"),  new NativeFunction(&operator_pos));
	char_prototype->table->put(string("__operator!x"),  new NativeFunction(&operator_not));
	char_prototype->table->put(string("__operator~x"),  new NativeFunction(&operator_bnot));
	char_prototype->table->put(string("__operator++"),  new NativeFunction(&operator_inc));
	char_prototype->table->put(string("__operator--"),  new NativeFunction(&operator_dec));
	
	char_prototype->table->put(string("toString"),      new NativeFunction(&function_tostring));
	char_prototype->table->put(string("SIZE"),          new Integer(sizeof(wchar_t)));
};


// - - - - - - - - - - - - - - - - - - - - - - - F I L E

static string normalize(string path) {
	string np;
	
	// Convert / to \\ on windows
	//        \\ to /  on linux
	for (int i = 0; i < path.length; ++i)
#if _FILE_LOC_LINUX == 1
		if (path.buffer[i] == winPathSeparatorChar)
			path.buffer[i] = linuxPathSeparatorChar;
#else
		if (path.buffer[i] == winPathSeparatorChar)
			path.buffer[i] = winPathSeparatorChar;
#endif

	for (int i = 0; i < path.length; ++i) {
		// ..path1///////path2.. -> ..path1/path2..
		// //path/..        - correct
		if (i != 0 && path.buffer[i] == pathSeparatorChar) {
			while (i + 1 < path.length && path.buffer[i + 1] == pathSeparatorChar)
				++i;
			
			np += pathSeparatorChar;
		}
		
		// Remove trailing separator:
		// /folder/my/babanas/ -> /folder/my/bananas
		else if (i == path.length - 1 && path.buffer[i] == pathSeparatorChar)
			continue;
		else
			np += path.buffer[i];
	}
	
	return np;
};

File::File(string path) {
	this->type  = FILETYPE;
	this->scope = NULL;
	this->path  = normalize(path);
	if (file_prototype)
		this->table->putAll(file_prototype->table);
};

File::File(string path, string subpath) {
	this->type  = FILETYPE;
	this->scope = NULL;
	if (path.length)
		this->path = normalize(path + pathSeparatorChar + subpath);
	else
		this->path = normalize(subpath);
	if (file_prototype)
		this->table->putAll(file_prototype->table);
};

File::File(string path, File *subpath) {
	this->type  = FILETYPE;
	this->scope = NULL;
	if (subpath)
		if (path.length)
			this->path = normalize(path + pathSeparatorChar + subpath->path);
		else
			this->path = normalize(subpath->path);
	else
		if (path.length)
			this->path = normalize(path);
		else
			this->path = "";
	if (file_prototype)
		this->table->putAll(file_prototype->table);
};

File::File(File *path, string subpath) {
	this->type  = FILETYPE;
	this->scope = path->scope;
	if (path && path->path.length)
		this->path = normalize(path->path + pathSeparatorChar + subpath);
	else
		this->path = normalize(subpath);
	if (file_prototype)
		this->table->putAll(file_prototype->table);
};

File::File(File *path, File *subpath) {
	this->type  = FILETYPE;
	this->scope = path->scope;
	if (subpath)
		if (path && path->path.length)
			this->path = normalize(path->path + pathSeparatorChar + subpath->path);
		else
			this->path = normalize(subpath->path);
	else
		if (path && path->path.length)
			this->path = normalize(path);
		else
			this->path = "";
	if (file_prototype)
		this->table->putAll(file_prototype->table);
};


void File::attachScope(Scope *scope) {
	this->scope = scope;
};

string File::getAbsolutePath() {
	if (scope == NULL || path.length == 0)
		return path;
	
	// Priority:
	// 1. /<path>
	// 2. <context_path>/<path>
	// 3. <path>
	
	// Check for:
	// 1. <device_name>:/<subpath>
	// 2. /<subpath>
	bool isFullPath = 0;
	
#if defined _FILE_PLATFORM_DEPENDENT && _FILE_LOC_LINUX == 1
	if (path.buffer[0] == pathSeparatorChar)
		isFullPath = 1;
#elif _FILE_PLATFORM_INDEPENDENT
	for (int i = 0; i < path.length; ++i)
		if (i != 0)
			if (path.buffer[i] == ':' && i < path.length - 1 && path.buffer[i + 1] == pathSeparatorChar) {
				isFullPath = 1;
				break;
			} else if (path.buffer[i] == pathSeparatorChar)
				break;
#else
	if (path.buffer[0] == pathSeparatorChar)
		isFullPath = 1;
	else for (int i = 0; i < path.length; ++i)
		if (i != 0)
			if (path.buffer[i] == ':' && i < path.length - 1 && path.buffer[i + 1] == pathSeparatorChar) {
				isFullPath = 1;
				break;
			} else if (path.buffer[i] == pathSeparatorChar)
				break;
#endif
	if (isFullPath)
		return path;
	
	return normalize(string(scope->context->script_dir_path->path) + pathSeparatorChar + path);
};

File *File::getAbsoluteFile() {
	return new File(getAbsolutePath());
};

string File::getPath() {
	return path;
};

string File::getFileName() {
	string fname;
	
	for (int i = path.length - 1; i >= 0; --i)
		if (path.buffer[i] == pathSeparatorChar)
			break;
		else
			fname += path.buffer[i];
		
	fname.reverse();
	return fname;
};

string File::getParentFilePath() {
	int lastSeparatorIndex = -1;
	for (int i = path.length - 1; i >= 0; --i)
		if (path.buffer[i] == pathSeparatorChar) {
			lastSeparatorIndex = i;
			break;
		}
		
	// Cases:
	// <parent>/<file> -> parent
	// <file>          -> getAbsolutePath().getParentFilePath()
	if (lastSeparatorIndex == -1 || lastSeparatorIndex == 0) {
		if (scope == NULL)
			return string("");
		
		string absPath = getAbsolutePath();
		lastSeparatorIndex = -1;
		for (int i = absPath.length - 1; i >= 0; --i)
			if (absPath.buffer[i] == pathSeparatorChar) {
				lastSeparatorIndex = i;
				break;
			}
		
		if (lastSeparatorIndex == 0 || lastSeparatorIndex == 0)
			return string("");
		
		string np;
		for (int i = 0; i < lastSeparatorIndex; ++i)
			np += absPath.buffer[i];
		
		return np;
	}
	
	string np;
	for (int i = 0; i < lastSeparatorIndex; ++i)
		np += path.buffer[i];
	
	return np;
};

File *File::getParentFile() {
	return new File(getParentFilePath());
};

bool File::exists() {
	string absPath = getAbsolutePath();
	char *cpath = absPath.toCString();
	
	struct stat buffer;   
	int result = stat(cpath, &buffer) == 0;
	
	free(cpath);
	return result;
};

bool File::isDirectory() {
	string absPath = getAbsolutePath();
	char *cpath = absPath.toCString();
	
	struct stat sb;
	int result = (stat(cpath, &sb) == 0 && S_ISDIR(sb.st_mode));
	
	free(cpath);
	return result;
};

VectorArray<VirtualObject> *File::listFiles() {
	string absPath = getAbsolutePath();
	char *cpath = absPath.toCString();
	
	DIR *dir;
	int dir_cnt = 0;
	struct dirent *ent;
	if ((dir = opendir(cpath)) != NULL) {
		VectorArray<VirtualObject> *listFiles = new VectorArray<VirtualObject>();
	
		while ((ent = readdir (dir)) != NULL) 
			listFiles->push(new File(string(ent->d_name)));

		closedir(dir);
		free(cpath);
		return listFiles;
	} else {
		free(cpath);
		return NULL;
	}
};

int File::createNewFile() {
	string absPath = getAbsolutePath();
	char *cpath = absPath.toCString();
	
	FILE *fptr = fopen(cpath, "rb+");
	if (fptr == NULL) {
		fptr = fopen(cpath, "wb");
		if (fptr == NULL) {
			free(cpath);
			return 0;
		}
		fclose(fptr);
	} else
		fclose(fptr);
	
	free(cpath);
	return 1;
};

static int f_wrap_remove_file(char *filename) {
	return remove(filename);
};

int File::deleteFile() {
	string absPath = getAbsolutePath();
	char *cpath = absPath.toCString();
	
	int result = f_wrap_remove_file(cpath);
	
	free(cpath);
	return result == 0;
};

int File::createDirectory() {
	string absPath = getAbsolutePath();
	char *cpath = absPath.toCString();
	
	int result = mkdir(cpath, 0777);
	
	free(cpath);
	return !result;
};

long File::getSize() {
	char *cpath = path.toCString();
	struct stat st;
	stat(cpath, &st);
	long size = st.st_size;
	free(cpath);
	
	return size;
};

void File::mark() {
	if (gc_reachable)
		return;
	
	if (scope)
		scope->mark();
	
	Object::mark();
};

long File::toInt() {
	return path.length + 1;
};

string File::toString() {
	return path;
};

double File::toDouble() {
	return path.length + 1;
};


FilePrototype::FilePrototype() {
	this->type = FILE_PROTOTYPE;
};

VirtualObject *FilePrototype::call(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	File *f;
	if (argc == 1) {
		if (args[0]->type == FILETYPE)
			f = new File(((File*) args[0])->path);
		else 
			f = new File(objectStringValue(args[0]));
	} else {
		if (args[0]->type == FILETYPE)
			if (args[1]->type == FILETYPE)
				f = new File(((File*) args[0])->path, ((File*) args[1])->path);
			else
				f = new File(((File*) args[0])->path, (objectStringValue(args[1])));
		else
			if (args[1]->type == FILETYPE)
				f = new File(objectStringValue(args[0]), ((File*) args[1])->path);
			else
				f = new File(objectStringValue(args[0]), (objectStringValue(args[1])));
	}
	
	f->attachScope(scope);
	return f;
};


FilePrototype *file_prototype = NULL;


// - - - - - - - - - - - - - - - - - - - - - - - F I L E _ I N P U T _ S T R E A M
	
FileInputStream::FileInputStream(File *f) {
	this->type = FILEINPUTSTREAM;
	this->file = f;
	this->cfile = NULL;
	this->cpath = f->path.toCString();
	
	if (file_input_stream_prototype) {
		this->table->putAll(file_input_stream_prototype->table);
		string key = "read";
		this->setReadFunc(file_input_stream_prototype->table->get(&key), file_input_stream_prototype->def_scope);
		key = "eof";
		this->setEofFunc(file_input_stream_prototype->table->get(&key), file_input_stream_prototype->def_scope);
		key = "close";
		this->setCloseFunc(file_input_stream_prototype->table->get(&key), file_input_stream_prototype->def_scope);
	}
};

FileInputStream::FileInputStream(string path) {
	this->type = FILEINPUTSTREAM;
	this->file = new File(path);
	this->cfile = NULL;
	this->cpath = path.toCString();
	
	if (file_input_stream_prototype) {
		this->table->putAll(file_input_stream_prototype->table);
		string key = "read";
		this->setReadFunc(file_input_stream_prototype->table->get(&key), file_input_stream_prototype->def_scope);
		key = "eof";
		this->setEofFunc(file_input_stream_prototype->table->get(&key), file_input_stream_prototype->def_scope);
		key = "close";
		this->setCloseFunc(file_input_stream_prototype->table->get(&key), file_input_stream_prototype->def_scope);
	}
};

void FileInputStream::attachScope(Scope *scope) {
	if (this->file)
		this->file->attachScope(scope);
};

int FileInputStream::open() {
	if (cfile)
		return 0;
	
	cfile = fopen(cpath, "r");
	
	//struct stat st;
	//stat(cpath, &st);
	//size = st.st_size;
	
	if (!cfile)
		return 0;
	
	//fseek(cfile, 0L, SEEK_END);
	//size = ftell(cfile);
	//fseek(cfile, 0L, SEEK_SET);
	
	return 1;
};

void FileInputStream::mark() {
	if (gc_reachable)
		return;
	
	if (file)
		file->mark();
	
	Object::mark();
};

void FileInputStream::finalize() {
	if (cfile)
		fclose(cfile);
	if (cpath)
		free(cpath);
	
	Object::finalize();
};


FileInputStreamPrototype::FileInputStreamPrototype() {
	this->type = FILEINPUTSTREAM_PROTOTYPE;
	this->def_scope = NULL;
};

VirtualObject *FileInputStreamPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	FileInputStream *fis;
	if (args[0]->type == FILETYPE) {
		File *f = ((File*) args[0]);
		f->attachScope(scope);
		f = f->getAbsoluteFile();
		fis = new FileInputStream(f);
	} else {
		File *f = new File(objectStringValue(args[0]));
		f->attachScope(scope);
		f->path = f->getAbsolutePath();
		fis = new FileInputStream(f);
	}
	fis->attachScope(scope);
	
	int result = fis->open();
	
	if (!result) 
		scope->context->executer->raiseError("Cant't open file");
	
	return fis;
};
	
	
FileInputStreamPrototype *file_input_stream_prototype = NULL;


// - - - - - - - - - - - - - - - - - - - - - - - F I L E _ O U T P U T _ S T R E A M
	
FileOutputStream::FileOutputStream(File *f) {
	this->type = FILEOUTPUTSTREAM;
	this->file = f;
	this->cfile = NULL;
	this->cpath = f->path.toCString();
	
	if (file_output_stream_prototype) {
		this->table->putAll(file_output_stream_prototype->table);
		string key = "write";
		this->setWriteFunc(file_output_stream_prototype->table->get(&key), file_output_stream_prototype->def_scope);
		key = "flush";
		this->setFlushFunc(file_output_stream_prototype->table->get(&key), file_output_stream_prototype->def_scope);
		key = "close";
		this->setCloseFunc(file_output_stream_prototype->table->get(&key), file_output_stream_prototype->def_scope);
	}
};

FileOutputStream::FileOutputStream(string path) {
	this->type = FILEOUTPUTSTREAM;
	this->file = new File(path);
	this->cfile = NULL;
	this->cpath = path.toCString();
	
	if (file_output_stream_prototype) {
		this->table->putAll(file_output_stream_prototype->table);
		string key = "write";
		this->setWriteFunc(file_output_stream_prototype->table->get(&key), file_output_stream_prototype->def_scope);
		key = "flush";
		this->setFlushFunc(file_output_stream_prototype->table->get(&key), file_output_stream_prototype->def_scope);
		key = "close";
		this->setCloseFunc(file_output_stream_prototype->table->get(&key), file_output_stream_prototype->def_scope);
	}
};

void FileOutputStream::attachScope(Scope *scope) {
	if (this->file)
		this->file->attachScope(scope);
};

int FileOutputStream::open() {
	if (cfile)
		return 0;
	
	cfile = fopen(cpath, "w");
	
	if (!cfile)
		return 0;
};

void FileOutputStream::mark() {
	if (gc_reachable)
		return;
	
	if (file)
		file->mark();
	
	Object::mark();
};

void FileOutputStream::finalize() {
	if (cfile)
		fclose(cfile);
	if (cpath)
		free(cpath);
	
	Object::finalize();
};


FileOutputStreamPrototype::FileOutputStreamPrototype() {
	this->type = FILEOUTPUTSTREAM_PROTOTYPE;
	this->def_scope = NULL;
};

VirtualObject *FileOutputStreamPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	FileOutputStream *fos;
	if (args[0]->type == FILETYPE) {
		File *f = ((File*) args[0]);
		f->attachScope(scope);
		f = f->getAbsoluteFile();
		fos = new FileOutputStream(f);
	} else {
		File *f = new File(objectStringValue(args[0]));
		f->attachScope(scope);
		f->path = f->getAbsolutePath();
		fos = new FileOutputStream(f);
	}
	fos->attachScope(scope);
	
	int result = fos->open();
	
	if (!result) 
		scope->context->executer->raiseError("Cant't open file");
	
	return fos;
};


FileOutputStreamPrototype *file_output_stream_prototype = NULL;


// - - - - - - - - - - - - - - - - - - - - - - - S C A N N E R

Scanner::Scanner(File *f) : FileInputStream(f) {
	this->type = SCANNER;
	if (scanner_prototype)
		this->table->putAll(scanner_prototype->table);
};

Scanner::Scanner(string path) : FileInputStream(path) {
	this->type = SCANNER;
	if (scanner_prototype)
		this->table->putAll(scanner_prototype->table);
};
 

ScannerPrototype::ScannerPrototype() {
	this->type = SCANNER_PROTOTYPE;
	this->def_scope = NULL;
};

VirtualObject *ScannerPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	Scanner *fis;
	if (args[0]->type == FILETYPE) {
		File *f = ((File*) args[0]);
		f->attachScope(scope);
		f = f->getAbsoluteFile();
		fis = new Scanner(f);
	} else {
		File *f = new File(objectStringValue(args[0]));
		f->attachScope(scope);
		f->path = f->getAbsolutePath();
		fis = new Scanner(f);
	}
	fis->attachScope(scope);
	
	int result = fis->open();
	
	if (!result) 
		scope->context->executer->raiseError("Cant't open file");
	
	return fis;
};


ScannerPrototype *scanner_prototype = NULL;


// - - - - - - - - - - - - - - - - - - - - - - - P R I N T _ W R I T E R

Printer::Printer(File *f) : FileOutputStream(f) {
	this->type = PRINTER;
	if (printer_prototype)
		this->table->putAll(printer_prototype->table);
};

Printer::Printer(string path) : FileOutputStream(path) {
	this->type = PRINTER;
	if (printer_prototype)
		this->table->putAll(printer_prototype->table);
};


PrinterPrototype::PrinterPrototype() {
	this->type = PRINTER_PROTOTYPE;
	this->def_scope = NULL;
};

VirtualObject *PrinterPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	Printer *fos;
	if (args[0]->type == FILETYPE) {
		File *f = ((File*) args[0]);
		f->attachScope(scope);
		f = f->getAbsoluteFile();
		fos = new Printer(f);
	} else {
		File *f = new File(objectStringValue(args[0]));
		f->attachScope(scope);
		f->path = f->getAbsolutePath();
		fos = new Printer(f);
	}
	fos->attachScope(scope);
	
	int result = fos->open();
	
	if (!result) 
		scope->context->executer->raiseError("Cant't open file");
	
	return fos;
};


PrinterPrototype *printer_prototype = NULL;


// - - - - - - - - - - - - - - - - - - - - - - - F U N C T I O N S

static VirtualObject* fp_operator_eq(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(args[0]->type == FILETYPE && args[0]->type == FILETYPE && ((File*) args[0])->path == ((File*) args[1])->path);
};

static VirtualObject* fp_operator_neq(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Boolean(!(args[0]->type == FILETYPE && args[0]->type == FILETYPE && ((File*) args[0])->path == ((File*) args[1])->path));
};

static VirtualObject* fp_getAbsolutePath(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object) && o->type == FILETYPE))
		return NULL;
	
	File *f = (File*) o;
	return new String(f->getAbsolutePath());
};

static VirtualObject* fp_getAbsoluteFile(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object) && o->type == FILETYPE))
		return NULL;
	
	File *f = (File*) o;
	return f->getAbsoluteFile();
};

static VirtualObject* fp_getParentFilePath(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object) && o->type == FILETYPE))
		return NULL;
	
	File *f = (File*) o;
	return new String(f->getParentFilePath());
};

static VirtualObject* fp_getParentFile(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object) && o->type == FILETYPE))
		return NULL;
	
	File *f = (File*) o;
	return f->getParentFile();
};

static VirtualObject* fp_exists(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object) && o->type == FILETYPE))
		return NULL;
	
	File *f = (File*) o;
	return new Boolean(f->exists());
};

static VirtualObject* fp_isDirectory(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object) && o->type == FILETYPE))
		return NULL;
	
	File *f = (File*) o;
	return new Boolean(f->isDirectory());
};

static VirtualObject* fp_listFiles(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object) && o->type == FILETYPE))
		return NULL;
	
	File *f = (File*) o;
	VectorArray<VirtualObject> *listFiles = f->listFiles();
	if (listFiles == NULL)
		return new Null();
	
	return new Array(listFiles);
};

static VirtualObject* fp_createNewFile(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object) && o->type == FILETYPE))
		return NULL;
	
	File *f = (File*) o;
	return new Boolean(f->createNewFile());
};

static VirtualObject* fp_deleteFile(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object) && o->type == FILETYPE))
		return NULL;
	
	File *f = (File*) o;
	return new Boolean(f->deleteFile());
};

static VirtualObject* fp_createDirectory(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object) && o->type == FILETYPE))
		return NULL;
	
	File *f = (File*) o;
	return new Boolean(f->createDirectory());
};

static VirtualObject* fp_getFileName(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object) && o->type == FILETYPE))
		return NULL;
	
	File *f = (File*) o;
	return new String(f->getFileName());
};

static VirtualObject* fp_getSize(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object) && o->type == FILETYPE))
		return NULL;
	
	File *f = (File*) o;
	return new Long(f->getSize());
};


static VirtualObject* fis_read(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	FileInputStream *fis;
	try {
		fis = dynamic_cast<FileInputStream*>(o);
	} catch(...) { return NULL; }
	
	
	if (fis->cfile == NULL) {
		scope->context->executer->raiseError("InputStream is closed");
		return new Integer(-1);
	}
	
	// if (ftell(fis->cfile) >= fis->size)
	// 	return new Integer(-1);
	
	if (argc == 0) 
		return new Integer(fgetc(fis->cfile));
	else if (argc == 2) {
		// read(buffer, amount);
		if (args[0]->type != BYTEPTR) {
			scope->context->executer->raiseError("read expected byte buffer");
			return new Integer(-1);
		}
		
		BytePtr *buff = (BytePtr*) args[0];
		int amount = objectIntValue(args[1]);
		if (amount == 0)
			return new Integer(0);
		
		if (amount > buff->size) {
			scope->context->executer->raiseError("read of block larger than allocated");
			return new Integer(-1);
		}
		
		int read = fread(buff->ptr, sizeof(char), amount, fis->cfile);
		
		if (ferror(fis->cfile)) {
			scope->context->executer->raiseError("read error occurred");
			return new Integer(-1);
		}
		
		return read;
	} else if (argc == 3) {
		// read(buffer, offset, amount);
		if (args[0]->type != BYTEPTR) {
			scope->context->executer->raiseError("read expected byte buffer");
			return new Integer(-1);
		}
		
		BytePtr *buff = (BytePtr*) args[0];
		int offset = objectIntValue(args[1]);
		int amount = objectIntValue(args[2]);
		if (amount == 0)
			return new Integer(0);
		
		if (amount > buff->size - offset) {
			scope->context->executer->raiseError("read of block larger than allocated");
			return new Integer(-1);
		}
		
		int read = fread(buff->ptr + offset, sizeof(char), amount, fis->cfile);
		
		if (ferror(fis->cfile)) {
			scope->context->executer->raiseError("read error occurred");
			return new Integer(-1);
		}
		
		return read;
	}
};

static VirtualObject* fis_close(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	FileInputStream *fis;
	try {
		fis = dynamic_cast<FileInputStream*>(o);
	} catch(...) { return NULL; }
	
	
	if (fis->cfile == NULL) 
		return new Boolean(0);
	
	fclose(fis->cfile);
	fis->cfile = NULL;
	
	return new Boolean(1);
};

static VirtualObject* fis_eof(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	FileInputStream *fis;
	try {
		fis = dynamic_cast<FileInputStream*>(o);
	} catch(...) { return NULL; }
	
	if (fis->cfile == NULL)
		return new Boolean(0);
	
	
	struct stat st;
	fstat(fileno(fis->cfile), &st);
	long size = st.st_size;
	
	int pos = ftell(fis->cfile);
	
	return new Boolean(pos >= size);
};

static VirtualObject* fis_getFile(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	FileInputStream *fis;
	try {
		fis = dynamic_cast<FileInputStream*>(o);
	} catch(...) { return NULL; }
	
	
	return fis->file;
};

static VirtualObject* fis_reopen(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	FileInputStream *fis;
	try {
		fis = dynamic_cast<FileInputStream*>(o);
	} catch(...) { return NULL; }
	
	
	if (fis->cfile) 
		fclose(fis->cfile);
	
	fis->cfile = fopen(fis->cpath, "r");
	
	if (!fis->cfile) {
		scope->context->executer->raiseError("Can't open file");
		return NULL;
	}
	
	return fis;
};

static VirtualObject* fis_available(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	FileInputStream *fis;
	try {
		fis = dynamic_cast<FileInputStream*>(o);
	} catch(...) { return NULL; }
	
	if (!fis->cfile)
		return new Integer(-1);
	
	struct stat st;
	fstat(fileno(fis->cfile), &st);
	long size = st.st_size;
	
	int pos = ftell(fis->cfile);
	
	return new Long(size - pos);
};

static VirtualObject* fis_fileSize(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	FileInputStream *fis;
	try {
		fis = dynamic_cast<FileInputStream*>(o);
	} catch(...) { return NULL; }
	
	if (!fis->cfile)
		return new Integer(-1);
	
	struct stat st;
	fstat(fileno(fis->cfile), &st);
	long size = st.st_size;
	
	return new Long(size);
};


static VirtualObject* fos_write(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	FileOutputStream *fos;
	try {
		fos = dynamic_cast<FileOutputStream*>(o);
	} catch(...) { return NULL; }
	
	
	if (fos->cfile == NULL) {
		scope->context->executer->raiseError("OutputStream is closed");
		return new Integer(-1);
	}
	
	if (argc == 1) {
		char b = objectIntValue(args[0]);
		fwrite(&b, sizeof(char), sizeof(&b), fos->cfile);
		
		if (ferror(fos->cfile)) {
			scope->context->executer->raiseError("write error occurred");
			return new Integer(-1);
		}
		return new Integer(1);
	} else if (argc == 2) {
		// write(buffer, amount);
		if (args[0]->type != BYTEPTR) {
			scope->context->executer->raiseError("write expected byte buffer");
			return new Integer(-1);
		}
		
		BytePtr *buff = (BytePtr*) args[0];
		int amount = objectIntValue(args[1]);
		if (amount == 0)
			return new Integer(0);
		
		if (amount > buff->size) {
			scope->context->executer->raiseError("write of block larger than allocated");
			return new Integer(-1);
		}
		
		int write = fwrite(buff->ptr, sizeof(char), amount, fos->cfile);
		
		if (ferror(fos->cfile)) {
			scope->context->executer->raiseError("write error occurred");
			return new Integer(-1);
		}
		
		return write;
	} else if (argc == 3) {
		// read(buffer, offset, amount);
		if (args[0]->type != BYTEPTR) {
			scope->context->executer->raiseError("write expected byte buffer");
			return new Integer(-1);
		}
		
		BytePtr *buff = (BytePtr*) args[0];
		int offset = objectIntValue(args[1]);
		int amount = objectIntValue(args[2]);
		if (amount == 0)
			return new Integer(0);
		
		if (amount > buff->size - offset) {
			scope->context->executer->raiseError("write of block larger than allocated");
			return new Integer(-1);
		}
		
		int write = fwrite(buff->ptr + offset, sizeof(char), amount, fos->cfile);
		
		if (ferror(fos->cfile)) {
			scope->context->executer->raiseError("write error occurred");
			return new Integer(-1);
		}
		
		return write;
	}
};

static VirtualObject* fos_close(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	FileOutputStream *fos;
	try {
		fos = dynamic_cast<FileOutputStream*>(o);
	} catch(...) { return NULL; }
	
	
	if (fos->cfile == NULL) 
		return new Boolean(0);
	
	fclose(fos->cfile);
	fos->cfile = NULL;
	
	return new Boolean(1);
};

static VirtualObject* fos_getFile(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	FileOutputStream *fos;
	try {
		fos = dynamic_cast<FileOutputStream*>(o);
	} catch(...) { return NULL; }
	
	
	return fos->file;
};

static VirtualObject* fos_reopen(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	FileOutputStream *fos;
	try {
		fos = dynamic_cast<FileOutputStream*>(o);
	} catch(...) { return NULL; }
	
	
	if (fos->cfile) 
		fclose(fos->cfile);
		
	fos->cfile = fopen(fos->cpath, "w");
	
	if (!fos->cfile) {
		scope->context->executer->raiseError("Can't open file");
		return NULL;
	}
	
	return fos;
};


// #define S_READLINE_CHAR_BY_CHAR

static VirtualObject* s_readLine(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	Scanner *fis;
	try {
		fis = dynamic_cast<Scanner*>(o);
	} catch(...) { return NULL; }
	
	
	if (fis->cfile == NULL) {
		scope->context->executer->raiseError("InputStream is closed");
		return new Undefined();
	}
	
	string s;
#ifdef S_READLINE_CHAR_BY_CHAR
	wint_t c; 
	while ((c = fgetwc(fis->cfile)) != WEOF && c != '\n')
		s += c;
	
#else
	wchar_t buf[1024];

	while (fgetws(buf, 1024, fis->cfile) != NULL) {
		bool nline = 0;
		
		// Check if newline reached
		for (int i = 0; i < 1024; ++i)
			if (buf[i] == '\n') {
				buf[i] = '\0';
				nline = 1;
				break;
			}
		
		s += buf;
		if (nline)
			break;
	}
		
	if (ferror(fis->cfile))
		scope->context->executer->raiseError("read error occurred");
#endif
	
	return new String(s);
};

static VirtualObject* s_readInt(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	Scanner *fis;
	try {
		fis = dynamic_cast<Scanner*>(o);
	} catch(...) { return NULL; }
	
	
	if (fis->cfile == NULL) {
		scope->context->executer->raiseError("InputStream is closed");
		return new Undefined();
	}
	
	int i;
	int scan = fscanf(fis->cfile, "%d", &i);
	if (scan == -1) 
		return new Undefined();
	return new Integer(i);
};

static VirtualObject* s_readDouble(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	Scanner *fis;
	try {
		fis = dynamic_cast<Scanner*>(o);
	} catch(...) { return NULL; }
	
	
	if (fis->cfile == NULL) {
		scope->context->executer->raiseError("InputStream is closed");
		return new Undefined();
	}
	
	double d;
	int scan = fscanf(fis->cfile, "%lf", &d);
	if (scan == -1) 
		return new Undefined();
	return new Double(d);
};

static VirtualObject* s_readChar(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	Scanner *fis;
	try {
		fis = dynamic_cast<Scanner*>(o);
	} catch(...) { return NULL; }
	
	
	if (fis->cfile == NULL) {
		scope->context->executer->raiseError("InputStream is closed");
		return new Undefined();
	}
	
	wint_t read = fgetc(fis->cfile);
	if (read == -1) 
		return new Undefined();
	
	return new Char((wchar_t) read);
};


static VirtualObject* p_print(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	Printer *fos;
	try {
		fos = dynamic_cast<Printer*>(o);
	} catch(...) { return NULL; }
	
	
	if (fos->cfile == NULL) {
		scope->context->executer->raiseError("OutputStream is closed");
		return new Undefined();
	}
	
	int result = 0;
	for (int i = 0; i < argc; ++i) {
		string s = objectStringValue(args[i]);
		result += fprintf(fos->cfile, "%S", s.toCharSequence());
	}
	
	return new Integer(result);
};

static VirtualObject* p_println(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)))
		return NULL;
	
	Printer *fos;
	try {
		fos = dynamic_cast<Printer*>(o);
	} catch(...) { return NULL; }
	
	
	if (fos->cfile == NULL) {
		scope->context->executer->raiseError("OutputStream is closed");
		return new Undefined();
	}
	
	int result = 0;
	for (int i = 0; i < argc; ++i) {
		string s = objectStringValue(args[i]);
		result += fprintf(fos->cfile, "%S", s.toCharSequence());
	}
	
	fprintf(fos->cfile, "\n");
	
	return new Integer(result);
};


void define_fios(Scope *scope) {
	Scope *rootScope = scope->getRoot();
	file_prototype = new FilePrototype();
	rootScope->table->put(string("File"), file_prototype);
	
	file_prototype->table->put(string("__typename"),        new String("File"));
	file_prototype->table->put(string("__operator=="),      new NativeFunction(&fp_operator_eq));
	file_prototype->table->put(string("__operator!="),      new NativeFunction(&fp_operator_neq));
	file_prototype->table->put(string("getAbsolutePath"),   new NativeFunction(&fp_getAbsolutePath));
	file_prototype->table->put(string("getAbsoluteFile"),   new NativeFunction(&fp_getAbsoluteFile));
	file_prototype->table->put(string("getParentFilePath"), new NativeFunction(&fp_getParentFilePath));
	file_prototype->table->put(string("getParentFile"),     new NativeFunction(&fp_getParentFile));
	file_prototype->table->put(string("exists"),            new NativeFunction(&fp_exists));
	file_prototype->table->put(string("isDirectory"),       new NativeFunction(&fp_isDirectory));
	file_prototype->table->put(string("listFiles"),         new NativeFunction(&fp_listFiles));
	file_prototype->table->put(string("createNewFile"),     new NativeFunction(&fp_createNewFile));
	file_prototype->table->put(string("deleteFile"),        new NativeFunction(&fp_deleteFile));
	file_prototype->table->put(string("createDirectory"),   new NativeFunction(&fp_createDirectory));
	file_prototype->table->put(string("getFileName"),       new NativeFunction(&fp_getFileName));
	file_prototype->table->put(string("getSize"),           new NativeFunction(&fp_getSize));
	
	
	file_input_stream_prototype = new FileInputStreamPrototype();
	file_input_stream_prototype->def_scope = rootScope;
	rootScope->table->put(string("FileInputStream"), file_input_stream_prototype);
	
	file_input_stream_prototype->table->put(string("__typename"), new String("FileInputStream"));
	file_input_stream_prototype->table->put(string("read"),       new NativeFunction(&fis_read));
	file_input_stream_prototype->table->put(string("close"),      new NativeFunction(&fis_close));
	file_input_stream_prototype->table->put(string("eof"),        new NativeFunction(&fis_eof));
	file_input_stream_prototype->table->put(string("getFile"),    new NativeFunction(&fis_getFile));
	file_input_stream_prototype->table->put(string("reopen"),     new NativeFunction(&fis_reopen));
	file_input_stream_prototype->table->put(string("available"),  new NativeFunction(&fis_available));
	file_input_stream_prototype->table->put(string("fileSize"),   new NativeFunction(&fis_fileSize));
	
	
	file_output_stream_prototype = new FileOutputStreamPrototype();
	file_output_stream_prototype->def_scope = rootScope;
	rootScope->table->put(string("FileOutputStream"), file_output_stream_prototype);
	
	file_output_stream_prototype->table->put(string("__typename"), new String("FileOutputStream"));
	file_output_stream_prototype->table->put(string("read"),       new NativeFunction(&fos_write));
	file_output_stream_prototype->table->put(string("close"),      new NativeFunction(&fos_close));
	file_output_stream_prototype->table->put(string("getFile"),    new NativeFunction(&fos_getFile));
	file_output_stream_prototype->table->put(string("reopen"),     new NativeFunction(&fos_reopen));
	
	
	scanner_prototype = new ScannerPrototype();
	scanner_prototype->def_scope = rootScope;
	rootScope->table->put(string("Scanner"), scanner_prototype);
	
	scanner_prototype->table->putAll(file_input_stream_prototype->table);
	scanner_prototype->table->put(string("__typename"), new String("Scanner"));
	scanner_prototype->table->put(string("readLine"),   new NativeFunction(&s_readLine));
	scanner_prototype->table->put(string("readInt"),    new NativeFunction(&s_readInt));
	scanner_prototype->table->put(string("readDouble"), new NativeFunction(&s_readDouble));
	scanner_prototype->table->put(string("readChar"),   new NativeFunction(&s_readChar));
	
	
	printer_prototype = new PrinterPrototype();
	printer_prototype->def_scope = rootScope;
	rootScope->table->put(string("Printer"), printer_prototype);
	
	printer_prototype->table->putAll(file_output_stream_prototype->table);
	printer_prototype->table->put(string("__typename"), new String("Printer"));
	printer_prototype->table->put(string("print"),      new NativeFunction(&p_print));
	printer_prototype->table->put(string("println"),    new NativeFunction(&p_println));
};

#ifdef __cplusplus
extern "C" {
#endif

// Called on module being loaded by bla bla bla
void onLoad(Scope *scope, int argc, VirtualObject **args) {
	define_long(scope);
	define_char(scope);
	define_fios(scope);
};

// Called on module being unloaded on program exit
void onUnload() {};


#ifdef __cplusplus
}
#endif

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
 * Module for working with Streams in ck.
 */

#include <cstdio>
#include <cstdlib>
#include <clocale>

#include "StreamApi.h"

#include "../objects/Scope.h"
#include "../objects/StringType.h"
#include "../objects/Integer.h"
#include "../objects/Double.h"
#include "../objects/Undefined.h"
#include "../objects/Boolean.h"
#include "../objects/Array.h"
#include "../objects/Object.h"
#include "../objects/NativeFunction.h"
#include "../objects/CodeFunction.h"

#include "../ptr_wrapper.h"


// bash buildso.sh src/modules/StreamApi.cpp bin/StreamApi.so

// - - - - - - - - - - - - - - - - - - - - I N P U T / O U T P U T _ S T R E A M

// Override virtual methods.
int InputStream::read() { return -1; };
int InputStream::read(char *buff, int off, int len) { return -1; };
int InputStream::read(char *buff, int len) { return -1; };
int InputStream::eof() { return 1; };
int InputStream::close() { return -1; };

int OutputStream::write(char b) { return -1; };
int OutputStream::write(char *buff, int off, int len) { return -1; };
int OutputStream::write(char *buff, int len) { return -1; };
int OutputStream::close() { return -1; };
int OutputStream::flush() { return -1; };


// - - - - - - - - - - - - - - - - - - - - O B J E C T _ I N P U T / O U T P U T _ S T R E A M

ObjectInputStreamPrototype  *OIstream_prototype = NULL;
ObjectOutputStreamPrototype *OOstream_prototype = NULL;

// ObjectInputStream type
ObjectInputStream::ObjectInputStream() {
	type            = OBJECTINPUTSTREAM;
	assignedTraceId = OISF_NONE;
	readScope       = NULL;
	eofScope        = NULL;
	closeScope      = NULL;
	readFunc        = NULL;
	eofFunc         = NULL;
	closeFunc       = NULL;
	
	table->putAll(OIstream_prototype->table);
};

void ObjectInputStream::setReadFunc(Scope *scope, VirtualObject *f) {
	this->readScope = scope;
	if (assignedTraceId == OISF_READ)
		assignedTraceId = OISF_NONE;
	this->readFunc = f;
	string key = "read";
	Object::put(NULL, &key, f);
};

void ObjectInputStream::setEofFunc(Scope *scope, VirtualObject *f) {
	this->eofScope = scope;
	if (assignedTraceId == OISF_EOF)
		assignedTraceId = OISF_NONE;
	this->eofFunc = f;
	string key = "eof";
	Object::put(NULL, &key, f);
};

void ObjectInputStream::setCloseFunc(Scope *scope, VirtualObject *f) {
	this->closeScope = scope;
	if (assignedTraceId == OISF_CLOSE)
		assignedTraceId = OISF_NONE;
	this->closeFunc = f;
	string key = "close";
	Object::put(NULL, &key, f);
};

void ObjectInputStream::put(Scope *scope, string *name, VirtualObject *value) {
	if (!name)
		return;
	
	if (*name == "read")
		setReadFunc(scope, value);
	else if (*name == "eof")
		setEofFunc(scope, value);
	else if (*name == "close")
		setCloseFunc(scope, value);
	
	Object::put(scope, name, value);
};

void ObjectInputStream::remove(Scope *scope, string *name) {
	if (!name)
		return;
	
	if (*name == "read")
		setReadFunc(NULL, NULL);
	else if (*name == "eof")
		setEofFunc(NULL, NULL);
	else if (*name == "close")
		setCloseFunc(NULL, NULL);
	
	Object::remove(scope, name);
};

VirtualObject *ObjectInputStream::call(Scope *scope, int argc, VirtualObject **args) {
	return NULL;
};

void ObjectInputStream::mark() {
	if (gc_reachable)
		return;
	
	if (readScope)
		readScope->mark();
	if (eofScope)
		eofScope->mark();
	if (closeScope)
		closeScope->mark();
	
	if (readFunc)
		readFunc->mark();
	if (eofFunc)
		eofFunc->mark();
	if (closeFunc)
		closeFunc->mark();
	
	Object::mark();
};

int ObjectInputStream::read() {
	if (!readFunc)
		return -1;

	if (assignedTraceId != OISF_READ) {
		assignedTraceId = OISF_READ;
		localExecuter.aststacktrace->clear();
		if (readFunc && readFunc->type == CODE_FUNCTION)
			localExecuter.insertStackTrace(((CodeFunction*) readFunc)->scope->context->executer);
		else 
			localExecuter.aststacktrace->insert(string("virtual"));
	}
	
	ExecuterResult result = localExecuter.beginFunction(readScope, this, readFunc, 0);
	
	if (result.type == ExecuterResultType::ERT_OBJECT)
		return objectIntValue(result.object);
	
	return -1;
};

int ObjectInputStream::read(char *buff, int off, int len) {
	if (!readFunc)
		return -1;

	if (assignedTraceId != OISF_READ) {
		assignedTraceId = OISF_READ;
		localExecuter.aststacktrace->clear();
		if (readFunc && readFunc->type == CODE_FUNCTION)
			localExecuter.insertStackTrace(((CodeFunction*) readFunc)->scope->context->executer);
		else 
			localExecuter.aststacktrace->insert(string("virtual"));
	}
	
	BytePtr *ptr = new BytePtr(buff, len, SAPTRALLOCTYPE::SAPTR_UNDEF);
	ExecuterResult result = localExecuter.beginFunction(readScope, this, readFunc, 3, ptr, new Integer(off), new Integer(len));	
	
	if (result.type == ExecuterResultType::ERT_OBJECT)
		return objectIntValue(result.object);
	
	return -1;
};

int ObjectInputStream::read(char *buff, int len) {
	if (!readFunc)
		return -1;

	if (assignedTraceId != OISF_READ) {
		assignedTraceId = OISF_READ;
		localExecuter.aststacktrace->clear();
		if (readFunc && readFunc->type == CODE_FUNCTION)
			localExecuter.insertStackTrace(((CodeFunction*) readFunc)->scope->context->executer);
		else 
			localExecuter.aststacktrace->insert(string("virtual"));
	}
	
	BytePtr *ptr = new BytePtr(buff, len, SAPTRALLOCTYPE::SAPTR_UNDEF);
	ExecuterResult result = localExecuter.beginFunction(readScope, this, readFunc, 2, ptr, new Integer(len));	
	
	if (result.type == ExecuterResultType::ERT_OBJECT)
		return objectIntValue(result.object);
	
	return -1;
};

int ObjectInputStream::eof() {
	if (!eofFunc)
		return -1;

	if (assignedTraceId != OISF_EOF) {
		assignedTraceId = OISF_EOF;
		localExecuter.aststacktrace->clear();
		if (readFunc && readFunc->type == CODE_FUNCTION)
			localExecuter.insertStackTrace(((CodeFunction*) eofFunc)->scope->context->executer);
		else 
			localExecuter.aststacktrace->insert(string("virtual"));
	}
	
	ExecuterResult result = localExecuter.beginFunction(eofScope, this, eofFunc, 0);	
	
	if (result.type == ExecuterResultType::ERT_OBJECT)
		return objectIntValue(result.object);
	
	return -1;
};

int ObjectInputStream::close() {
	if (!closeFunc)
		return -1;

	if (assignedTraceId != OISF_CLOSE) {
		assignedTraceId = OISF_CLOSE;
		localExecuter.aststacktrace->clear();
		if (readFunc && readFunc->type == CODE_FUNCTION)
			localExecuter.insertStackTrace(((CodeFunction*) closeFunc)->scope->context->executer);
		else 
			localExecuter.aststacktrace->insert(string("virtual"));
	}
	
	ExecuterResult result = localExecuter.beginFunction(closeScope, this, closeFunc, 0);	
	
	if (result.type == ExecuterResultType::ERT_OBJECT)
		return objectIntValue(result.object);
	
	return -1;
};


// ObjectInputStream prototype
ObjectInputStreamPrototype::ObjectInputStreamPrototype() {
	type = OBJECTINPUTSTREAM_PROTOTYPE;
};

VirtualObject *ObjectInputStreamPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	return new ObjectInputStream();
};



// ObjectOutputStream type
ObjectOutputStream::ObjectOutputStream() {
	type            = OBJECTOUTPUTSTREAM;
	assignedTraceId = OISF_NONE;
	writeScope      = NULL;
	closeScope      = NULL;
	flushScope      = NULL;
	writeFunc       = NULL;
	closeFunc       = NULL;
	flushFunc       = NULL;
	
	table->putAll(OOstream_prototype->table);
};

void ObjectOutputStream::setWriteFunc(Scope *scope, VirtualObject *f) {
	this->writeFunc = scope;
	if (assignedTraceId == OISF_WRITE)
		assignedTraceId = OISF_NONE;
	this->writeFunc = f;
	string key = "write";
	Object::put(NULL, &key, f);
};

void ObjectOutputStream::setCloseFunc(Scope *scope, VirtualObject *f) {
	this->closeScope = scope;
	if (assignedTraceId == OISF_CLOSE)
		assignedTraceId = OISF_NONE;
	this->closeFunc = f;
	string key = "close";
	Object::put(NULL, &key, f);
};

void ObjectOutputStream::setFlushFunc(Scope *scope, VirtualObject *f) {
	this->flushScope = scope;
	if (assignedTraceId == OISF_FLUSH)
		assignedTraceId = OISF_NONE;
	this->flushFunc = f;
	string key = "flush";
	Object::put(NULL, &key, f);
};

void ObjectOutputStream::put(Scope *scope, string *name, VirtualObject *value) {
	if (!name)
		return;
	
	if (*name == "read")
		setWriteFunc(scope, value);
	else if (*name == "close")
		setCloseFunc(scope, value);
	else if (*name == "flush")
		setFlushFunc(scope, value);
	
	Object::put(scope, name, value);
};

void ObjectOutputStream::remove(Scope *scope, string *name) {
	if (!name)
		return;
	
	if (*name == "read")
		setWriteFunc(NULL, NULL);
	else if (*name == "close")
		setCloseFunc(NULL, NULL);
	else if (*name == "flush")
		setFlushFunc(NULL, NULL);
	
	Object::remove(scope, name);
};

VirtualObject *ObjectOutputStream::call(Scope *scope, int argc, VirtualObject **args) {
	return NULL;
};

void ObjectOutputStream::mark() {
	if (gc_reachable)
		return;
	
	if (writeScope)
		writeScope->mark();
	if (closeScope)
		closeScope->mark();
	if (flushScope)
		flushScope->mark();
	
	if (writeFunc)
		writeFunc->mark();
	if (closeFunc)
		closeFunc->mark();
	if (flushFunc)
		flushFunc->mark();
	
	Object::mark();
}

int ObjectOutputStream::write(char b) {
	if (!writeFunc)
		return -1;

	if (assignedTraceId != OISF_WRITE) {
		assignedTraceId = OISF_WRITE;
		localExecuter.aststacktrace->clear();
		if (writeFunc && writeFunc->type == CODE_FUNCTION)
			localExecuter.insertStackTrace(((CodeFunction*) writeFunc)->scope->context->executer);
		else 
			localExecuter.aststacktrace->insert(string("virtual"));
	}
	
	ExecuterResult result = localExecuter.beginFunction(writeScope, this, writeFunc, 1, new Byte(b));	
	
	if (result.type == ExecuterResultType::ERT_OBJECT)
		return objectIntValue(result.object);
	
	return -1;
};

int ObjectOutputStream::write(char *buff, int off, int len) {
	if (!writeFunc)
		return -1;

	if (assignedTraceId != OISF_WRITE) {
		assignedTraceId = OISF_WRITE;
		localExecuter.aststacktrace->clear();
		if (writeFunc && writeFunc->type == CODE_FUNCTION)
			localExecuter.insertStackTrace(((CodeFunction*) writeFunc)->scope->context->executer);
		else 
			localExecuter.aststacktrace->insert(string("virtual"));
	}
	
	BytePtr *ptr = new BytePtr(buff, len, SAPTRALLOCTYPE::SAPTR_UNDEF);
	ExecuterResult result = localExecuter.beginFunction(writeScope, this, writeFunc, 3, ptr, new Integer(off), new Integer(len));	
	
	if (result.type == ExecuterResultType::ERT_OBJECT)
		return objectIntValue(result.object);
	
	return -1;
};

int ObjectOutputStream::write(char *buff, int len) {
	if (!writeFunc)
		return -1;

	if (assignedTraceId != OISF_WRITE) {
		assignedTraceId = OISF_WRITE;
		localExecuter.aststacktrace->clear();
		if (writeFunc && writeFunc->type == CODE_FUNCTION)
			localExecuter.insertStackTrace(((CodeFunction*) writeFunc)->scope->context->executer);
		else 
			localExecuter.aststacktrace->insert(string("virtual"));
	}
	
	BytePtr *ptr = new BytePtr(buff, len, SAPTRALLOCTYPE::SAPTR_UNDEF);
	ExecuterResult result = localExecuter.beginFunction(writeScope, this, writeFunc, 2, ptr, new Integer(len));	
	
	if (result.type == ExecuterResultType::ERT_OBJECT)
		return objectIntValue(result.object);
	
	return -1;
};

int ObjectOutputStream::close() {
	if (!closeFunc)
		return -1;

	if (assignedTraceId != OISF_CLOSE) {
		assignedTraceId = OISF_CLOSE;
		localExecuter.aststacktrace->clear();
		if (closeFunc && closeFunc->type == CODE_FUNCTION)
			localExecuter.insertStackTrace(((CodeFunction*) closeFunc)->scope->context->executer);
		else 
			localExecuter.aststacktrace->insert(string("virtual"));
	}
	
	ExecuterResult result = localExecuter.beginFunction(closeScope, this, closeFunc, 0);	
	
	if (result.type == ExecuterResultType::ERT_OBJECT)
		return objectIntValue(result.object);
	
	return -1;
};

int ObjectOutputStream::flush() {
	if (!closeFunc)
		return -1;

	if (assignedTraceId != OISF_FLUSH) {
		assignedTraceId = OISF_FLUSH;
		localExecuter.aststacktrace->clear();
		if (flushFunc && flushFunc->type == CODE_FUNCTION)
			localExecuter.insertStackTrace(((CodeFunction*) closeFunc)->scope->context->executer);
		else 
			localExecuter.aststacktrace->insert(string("virtual"));
	}
	
	ExecuterResult result = localExecuter.beginFunction(flushScope, this, flushFunc, 0);	
	
	if (result.type == ExecuterResultType::ERT_OBJECT)
		return objectIntValue(result.object);
	
	return -1;
};



// ObjectOutputStream prototype
ObjectOutputStreamPrototype::ObjectOutputStreamPrototype() {
	type = OBJECTOUTPUTSTREAM_PROTOTYPE;
};

VirtualObject *ObjectOutputStreamPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	return new ObjectOutputStream();
};


// Called on load. Defines integer prototype & type
void define_oio(Scope *scope) {
	Scope *rootScope = scope->getRoot();
	OIstream_prototype = new ObjectInputStreamPrototype();
	rootScope->table->put(string("ObjectInputStream"), OIstream_prototype);
	
	OIstream_prototype->table->put(string("__typename"), new String("ObjectInputStream"));
	
	OOstream_prototype = new ObjectOutputStreamPrototype();
	rootScope->table->put(string("ObjectOutputStream"), OOstream_prototype);
	
	OOstream_prototype->table->put(string("__typename"), new String("ObjectOutputStream"));
};


// - - - - - - - - - - - - - - - - - - - - B Y T E _ T Y P E

BytePrototype *byte_prototype = NULL;

// Byte type
Byte::Byte() {		
	type  = BYTE_TYPE;
	value = 0;
};

Byte::Byte(int i) {		
	type  = BYTE_TYPE;
	value = i;
};

VirtualObject *Byte::get(Scope *scope, string *name) {
	return byte_prototype->table->get(*name);
};

void Byte::put(Scope *scope, string *name, VirtualObject *value) {};

void Byte::remove(Scope *scope, string *name) {};

bool Byte::contains(Scope *scope, string *name) {
	return byte_prototype->contains(scope, name);
};

VirtualObject *Byte::call(Scope *scope, int argc, VirtualObject **args) {
	return new Byte(value);
};

long Byte::toInt() {
	return value;
};

double Byte::toDouble() {
	return value;
};

string Byte::toString() {
	return string::toString(value % 256);
};


// Byte prototype	
BytePrototype::BytePrototype() {		
	type  = BYTE_TYPE_PROTOTYPE;
};

VirtualObject *BytePrototype::call(Scope *scope, int argc, VirtualObject **args) {
	if (argc)
		return new Byte(objectIntValue(args[0]));
	return new Byte;
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
	
	return new Integer(objectIntValue(args[0]) + objectIntValue(args[1]));
};

// -
static VirtualObject* operator_sub(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]) + objectIntValue(args[1]));
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
	
	Byte       *a = (Byte*) args[0];
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
	
	return new Integer(objectIntValue(args[0]) + 1);
};

// --
static VirtualObject* operator_dec(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 1)
		return new Undefined;
	
	return new Integer(objectIntValue(args[0]) - 1);
};


static char baseDigit(int digit) {
	if (digit < 10)
		return digit + '0';
	return 'a' + digit - 10;
};

// Functions
// toString(integer[, base])
static VirtualObject* function_tostring(Scope *scope, int argc, VirtualObject **args) {
	if (!argc || args[0]->type != BYTE_TYPE)
		return NULL;
	
	int i = ((Byte*) args[0])->value;
	int b = argc == 1 ? 10 : objectIntValue(args[1]);
	
	if (b < 2 || b > 36)
		return new Undefined;
	
	ptr_wrapper wrapper(new string, PTR_NEW);
	string *s = (string*) wrapper.ptr;
	
	*s += string::toString((long long) i, b);
	
	wrapper.deattach();
	return new String(s);
};


static void printObject(void *o, size_t size) {
	char *c = (char*) o;
	printf("0x%08x[%d]: ", (int) o, size);
	for (int i = 0; i < size; ++i)
		printf("%02X", c[i]);
	printf("\n");
};

// Called on start. Defines byte prototype & type
void define_byte(Scope *scope) {
	Scope *rootScope = scope->getRoot();
	byte_prototype = new BytePrototype();
	
	rootScope->define(string("Byte"), byte_prototype);
	
	byte_prototype->table->put(string("__typename"),    new String("Byte"));
	byte_prototype->table->put(string("__operator=="),  new NativeFunction(&operator_eq));
	byte_prototype->table->put(string("__operator!="),  new NativeFunction(&operator_neq));
	byte_prototype->table->put(string("__operator>"),   new NativeFunction(&operator_gt));
	byte_prototype->table->put(string("__operator<"),   new NativeFunction(&operator_lt));
	byte_prototype->table->put(string("__operator>="),  new NativeFunction(&operator_ge));
	byte_prototype->table->put(string("__operator>="),  new NativeFunction(&operator_le));
	byte_prototype->table->put(string("__operator+"),   new NativeFunction(&operator_sum));
	byte_prototype->table->put(string("__operator-"),   new NativeFunction(&operator_sub));
	byte_prototype->table->put(string("__operator*"),   new NativeFunction(&operator_mul));
	byte_prototype->table->put(string("__operator/"),   new NativeFunction(&operator_div));
	byte_prototype->table->put(string("__operator%"),   new NativeFunction(&operator_mod));
	byte_prototype->table->put(string("__operator<<"),  new NativeFunction(&operator_div));
	byte_prototype->table->put(string("__operator>>"),  new NativeFunction(&operator_shr));
	byte_prototype->table->put(string("__operator>>>"), new NativeFunction(&operator_ushr));
	byte_prototype->table->put(string("__operator&"),   new NativeFunction(&operator_band));
	byte_prototype->table->put(string("__operator|"),   new NativeFunction(&operator_bor));
	byte_prototype->table->put(string("__operator&&"),  new NativeFunction(&operator_and));
	byte_prototype->table->put(string("__operator||"),  new NativeFunction(&operator_or));
	byte_prototype->table->put(string("__operator^"),   new NativeFunction(&operator_bxor));
	byte_prototype->table->put(string("__operator-x"),  new NativeFunction(&operator_neg));
	byte_prototype->table->put(string("__operator+x"),  new NativeFunction(&operator_pos));
	byte_prototype->table->put(string("__operator!x"),  new NativeFunction(&operator_not));
	byte_prototype->table->put(string("__operator~x"),  new NativeFunction(&operator_bnot));
	byte_prototype->table->put(string("__operator++"),  new NativeFunction(&operator_inc));
	byte_prototype->table->put(string("__operator--"),  new NativeFunction(&operator_dec));
	
	byte_prototype->table->put(string("toString"),      new NativeFunction(&function_tostring));
	byte_prototype->table->put(string("SIZE"),          new Integer(sizeof(char)));
};


// - - - - - - - - - - - - - - - - - - - - B Y T E _ P O I N T E R _ T Y P E

BytePtrPrototype *BytePtr_prototype = NULL;

// BytePtr type
BytePtr::BytePtr(size_t size) {
	allocType  = SAPTRALLOCTYPE::SAPTR_ALLOC;
	this->size = size;
	this->ptr  = (char*) malloc(size);
	
	this->table->putAll(BytePtr_prototype->table);
	this->table->put(string("size"), new Integer(size));
};

BytePtr::BytePtr(char *ptr, size_t size) {
	allocType  = SAPTRALLOCTYPE::SAPTR_ALLOC;
	this->size = size;
	this->ptr  = ptr;
	
	this->table->putAll(BytePtr_prototype->table);
	this->table->put(string("size"), new Integer(size));
};

BytePtr::BytePtr(char *ptr, size_t size, SAPTRALLOCTYPE allocType) {
	allocType  = allocType;
	this->size = size;
	this->ptr  = ptr;
	
	this->table->putAll(BytePtr_prototype->table);
	this->table->put(string("size"), new Integer(size));
};

VirtualObject *BytePtr::get(Scope *scope, string *name) {
	if (!name)
		return NULL;
	
	int index = name->toInt(10, -1);
	if (index == -1 && *name != "-1")
		return Object::get(scope, name);
	
	if (index < 0 || index >= size) {
		scope->context->executer->raiseError("Native pointer out of bounds");
		return NULL;
	}
	
	return new Byte(ptr[index]);
};

void BytePtr::put(Scope *scope, string *name, VirtualObject *value) {
	int index = name->toInt(10, -1);
	if (index == -1 && *name != "-1") {
		Object::put(scope, name, value);
		return;
	}
	
	if (index < 0 || index >= size) {
		scope->context->executer->raiseError("Native pointer out of bounds");
		return NULL;
	}
	
	ptr[index] = objectIntValue(value);
};

VirtualObject *BytePtr::call(Scope *scope, int argc, VirtualObject **args) {
	scope->context->executer->raiseError("That's not C++");
	return NULL;
};

void BytePtr::finalize() {
	if (allocType == SAPTR_ALLOC)
		free(ptr);
	if (allocType == SAPTR_NEW)
		free(ptr);
};


// BytePtr prototype	
BytePtrPrototype::BytePtrPrototype() {
	type = BYTEPTR_PROTOTYPE;
};

VirtualObject *BytePtrPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	if (argc)
		return new BytePtr(objectIntValue(args[0]));
	return new BytePtr(1);
};


/* char to wchar_t */
static wchar_t* sa_bp_ctow(const char* c) {
	setlocale(LC_ALL, "");	
	
	int i = (int) mbstowcs(NULL, c, 0);
	
	if (i < 0)
		return NULL;
	
	wchar_t *d = (wchar_t*) malloc((i + 1) * sizeof(wchar_t));
	mbstowcs(d, c, i);
	d[i] = 0;
	
	return d;
}

// Convert String to UTF-8 representing bytes
static VirtualObject* bp_string_getBytes(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object) && o->type == STRING))
		return NULL;

	string ns;
	String *s = (String*) o;
	for (int i = 0; i < s->stringLength(); ++i)
		ns += s->stringValue()->buffer[s->stringOffset() + i];
	
	char *cs = ns.toCString();
	int l = 0;
	while (cs[l]) ++l;
	return new BytePtr(cs, l);
};

// Convert UTF-8 representing bytes to string
static VirtualObject* bp_string_fromBytes(Scope *scope, int argc, VirtualObject **args) {
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object) && o->type == STRING))
		return NULL;

	if (args[0]->type != BYTEPTR)
		return NULL;
	
	BytePtr *bp = (BytePtr*) o;
	if (!bp->size)
		return new String("");
	
	if (bp->ptr[bp->size - 1] == 0) {
		wchar_t *st = sa_bp_ctow(bp->ptr);
		if (st == NULL) {
			scope->context->executer->raiseError("unable convert bytes to string");
			return NULL;
		}
		
		String *rs = new String(new string(st));
		free(st);
		
		return rs;
	} else {
		char *temp = malloc(bp->size + 1);
		for (int i = 0; i < bp->size; ++i)
			temp[i] = bp->ptr[i];
		temp[bp->size] = 0;
		
		wchar_t *st = sa_bp_ctow(bp->ptr);
		if (st == NULL) {
			scope->context->executer->raiseError("unable convert bytes to string");
			free(temp);
			return NULL;
		}
		
		String *rs = new String(new string(st));
		free(st);
		
		return rs;
	}
};


// Called on start. Defines BytePtr prototype & type
void define_byteptr(Scope *scope) {
	Scope *rootScope = scope->getRoot();
	BytePtr_prototype = new BytePtrPrototype();
	rootScope->table->put(string("BytePtr"), BytePtr_prototype);
	
	BytePtr_prototype->table->put(string("__typename"),    new String("BytePtr"));
	
	// Additional functions for support string to bytes conversion
	string_prototype->table->put(string("getBytes"), new NativeFunction(&bp_string_getBytes));
};

#ifdef __cplusplus
extern "C" {
#endif


// Called on module being loaded by bla bla bla
void onLoad(Scope *scope, int argc, VirtualObject **args) {
	define_byte(scope);
	define_oio(scope);
	define_byteptr(scope);
};

// Called on module being unloaded on program exit
void onUnload() {};


#ifdef __cplusplus
}
#endif


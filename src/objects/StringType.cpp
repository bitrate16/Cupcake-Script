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
#include "Integer.h"
#include "Boolean.h"
#include "NativeFunction.h"
#include "StringType.h"

#include "ObjectConverter.h"
#include "LinkedObjectMap.h"

#include "../string.h"
#include "../ptr_wrapper.h"

StringPrototype *string_prototype = NULL;


// String type
String::String() {		
	type   = STRING;
	value  = new string;
	parent = NULL;
	offset = -1;
	length = -1;
};

String::String(string s) {		
	type   = STRING;
	value  = new string(s);
	parent = NULL;
	offset = -1;
	length = -1;
};

String::String(string *s) {		
	type   = STRING;
	value  = s;
	parent = NULL;
	offset = -1;
	length = -1;
};

String::String(const char *s) {		
	type   = STRING;
	value  = new string(s);
	parent = NULL;
	offset = -1;
	length = -1;
};

String::String(String *parent, int offset, int length) {
	// printf("==> %d is referred by %d\n", (int) parent, (int) this);
	this->type  = STRING;
	this->value = NULL;
	String *p   = parent;
	
	while (p->parent) {
		offset += p->offset;
		p = p->parent;
	}
	
	this->parent = p;
	this->offset = offset;
	this->length = length;
};

String::String(String *parent, int offset) {
	// printf("==> %d is referred by %d\n", (int) parent, (int) this);
	this->type  = STRING;
	this->value = NULL;
	String *p   = parent;
	
	while (p->parent) {
		offset += p->offset;
		p = p->parent;
	}
	
	this->parent = p;
	this->offset = offset;
	this->length = p->value->length - offset;
};

void String::finalize(void) {
	delete value;
};

VirtualObject *String::get(Scope *scope, string *name) {
	int i = parsePositiveInt();
	
	if (i > stringLength())
		return new Undefined;
	
	if (i != -1)
		return new String(this, i, 1);
	
	return string_prototype->table->get(*name);
};

void String::put(Scope *scope, string *name, VirtualObject *value) {};

void String::remove(Scope *scope, string *name) {};

bool String::contains(Scope *scope, string *name) {
	return string_prototype->table->contains(*name);
};

VirtualObject *String::call(Scope *scope, int argc, VirtualObject **args) {
	return new String(value);
};

void String::mark(void) {
	if (gc_reachable)
		return;
	gc_reachable = 1;
	
	if (parent && !parent->gc_reachable)
		parent->mark();
	
};

int String::stringLength() {
	return value ? value->length : length;
};

int String::stringOffset() {
	return parent ? offset : 0;
};

string *String::stringValue() {
	return parent ? parent->stringValue() : value;
};

static bool digit(int c) {
	return '0' <= c && c <= '9';
};

int String::parsePositiveInt() {
	if (stringLength() == 0)
		return -1;
	
	int n = 0;
	for (int i = stringOffset(); i < stringLength(); ++i) {
		if (n >= 0 && digit(stringValue()->buffer[i]))
			n = n * 10 + (stringValue()->buffer[i] - '0');
		else
			return -1;
	}
	
	return n;	
};

int String::stringCharAt(int index) {
	return index < 0 || index >= stringLength() ? -1 : stringValue()->buffer[stringOffset() + index];
};


// String prototype	
StringPrototype::StringPrototype() {		
	// table = new TreeObjectMap;
	type  = STRING_PROTOTYPE;
};

void StringPrototype::finalize(void) {
	table->finalize();
};

VirtualObject *StringPrototype::get(Scope *scope, string *name) {
	return table->get(*name);
};

void StringPrototype::put(Scope *scope, string *name, VirtualObject *value) {
	table->put(*name, value);
};

void StringPrototype::remove(Scope *scope, string *name) {
	table->remove(*name);
};

bool StringPrototype::contains(Scope *scope, string *name) {
	return table->contains(*name);
};

VirtualObject *StringPrototype::call(Scope *scope, int argc, VirtualObject **args) {
	if (argc)
		return new String(objectStringValue(args[0]));
	return new String;
};

void StringPrototype::mark(void) {
	if (gc_reachable)
		return;
	gc_reachable = 1;
	table->mark();
};


// Operators

// ==
static VirtualObject* operator_eq(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 1)
		return new Undefined;
	
	if (args[0] == args[1])
		return new Boolean(1);
	
	if (args[1]->type == STRING) {
		string *a = (string*) ((String*) args[0])->stringValue();
		string *b = (string*) ((String*) args[1])->stringValue();
		
		if (a == b)
			return new Boolean(1);
		
		int lena = ((String*) args[0])->stringLength();
		int lenb = ((String*) args[1])->stringLength();
		
		if (lena != lenb)
			return new Boolean;
		
		int offa = ((String*) args[0])->stringOffset();
		int offb = ((String*) args[1])->stringOffset();
		
		for (int i = 0; i < lena; ++i) 
			if (a->buffer[i + offa] != b->buffer[i + offb])
				return new Boolean;
			
		return new Boolean(1);
	}
	
	return new Boolean(0);
};

// !=
static VirtualObject* operator_neq(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 1)
		return new Undefined;
	
	if (args[0] == args[1])
		return new Boolean;
	
	if (args[1]->type == STRING) {
		string *a = (string*) ((String*) args[0])->stringValue();
		string *b = (string*) ((String*) args[1])->stringValue();
		
		if (a == b)
			return new Boolean;
		
		int lena = ((String*) args[0])->stringLength();
		int lenb = ((String*) args[1])->stringLength();
		
		if (lena != lenb)
			return new Boolean(1);
		
		int offa = ((String*) args[0])->stringOffset();
		int offb = ((String*) args[1])->stringOffset();
		
		for (int i = 0; i < lena; ++i) 
			if (a->buffer[i + offa] != b->buffer[i + offb])
				return new Boolean(1);
			
		return new Boolean(0);
	}
	
	return new Boolean(1);
};

// +
static VirtualObject* operator_sum(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	if (args[0]->type != STRING)
		return new Undefined;
	
	String *a = (String*) args[0];
	
	if (args[1]->type == STRING) {
		String *h = (String*) args[1];
		
		ptr_wrapper wrapper(new string, PTR_NEW);
		string *n = (string*) wrapper.ptr;
		
		for (int i = 0; i < a->stringLength(); ++i)
			*n += a->stringValue()->buffer[i + a->stringOffset()];
		
		for (int i = 0; i < h->stringLength(); ++i)
			*n += h->stringValue()->buffer[i + h->stringOffset()];
		
		wrapper.deattach();
		
		return new String(n);
	} else {
		string h = objectStringValue(args[1]);
		
		ptr_wrapper wrapper(new string, PTR_NEW);
		string *n = (string*) wrapper.ptr;
		
		for (int i = 0; i < a->stringLength(); ++i)
			*n += a->stringValue()->buffer[i + a->stringOffset()];
		
		for (int i = 0; i < h.length; ++i)
			*n += h.buffer[i];
		
		wrapper.deattach();
		
		return new String(n);
	}
};

// *
static VirtualObject* operator_mul(Scope *scope, int argc, VirtualObject **args) {
	if (argc != 2)
		return new Undefined;
	
	if (args[0]->type != STRING)
		return new Undefined;
	
	String        *a = (String*) args[0];
	VirtualObject *b = args[1];
	
	int n = objectIntValue(args[1]);
	n = n > 0 ? n : -n;
	
	string s = "";
	
	for (int i = 0; i < n; ++i)
		s += *a->value;
	
	return new String(s);
};


// Functions
// length()
static VirtualObject* function_length(Scope *scope, int argc, VirtualObject **args) {
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
	// 5. parent scope object type == STRING
	
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)) && o->type == STRING)
		return NULL;
	
	return new Integer(((String*) o)->stringLength());
};

// substring(offset[, length])
static VirtualObject* function_substring(Scope *scope, int argc, VirtualObject **args) {
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
	// 5. parent scope object type == STRING
	
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)) && o->type == STRING)
		return NULL;
	
	String *s = (String*) o;
	
	if (argc == 1) {
		int offset = objectIntValue(args[0]);
		
		if (offset < 0 || offset >= s->stringLength()) 
			return new Undefined;
		
		return new String(s, offset);
	} else {
		int offset = objectIntValue(args[0]);
		int length = objectIntValue(args[1]);
		
		if (offset < 0 || length < 0 || offset + length >= s->stringLength()) 
			return new Undefined;
		
		return new String(s, offset, length);
	}
};

// charAt(index)
static VirtualObject* function_charat(Scope *scope, int argc, VirtualObject **args) {
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
	// 5. parent scope object type == STRING
	
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)) && o->type == STRING)
		return NULL;
	
	String *s = (String*) o;
	
	int index = objectIntValue(args[0]);
	
	if (index >= s->stringLength())
		return new Undefined;
	
	return new String(s, index, 1);
};

// charCodeAt(index)
static VirtualObject* function_charcodeat(Scope *scope, int argc, VirtualObject **args) {
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
	// 5. parent scope object type == STRING
	
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)) && o->type == STRING)
		return NULL;
	
	String *s = (String*) o;
	
	int index = objectIntValue(args[0]);
	
	if (index >= s->stringLength())
		return new Undefined;
	
	return new Integer(s->stringValue()->buffer[s->stringOffset() + index]);
};

// startsWith(other_string)
static VirtualObject* function_startswith(Scope *scope, int argc, VirtualObject **args) {
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
	// 5. parent scope object type == STRING
	
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)) && o->type == STRING)
		return NULL;
	
	String *s = (String*) o;
	
	if (args[0]->type == STRING) {
		String *h = (String*) args[0];
		
		if (h->stringLength() > s->stringLength())
			return new Boolean;
		
		for (int i = 0; i < h->stringLength(); ++i)
			if (h->stringValue()->buffer[i + h->stringOffset()] != s->stringValue()->buffer[i + s->stringOffset()])
				return new Boolean;
			
		return new Boolean(1);
	} else {
		string h = objectStringValue(args[0]);
		
		if (h.length > s->stringLength()) 
			return new Boolean;
		
		for (int i = 0; i < h.length; ++i)
			if (h.buffer[i] != s->stringValue()->buffer[i + s->stringOffset()])
				return new Boolean;
		
		return new Boolean(1);
	}
};

// endsWith(other_string)
static VirtualObject* function_endswith(Scope *scope, int argc, VirtualObject **args) {
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
	// 5. parent scope object type == STRING
	
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)) && o->type == STRING)
		return NULL;
	
	String *s = (String*) o;
	
	if (args[0]->type == STRING) {
		String *h = (String*) args[0];
		
		if (h->stringLength() > s->stringLength())
			return new Boolean;
		
		for (int i = 0; i < h->stringLength(); ++i)
			if (h->stringValue()->buffer[i + h->stringOffset()] != s->stringValue()->buffer[i + s->stringOffset() + s->stringLength() - h->stringLength()])
				return new Boolean;
			
		return new Boolean(1);
	} else {
		string h = objectStringValue(args[0]);
		
		if (h.length > s->stringLength()) 
			return new Boolean;
		
		for (int i = 0; i < h.length; ++i)
			if (h.buffer[i] != s->stringValue()->buffer[i + s->stringOffset() + s->stringLength() - h.length])
				return new Boolean;
		
		return new Boolean(1);
	}
};

// XXX: Concat string without doubling string.
// concat(other_string)
static VirtualObject* function_concat(Scope *scope, int argc, VirtualObject **args) {
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
	// 5. parent scope object type == STRING
	
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)) && o->type == STRING)
		return NULL;
	
	String *a = (String*) o;
	
	if (args[0]->type == STRING) {
		String *h = (String*) args[0];
		
		ptr_wrapper wrapper(new string, PTR_NEW);
		string *n = (string*) wrapper.ptr;
		
		for (int i = 0; i < a->stringLength(); ++i)
			*n += a->stringValue()->buffer[i + a->stringOffset()];
		
		for (int i = 0; i < h->stringLength(); ++i)
			*n += h->stringValue()->buffer[i + h->stringOffset()];
		
		wrapper.deattach();
		
		return new String(n);
	} else {
		string h = objectStringValue(args[0]);
		
		ptr_wrapper wrapper(new string, PTR_NEW);
		string *n = (string*) wrapper.ptr;
		
		for (int i = 0; i < a->stringLength(); ++i)
			*n += a->stringValue()->buffer[i + a->stringOffset()];
		
		for (int i = 0; i < h.length; ++i)
			*n += h.buffer[i];
		
		wrapper.deattach();
		
		return new String(n);
	}
};

// replace(substring, replacement)
static VirtualObject* function_replace(Scope *scope, int argc, VirtualObject **args) {
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
	// 5. parent scope object type == STRING
	
	VirtualObject *o;
	if (!(argc >= 2 && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)) && o->type == STRING)
		return NULL;
	
	String *a = (String*) o;
	string  s = objectStringValue(args[0]);
	if (s.length == 0)
		return new Undefined();
	
	string  r = objectStringValue(args[1]);
	
	ptr_wrapper wrapper(new string, PTR_NEW);
	string *n = (string*) wrapper.ptr;
	
	int matchlen = 0;
	int i = 0;
	
	while (1) {		
		if (matchlen)
			if (matchlen == s.length) {
				n->length -= matchlen;
				*n        += r;
				matchlen   = 0;
			} else {
				if (i >= a->stringLength())
					break;
				
				*n += a->stringValue()->buffer[i + a->stringOffset()];
				
				if (a->stringValue()->buffer[i + a->stringOffset()] == s.buffer[matchlen]) {
					++matchlen;
					++i;
				} else {
					matchlen = 0;
					++i;
				}
			}
		else {
			if (i >= a->stringLength())
				break;
		
			*n += a->stringValue()->buffer[i + a->stringOffset()];
			
			if (a->stringValue()->buffer[i + a->stringOffset()] == s.buffer[matchlen])
				++matchlen;
			
			++i;
		}
	}
	
	wrapper.deattach();
	
	return new String(n);
};

// contains(substring)
static VirtualObject* function_contains(Scope *scope, int argc, VirtualObject **args) {
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
	// 5. parent scope object type == STRING
	
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)) && o->type == STRING)
		return NULL;
	
	String *a = (String*) o;
	string  s = objectStringValue(args[0]);
	if (s.length == 0)
		return new Boolean(1);
	
	int matchlen = 0;
	int i = 0;
	
	while (1) {		
		if (matchlen)
			if (matchlen == s.length) 
				return new Boolean(1);
			else {
				if (i >= a->stringLength())
					break;
				
				if (a->stringValue()->buffer[i + a->stringOffset()] == s.buffer[matchlen]) {
					++matchlen;
					++i;
				} else
					matchlen = 0;
			}
		else {
			if (i >= a->stringLength())
				break;
			
			if (a->stringValue()->buffer[i + a->stringOffset()] == s.buffer[matchlen])
				++matchlen;
			
			++i;
		}
	}
	
	return new Boolean;
};

// indexOf(substring)
static VirtualObject* function_indexof(Scope *scope, int argc, VirtualObject **args) {
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
	// 5. parent scope object type == STRING
	
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)) && o->type == STRING)
		return NULL;
	
	String *a = (String*) o;
	string  s = objectStringValue(args[0]);
	if (s.length == 0)
		return new Boolean(1);
	
	int matchlen = 0;
	int i = 0;
	
	while (1) {		
		if (matchlen)
			if (matchlen == s.length) 
				return new Integer(i);
			else {
				if (i >= a->stringLength())
					break;
				
				if (a->stringValue()->buffer[i + a->stringOffset()] == s.buffer[matchlen]) {
					++matchlen;
					++i;
				} else
					matchlen = 0;
			}
		else {
			if (i >= a->stringLength())
				break;
			
			if (a->stringValue()->buffer[i + a->stringOffset()] == s.buffer[matchlen])
				++matchlen;
			
			++i;
		}
	}
	
	return new Integer(-1);
};

// lastIndexOf(substring)
static VirtualObject* function_lastindexof(Scope *scope, int argc, VirtualObject **args) {
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
	// 5. parent scope object type == STRING
	
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)) && o->type == STRING)
		return NULL;
	
	String *a = (String*) o;
	string  s = objectStringValue(args[0]);
	if (s.length == 0)
		return new Boolean(1);
	
	int alen = a->stringLength();
	int slen = s.length;
	int matchlen = 0;
	int i = 0;
	
	while (1) {		
		if (matchlen)
			if (matchlen == s.length) 
				return new Integer(alen - i);
			else {
				if (i >= a->stringLength())
					break;
				
				if (a->stringValue()->buffer[a->stringOffset() + alen - i - 1] == s.buffer[matchlen]) {
					++matchlen;
					++i;
				} else
					matchlen = 0;
			}
		else {
			if (i >= a->stringLength())
				break;
			
			if (a->stringValue()->buffer[a->stringOffset() + alen - i - 1] == s.buffer[matchlen])
				++matchlen;
			
			++i;
		}
	}
	
	return new Integer(-1);
};

// splitBy(delimiter_string[, piece_count])
static VirtualObject* function_splitby(Scope *scope, int argc, VirtualObject **args) {
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
	// 5. parent scope object type == STRING
	
	VirtualObject *o;
	if (!(argc && scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)) && o->type == STRING)
		return NULL;
	
	String *a = (String*) o;
	
	string  d = objectStringValue(args[0]);
	if (d.length == 0)
		return new Undefined();
	
	ptr_wrapper wrapper(new Array, PTR_NEW);
	Array *n = (Array*) wrapper.ptr;
	
	int max_pieces = argc > 1 ? objectIntValue(args[1]) : -1;
	
	string temp;
	int matchlen = 0;
	int matchcnt = 1;
	int i        = 0;
	
	while (1) {		
		if (matchlen)
			if (matchlen == d.length) {
				temp.length -= matchlen;
				matchlen     = 0;
				n->push(new String(temp));
				temp.length  = 0;
				++matchcnt;
			} else {
				if (i >= a->stringLength())
					break;
				
				temp += a->stringValue()->buffer[i + a->stringOffset()];
				
				if (a->stringValue()->buffer[i + a->stringOffset()] == d.buffer[matchlen]) {
					++matchlen;
					++i;
				} else
					matchlen = 0;
			}
		else {
			if (i >= a->stringLength())
				break;
		
			if (max_pieces != -1 && matchcnt >= max_pieces)
				break;
		
			temp += a->stringValue()->buffer[i + a->stringOffset()];
			
			if (a->stringValue()->buffer[i + a->stringOffset()] == d.buffer[matchlen])
				++matchlen;
			
			++i;
		}
	}
	
	if ((max_pieces == -1 || matchcnt <= max_pieces) && i < a->stringLength()) {
		for (; i < a->stringLength(); ++i)
			temp += a->stringValue()->buffer[i + a->stringOffset()];
		
		n->push(new String(temp));
	} else if (temp.length != 0)
		n->push(new String(temp));
	
	wrapper.deattach();
	
	return n;
};

static inline bool whitespace(wchar_t c) {
	return c == ' ' || c == '\t' || c == '\n';
};

// trim()
static VirtualObject* function_trim(Scope *scope, int argc, VirtualObject **args) {
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
	// 5. parent scope object type == STRING
	
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)) && o->type == STRING)
		return NULL;
	
	String *a  = (String*) o;
	int down   = 0;
	int up     = 0;
	
	for (down = 0; down < a->stringLength(); ++down)
		if (!whitespace(a->stringValue()->buffer[a->stringOffset() + down]))
			break;
		
	for (up = a->stringLength() - 1; up >= 0 ; --up)
		if (!whitespace(a->stringValue()->buffer[a->stringOffset() + up]))
			break;
		
	if (up <= down)
		return new String("");
	
	
	return new String(a, down, up - down + 1);
};

// toUpperCase()
static VirtualObject* function_touppercase(Scope *scope, int argc, VirtualObject **args) {
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
	// 5. parent scope object type == STRING
	
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)) && o->type == STRING)
		return NULL;
	
	String *a = (String*) o;
	
	ptr_wrapper wrapper(new String(new string), PTR_NEW);
	
	string *n = (string*) ((String*) wrapper.ptr)->value;
	
	for (int i = 0; i < a->stringLength(); ++i)
		*n += string::toUpper(a->stringCharAt(i));
	
	wrapper.deattach();
	
	return (String*) wrapper.ptr;
};

// toLowerCase()
static VirtualObject* function_tolowercase(Scope *scope, int argc, VirtualObject **args) {
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
	// 5. parent scope object type == STRING
	
	VirtualObject *o;
	if (!(scope && scope->parent && scope->parent->type == PROXY_SCOPE && (o = ((ProxyScope*) scope->parent)->object)) && o->type == STRING)
		return NULL;
	
	String *a = (String*) o;
	
	ptr_wrapper wrapper(new String(new string), PTR_NEW);
	
	string *n = (string*) ((String*) wrapper.ptr)->value;
	
	for (int i = 0; i < a->stringLength(); ++i)
		*n += string::toLower(a->stringCharAt(i));
	
	wrapper.deattach();
	
	return (String*) wrapper.ptr;
};



// Called on start. Defines string prototype & type
void define_string(Scope *scope) {
	string_prototype = new StringPrototype();
	scope->table->put(string("String"), string_prototype);
	
	string_prototype->table->put(string("__typename"),     new String("String"));
	string_prototype->table->put(string("__operator=="),   new NativeFunction(&operator_eq));
	string_prototype->table->put(string("__operator!="),   new NativeFunction(&operator_neq));
	string_prototype->table->put(string("__operator+"),    new NativeFunction(&operator_sum));
	string_prototype->table->put(string("__operator*"),    new NativeFunction(&operator_mul));
	
	string_prototype->table->put(string("length"),         new NativeFunction(&function_length));
	string_prototype->table->put(string("subString"),      new NativeFunction(&function_substring));
	string_prototype->table->put(string("charAt"),         new NativeFunction(&function_charat));
	string_prototype->table->put(string("charCodeAt"),     new NativeFunction(&function_charcodeat));
	string_prototype->table->put(string("startsWith"),     new NativeFunction(&function_startswith));
	string_prototype->table->put(string("endsWith"),       new NativeFunction(&function_endswith));
	string_prototype->table->put(string("concat"),         new NativeFunction(&function_concat));
	string_prototype->table->put(string("replace"),        new NativeFunction(&function_replace));
	string_prototype->table->put(string("contains"),       new NativeFunction(&function_contains));
	string_prototype->table->put(string("indexOf"),        new NativeFunction(&function_indexof));
	string_prototype->table->put(string("lastIndexOf"),    new NativeFunction(&function_lastindexof));
	string_prototype->table->put(string("splitBy"),        new NativeFunction(&function_splitby));
	string_prototype->table->put(string("trim"),           new NativeFunction(&function_trim));
	string_prototype->table->put(string("toUpperCase"),    new NativeFunction(&function_touppercase));
	string_prototype->table->put(string("toLowerCase"),    new NativeFunction(&function_tolowercase));
};


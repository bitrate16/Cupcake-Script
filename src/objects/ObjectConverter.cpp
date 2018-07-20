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


#include <typeinfo>

#include "Scope.h"
#include "Null.h"
#include "Undefined.h"
#include "Integer.h"
#include "Boolean.h"
#include "NativeFunction.h"
#include "CodeFunction.h"
#include "StringType.h"
#include "Double.h"
#include "Object.h"
#include "Array.h"
#include "ObjectConverter.h"

#include "../TokenNamespace.h"


long IntCastObject::toInt() { return 1; };
double DoubleCastObject::toDouble() { return 0; };
string StringCastObject::toString() { return string("[VirtualObject]"); };


long objectIntValue(VirtualObject *o) {
	if (o == NULL)
		return 0;

	try {
		IntCastObject *convt = dynamic_cast<IntCastObject*>(o);
		if (convt)
			return convt->toInt();
	} catch(...) {}
	
	switch (o->type) {
		case NONE:
		case TNULL:
		case UNDEFINED:
			return 0;
		
		case SCOPE:
		case PROXY_SCOPE:
			return 1;
			
		case INTEGER:
			return ((Integer*) o)->value;
			
		case BOOLEAN:
			return ((Boolean*) o)->value;
			
		case NATIVE_FUNCTION:
			return *((int*) &((NativeFunction*) o)->handler);
			
		case CODE_FUNCTION:
			return *((int*) &((CodeFunction*) o)->node);
			
		case STRING: {
			String *s = (String*) o;
			int value = s->parsePositiveInt();
			
			return value == -1 ? s->stringValue()->length : value;
		}
			
		case OBJECT:
			return ((Object*) o)->table->size + 1;
			
		case ARRAY:
			return ((Array*) o)->array->length + 1;
			
		default:
			return 1;
	};
};

double objectDoubleValue(VirtualObject *o) {
	if (o == NULL)
		return 0.0;
	
	try {
		DoubleCastObject *convt = dynamic_cast<DoubleCastObject*>(o);
		if (convt)
			return convt->toDouble();
	} catch(...) {}
	
	switch (o->type) {
		case NONE:
		case TNULL:
		case UNDEFINED:
			return 0;
		
		case SCOPE:
		case PROXY_SCOPE:
			return 1.0;
			
		case INTEGER:
			return (double) ((Integer*) o)->value;
			
		case BOOLEAN:
			return ((Boolean*) o)->value ? 1.0 : 0.0;
			
		case NATIVE_FUNCTION:
			return (double) *((int*) &((NativeFunction*) o)->handler);
			
		case CODE_FUNCTION:
			return (double) *((int*) &((CodeFunction*) o)->node);
			
		case STRING: {
			String *s = (String*) o;
			int value = s->parsePositiveInt();
			
			return value == -1 ? s->stringValue()->length : value;
		}
			
		case OBJECT:
			return ((Object*) o)->table->size + 1;
			
		case ARRAY:
			return ((Array*) o)->array->length + 1;
			
		default:			
			return 1;
	};
};

string objectStringValue(VirtualObject *o) {
	if (o == NULL)
		return string("NULL");
	
	try {
		StringCastObject *convt = dynamic_cast<StringCastObject*>(o);
		if (convt)
			return convt->toString();
	} catch(...) {}
	
	switch (o->type) {
		case NONE:
			return string("NONE");
		case TNULL:
			return string("null");
		case UNDEFINED:
			return string("undefined");
		
		case SCOPE:
			return string("[Scope]");
		case PROXY_SCOPE:
			return string("[ProxyScope]");
		case NATIVE_FUNCTION:
			return string("[NativeFunction(") + string::toString(*((int*) &((NativeFunction*) o)->handler)) + ")]";
		case CODE_FUNCTION:
			return string("[Function]");
			
		case INTEGER: 
			return string::toString(((Integer*) o)->value);
		
		case DOUBLE: 
			return string::toString(((Double*) o)->value);
			
		case BOOLEAN:
			return ((Boolean*) o)->value ? string("true") : string("false");
			
		case STRING: {
			String *s = (String*) o;
			
			if (s->parent) {
				string st;
				int len = s->length + s->offset;
				len = len < s->parent->value->length ? len : s->parent->value->length;
				
				for (int i = s->offset; i < len; ++i)
					st += s->parent->value->buffer[i];
				
				return st;
			}
			
			return *s->value;
		}
		
		case ARRAY: {
			Array *a = (Array*) o;
			string s = "[";
			for (int i = 0; i < a->array->length; ++i) {
				if (a->array->vector[i]->type == ARRAY)
					if (((Array*) a->array->vector[i])->array->length == 0)
						s += "[]";
					else
						s += "[..]";
				else
					s += objectStringValue(a->array->vector[i]);
				
				if (i != a->array->length - 1)
					s += ", ";
			}
			s += "]";
			
			return s;
		}
		
		case OBJECT:
		default: {			
			string key = "__typename";
			VirtualObject *g = o->get(NULL, &key);
			if (g != o) {
				string s = "[";
				s += objectStringValue(g);
				s += "]";
				return s;
			}
			return string("@__typename");
		}
	};
};
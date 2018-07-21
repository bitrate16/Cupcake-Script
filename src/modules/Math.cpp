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

#include <cmath>
#include "../objects/Scope.h"
#include "../objects/Object.h"
#include "../objects/ObjectConverter.h"
#include "../objects/VirtualObject.h"
#include "../objects/Double.h"
#include "../objects/Integer.h"
#include "../objects/NativeFunction.h"
#include "../objects/StringType.h"
#include "../objects/Undefined.h"

#define MATH_PI 3.14159265358979323846
#define RAD2DEG 180.0 / MATH_PI
#define DEG2RAD MATH_PI / 180.0

static VirtualObject* f_sin(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	double v = sin(a);
	return new Double(v);
};

static VirtualObject* f_cos(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	double v = cos(a);
	return new Double(v);
};

static VirtualObject* f_tan(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	double v = tan(a);
	return new Double(v);
};

static VirtualObject* f_acos(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	if (a < -1 || a > 1) {
		scope->context->executer->raiseError("acos argument out of bounds [-1, 1]");
		return new Undefined();
	}
	double v = acos(a);
	return new Double(v);
};

static VirtualObject* f_asin(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	if (a < -1 || a > 1) {
		scope->context->executer->raiseError("asin argument out of bounds [-1, 1]");
		return new Undefined();
	}
	double v = asin(a);
	return new Double(v);
};

static VirtualObject* f_atan(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	double v = atan(a);
	return new Double(v);
};

static VirtualObject* f_atan2(Scope *scope, int argc, VirtualObject **args) {
	if (argc < 2)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	double b = objectDoubleValue(args[1]);
	double v = atan2(a, b);
	return new Double(v);
};

static VirtualObject* f_cosh(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	double v = cosh(a);
	return new Double(v);
};

static VirtualObject* f_sinh(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	double v = sinh(a);
	return new Double(v);
};

static VirtualObject* f_tanh(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	double v = tanh(a);
	return new Double(v);
};

static VirtualObject* f_acosh(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	if (a < 1) {
		scope->context->executer->raiseError("acosh argument out of bounds [1, infty)");
		return new Undefined();
	}
	double v = acosh(a);
	return new Double(v);
};

static VirtualObject* f_asinh(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	double v = asinh(a);
	return new Double(v);
};

static VirtualObject* f_atanh(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	if (a < -1 || a > 1) {
		scope->context->executer->raiseError("atang argument out of bounds [-1, 1]");
		return new Undefined();
	}
	double v = atanh(a);
	return new Double(v);
};

static VirtualObject* f_exp(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	double v = exp(a);
	return new Double(v);
};

static VirtualObject* f_log(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	if (a < 0) {
		scope->context->executer->raiseError("log argument out of bounds [0, infty)");
		return new Undefined();
	}
	double v = log(a);
	return new Double(v);
};

static VirtualObject* f_log2(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	if (a < 0) {
		scope->context->executer->raiseError("log2 argument out of bounds [0, infty)");
		return new Undefined();
	}
	double v = log2(a);
	return new Double(v);
};

static VirtualObject* f_log10(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	if (a < 0) {
		scope->context->executer->raiseError("log10 argument out of bounds [0, infty)");
		return new Undefined();
	}
	double v = log10(a);
	return new Double(v);
};

static VirtualObject* f_pow(Scope *scope, int argc, VirtualObject **args) {
	if (argc < 2)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	double b = objectDoubleValue(args[1]);
	double v = pow(a, b);
	return new Double(v);
};

static VirtualObject* f_sqrt(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	if (a < 0) {
		scope->context->executer->raiseError("sqrt argument out of bounds [0, infty)");
		return new Undefined();
	}
	double v = sqrt(a);
	return new Double(v);
};

static VirtualObject* f_cbrt(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	double v = cbrt(a);
	return new Double(v);
};

static VirtualObject* f_hypot(Scope *scope, int argc, VirtualObject **args) {
	if (argc < 2)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	double b = objectDoubleValue(args[1]);
	double v = hypot(a, b);
	return new Double(v);
};

static VirtualObject* f_ceil(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	int v = ceil(a);
	return new Integer(v);
};

static VirtualObject* f_floor(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	int v = floor(a);
	return new Integer(v);
};

static VirtualObject* f_trunc(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	int v = trunc(a);
	return new Integer(v);
};

static VirtualObject* f_fmod(Scope *scope, int argc, VirtualObject **args) {
	if (argc < 2)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	double b = objectDoubleValue(args[1]);
	double v = fmod(a, b);
	return new Double(v);
};

static VirtualObject* f_abs(Scope *scope, int argc, VirtualObject **args) {
	if (argc == 0)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	double v = abs(a);
	return new Double(v);
};

// clamp(min, max, x)
static VirtualObject* f_clamp(Scope *scope, int argc, VirtualObject **args) {
	if (argc < 3)
		return NULL;
	
	double a = objectDoubleValue(args[0]);
	double b = objectDoubleValue(args[1]);
	double x = objectDoubleValue(args[1]);
	if (x < a)
		return new Double(a);
	if (x > b)
		return new Double(b);
	return new Double(x);
};


static void define_math(Scope *scope) {
	Scope *rootScope = scope->getRoot();
	Object *math = new Object();
	math->table->put(string("__typename"), new String("Math"));
	rootScope->define(string("Math"), math);
	
	math->table->put(string("sin"),    new NativeFunction(&f_sin));
	math->table->put(string("cos"),    new NativeFunction(&f_cos));
	math->table->put(string("tan"),    new NativeFunction(&f_tan));
	math->table->put(string("acos"),   new NativeFunction(&f_acos));
	math->table->put(string("asin"),   new NativeFunction(&f_asin));
	math->table->put(string("atan"),   new NativeFunction(&f_atan));
	math->table->put(string("atan2"),  new NativeFunction(&f_atan2));
	math->table->put(string("cosh"),   new NativeFunction(&f_cosh));
	math->table->put(string("sinh"),   new NativeFunction(&f_sinh));
	math->table->put(string("tanh"),   new NativeFunction(&f_tanh));
	math->table->put(string("acosh"),  new NativeFunction(&f_acosh));
	math->table->put(string("asinh"),  new NativeFunction(&f_asinh));
	math->table->put(string("atanh"),  new NativeFunction(&f_atanh));
	math->table->put(string("exp"),    new NativeFunction(&f_exp));
	math->table->put(string("log"),    new NativeFunction(&f_log));
	math->table->put(string("log2"),   new NativeFunction(&f_log2));
	math->table->put(string("log10"),  new NativeFunction(&f_log10));
	math->table->put(string("pow"),    new NativeFunction(&f_pow));
	math->table->put(string("sqrt"),   new NativeFunction(&f_sqrt));
	math->table->put(string("cbrt"),   new NativeFunction(&f_cbrt));
	math->table->put(string("hypot"),  new NativeFunction(&f_hypot));
	math->table->put(string("ceil"),   new NativeFunction(&f_ceil));
	math->table->put(string("floor"),  new NativeFunction(&f_floor));
	math->table->put(string("trunc"),  new NativeFunction(&f_trunc));
	math->table->put(string("fmod"),   new NativeFunction(&f_fmod));
	math->table->put(string("abs"),    new NativeFunction(&f_abs));
	
	math->table->put(string("PI"),      new Double(MATH_PI));
	math->table->put(string("RAD2DEG"), new Double(RAD2DEG));
	math->table->put(string("DEG2RAD"), new Double(DEG2RAD));
};

#ifdef __cplusplus
extern "C" {
#endif

// Called on module being loaded by bla bla bla
void onLoad(Scope *scope, int argc, VirtualObject **args) {
	define_math(scope);
};

// Called on module being unloaded on program exit
void onUnload() {};

#ifdef __cplusplus
}
#endif


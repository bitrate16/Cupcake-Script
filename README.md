Cupcake Script interpreter
==========================

![](https://github.com/bitrate16/Cupcake-Script/blob/master/icon.png)

- Custom scripting language written in C++.
- Fully supported on linux, unicode only on Windows.
- Supports operators overloading.
- Object-oriented language.
- Support native C++ modules, customizable objects & functions.
- Supports Streams, FIles & Math library.

Table of Contents
=================

* [Cupcake Script interpreter](#cupcake-script-interpreter)
* [Installation](#installation)
* [Modules](#modules)
* [Object types](#object-types)
* [Syntax](#syntax)
* [Operators](#operators)
* [Additional review](#additional-review)
* [TODO](#todo)
* [TOTypes list](#types-list)

Installation
============

```
git clone https://github.com/bitrate16/Cupcake-Script
cd Cupcake-Script
bash compile.sh install
```

Modules
=======

Write your own module in C++ and put it into `src/modules`
Compile with
```
bash buildso.sh src/modules/MyModule.cpp bin/MyModule.so
```
And load it in cupcake source code
```
NativeLoader.load('bin/MyModule.so');
```

Object types
============

`Array`
```
var arr = ['foo', 12, 'bar']
```
`Object`
```
var obj = {'foo': 14, 'bar': [1, 3, 5], duck: 'krya'}
```
`Integer`
```
var i = 12
```
`Double`
```
var d = 12.4E-13
```
`String`
```
var s = 'my string'
// or
var s = "my fat string"
```
`Function`
```
var f = function(x) { return 13 + x }
```
`Error`
Raised on any kind of errors during execution
```
raise Error('Shit happens')
```
`Undefined`
Returned in any case of undefined state like division by zero
```
var u = undefined
```
`Null`
```
var n = null
```
`Scope`
Widely used obejct type in any execution case
```
var scope = this
```

Syntax
======

`for (<initialization>; <condition>; <increment>) <block>`

`if <condition> <block>`

`while <condition> <block>`

`do <block> while <condition>`

`raise <error expression>`

`return <expression>`

`break`

`continue`

`try <block> [expect [<handler name>] <block>]`

`var <name> [= <expression>]{, <name> [= <expression>]}`

`this` refers to closest object 
`func = function() { return this }; var t = obj.func()`
`t` is containing `obj`

`self` refers to closest scope 
`func = function() { return this }; var t = obj.func()`
`t` is containing `function.__scope`

Operators
=========

`==, !=, >, <, >=, <=, +, -,*, /, %, <<, >>,`
`>>>, &, |, &&, ||, ^, !x, ~x, -x, +x, ++, --`

Types list
==========

* General
	* Integer
	* Double
	* Boolean
	* Object
	* Array
	* String
	* CodeFunction
	* NativeFunction
	* Scope
	* ProxyScope
	* Null
	* Undefined
	* Error
	* NativeLoader
	* VirtualObject
* Additional
	* InputStream
	* OutputStream
	* FileInputStream
	* FileOutputStream
	* File
	* Char
	* Byte
	* Scanner
	* Printer
	* BytePtr

Additional review
=================

[check review](https://youtu.be/dQw4w9WgXcQ "check review")

TODO
====

`XXX: Finish readme, add wiki`

`XXX: popen implementation with streams`

`XXX [+]: allow passing arguments to the script file`

`XXX: delete <ref_expr> statement`

`XXX: define Char type in String`

`XXX [+]: zero character`

`XXX: string.__operator compare [>/>=/</<=/==/!=] with integer/char types`

`XXX: string.__operator# convert string to char`

`XXX: collection structure: (<exp>[, <exp>])`

`XXX: return collection`

`XXX: assign collection to multiple variables: var a, b, c = (13, 'kek', ['lol', 'pek', 'cheburek'])`

`XXX: sockets implementation`

`XXX: implement support for __operator# and others`

`XXX: string: .setcharcode, .getcharcode, .setchar, .getchar, __operator+ char, __operator# -> array of characters`

`XXX: bitwise looped shift: @> / <@`

`XXX: list generators`

`XXX: list iterator loop`

`XXX: GIL like thread synchronizing system`

`XXX: Combine Parser and Tokenizer`

`XXX [+]: Save object type extendtion hierarchy in object:`
```
struct TypeIdList {
  int type;
  TypeList parent;
  
  TypeIdList(TypeList parent, int type) {
    this->type = type;
    this->parent = parent;
  }
  
  TypeIdList(int type) {
    this->type = type;
    this->parent = NULL;
  }
};

struct VirtualObject {
  static const int type = VIRTUAL_OBJECT;
  statis const TypeIdList parentType = NULL;
  
  // ...
};

struct Scope : VirtualObject {
  static const int type = SCOPE;
  static const TypeIdList parentType = TypeIdList(VirtualObject::parentType, Scope::type);
  
  // ...
};
```

`XXX: Memory allocation control`

`XXX: BigInteger, BigDouble`

`XXX: TreeObjectMap extends abstract ObjectMap`

`XXX: GarbageCollector object allocation control`

`XXX: CodeFunction, NativeFunction extends Function`

`XXX: CodeFunction::apply`

`XXX: Simplify error rising`

`XXX: import instead of NativeLoader and System.import`

`XXX: typles, typle arguments, tuple assignment, tuple packing/unpacking`

`XXX: ck console`

`XXX: default argument values for funtions, selectable arguments assignment`

`XXX: function call cope points to function`

`XXX: generators with yield/list generators`

`XXX: fucntion call with object:`
```
f(12, "foo") { bar: ['tar', 'sar'], par: -1 }
```

`XXX: Runtime.__exitHandler`

`XXX: VirtualObject.__callable`

`XXX: eval`

`XXX: Token namespace encapsulation`

`XXX: using(x) ~ with(x) - unpack object to scope`

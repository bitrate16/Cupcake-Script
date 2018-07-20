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


#ifndef FILES_H
#define FILES_H

#include "StreamApi.h"
#include "../FileUrl.h"
#include "../VectorArray.h"
#include "../objects/VirtualObject.h"

// Windows definition section
#ifdef _WIN32
#define pathSeparator     "\\"
#define pathSeparatorChar '\\'
#define _FILE_LOC_LINUX      0      


// Linux definition section
#elif defined linux
#define pathSeparator     "/"
#define pathSeparatorChar '/'
#define _FILE_LOC_LINUX     1


// Default section
#else
#define pathSeparator     "/"
#define pathSeparatorChar '/'


#endif

#define winPathSeparatorChar   '\\'
#define linuxPathSeparatorChar  '/'
// #define _FILE_PLATFORM_INDEPENDENT

#define FILETYPE                   49
#define FILE_PROTOTYPE             50
#define FILEINPUTSTREAM            51
#define FILEINPUTSTREAM_PROTOTYPE  52
#define FILEOUTPUTSTREAM           53
#define FILEOUTPUTSTREAM_PROTOTYPE 54
#define SCANNER                    55
#define SCANNER_PROTOTYPE          56
#define PRINTER                    57
#define PRINTER_PROTOTYPE          58
#define CHAR_TYPE                  59
#define CHAR_TYPE_PROTOTYPE        60
#define LONGTYPE                   61
#define LONGTYPE_PROTOTYPE         62


// - - - - - - - - - - - - - - - - - - - - - - - L O N G

// Long prototype's prototype	
struct LongPrototype : Object {
	LongPrototype();
	void finalize(void);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	VirtualObject *call(Scope*, int, VirtualObject**);
	void mark(void);
};

// Long type's prototype
struct Long : VirtualObject, IntCastObject, StringCastObject, DoubleCastObject {
	long value;
	Long();
	Long(long);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	VirtualObject *call(Scope*, int, VirtualObject**);
	
	long toInt();
	double toDouble();
	string toString();
};

// Called on start. Defines long prototype & type
void define_long(Scope*);

extern LongPrototype *long_prototype;


// - - - - - - - - - - - - - - - - - - - - - - - C H A R

// Char prototype's prototype	
struct CharPrototype : Object {
	CharPrototype();
	VirtualObject *call(Scope*, int, VirtualObject**);
};

// Char type's prototype
struct Char : VirtualObject, IntCastObject, StringCastObject, DoubleCastObject {
	wchar_t value;
	Char();
	Char(char);
	Char(wchar_t);
	Char(int);
	Char(long);
	VirtualObject *get(Scope*, string*);
	void put(Scope*, string*, VirtualObject*);
	void remove(Scope*, string*);
	bool contains(Scope*, string*);
	VirtualObject *call(Scope*, int, VirtualObject**);
	
	long toInt();
	double toDouble();
	string toString();
};

// Called on start. Defines integer prototype & type
void define_char(Scope*);

extern CharPrototype *char_prototype;


// - - - - - - - - - - - - - - - - - - - - - - - F I L E

struct File : Object, IntCastObject, StringCastObject, DoubleCastObject {
	// Context scope of the file. Used for constructing correct path
	Scope *scope;
	// Path of the file
	string path;
	
	// Initializes new file with given path.
	File(string path);
	// Initializes new file with path:
	// path/subpath
	File(string path, string subpath);
	// Constructs new file in the same way
	File(string path, File *subpath);
	// Constructs new file in the same way & assigns scope
	// of the path file to this file
	File(File *path, string subpath);
	// Constructs new file in the same way
	File(File *path, File *subpath);
	
	void attachScope(Scope *scope);
	string getAbsolutePath();
	File *getAbsoluteFile();
	string getPath();
	string getFileName();
	string getParentFilePath();
	File *getParentFile();
	bool exists();
	bool isDirectory();
	// Uses malloc. Returns list of files in directory.
	VectorArray<VirtualObject> *listFiles();
	// Attempts to create new file.
	// Retuns 1 if success, 0 else.
	int createNewFile();
	int deleteFile();
	int createDirectory();
	long getSize();
	
	void mark();
	
	long toInt();
	string toString();
	double toDouble();
};

struct FilePrototype : Object {
	FilePrototype();
	VirtualObject *call(Scope*, int, VirtualObject**);
};

extern FilePrototype *file_prototype;


// - - - - - - - - - - - - - - - - - - - - - - - F I L E _ I N P U T _ S T R E A M

struct FileInputStream : ObjectInputStream {
	File *file;
	FILE *cfile;
	char *cpath;
	
	FileInputStream(File *f);
	FileInputStream(string path);
	
	// Set scope for the file
	void attachScope(Scope *scope);
	
	// Attempting to opena file with given path. 
	// Returns 1 if ok, 0 else
	int open();

	void mark();
	
	void finalize();
};

struct FileInputStreamPrototype : Object {
	Scope *def_scope;
	FileInputStreamPrototype();
	VirtualObject *call(Scope*, int, VirtualObject**);
};

extern FileInputStreamPrototype *file_input_stream_prototype;


// - - - - - - - - - - - - - - - - - - - - - - - F I L E _ O U T P U T _ S T R E A M

struct FileOutputStream : ObjectOutputStream {
	File *file;
	FILE *cfile;
	char *cpath;
	
	FileOutputStream(File *file);
	FileOutputStream(string path);
	
	// Set scope for the file
	void attachScope(Scope *scope);
	
	// Attempting to opena file with given path. 
	// Returns 1 if ok, 0 else
	int open();

	void mark();
	
	void finalize();
};

struct FileOutputStreamPrototype : Object {
	Scope *def_scope;
	FileOutputStreamPrototype();
	VirtualObject *call(Scope*, int, VirtualObject**);
};

extern FileOutputStreamPrototype *file_output_stream_prototype;
	
	
// - - - - - - - - - - - - - - - - - - - - - - - S C A N N E R

struct Scanner : FileInputStream {
	Scanner(File *f);
	Scanner(string path);
};

struct ScannerPrototype : Object {
	Scope *def_scope;
	ScannerPrototype();
	VirtualObject *call(Scope*, int, VirtualObject**);
};

extern ScannerPrototype *scanner_prototype;


// - - - - - - - - - - - - - - - - - - - - - - - P R I N T _ W R I T E R

struct Printer : FileOutputStream {
	Printer(File *file);
	Printer(string path);
};

struct PrinterPrototype : Object {
	Scope *def_scope;
	PrinterPrototype();
	VirtualObject *call(Scope*, int, VirtualObject**);
};

extern PrinterPrototype *printer_prototype;
	
	
void define_fios(Scope *scope);

#endif
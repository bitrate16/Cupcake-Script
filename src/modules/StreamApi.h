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


#ifndef STREAM_API
#define STREAM_API

#include "../objects/Object.h"
#include "../objects/Scope.h"
#include "../objects/TreeObjectMap.h"

#include "../TokenNamespace.h"
#include "../ASTExecuter.h"
#include "../string.h"

// EOF marker.
#define SEOF -1

// Additional typecodes
#define BYTE_TYPE                    41
#define BYTE_TYPE_PROTOTYPE          42
#define BYTEPTR                      43
#define BYTEPTR_PROTOTYPE            44
#define OBJECTINPUTSTREAM            45
#define OBJECTINPUTSTREAM_PROTOTYPE  46
#define OBJECTOUTPUTSTREAM           47
#define OBJECTOUTPUTSTREAM_PROTOTYPE 48

// - - - - - - - - - - - - - - - - - - - - B Y T E _ T Y P E

// Byte prototype's prototype	
struct BytePrototype : Object {
	BytePrototype();
	VirtualObject *call(Scope*, int, VirtualObject**);
};

// Byte type's prototype
struct Byte : VirtualObject, IntCastObject, StringCastObject, DoubleCastObject {
	char value;
	Byte();
	Byte(int);
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
void define_byte(Scope*);

extern BytePrototype *byte_prototype;


// - - - - - - - - - - - - - - - - - - - - I N P U T _ S T R E A M

// Prototype for Input/Output Streams.
struct InputStream {
	// Reads single byte from InputStream.
	// Returns -1 if stream reaches EOF.
	virtual int read();
	
	// Read input bytes into buffer. 
	// Reurns amount of read, -1 if error occurred.
	virtual int read(char *buff, int off, int len);
	
	// Read input bytes into buffer.
	virtual int read(char *buff, int len);
	
	// Returns 1 whenewer EOF is reached.
	virtual int eof();
	
	// Closes InputStream
	virtual int close();
};

struct OutputStream {
	// Writes single byte into InputStream.
	virtual int write(char b);
	
	// Writes bytes into buffer. 
	virtual int write(char *buff, int off, int len);
	
	// Writes bytes into buffer.
	virtual int write(char *buff, int len);
	
	// Closes OutputStream
	virtual int close();
	
	// Push bytes into OutputStream
	virtual int flush();
};


// - - - - - - - - - - - - - - - - - - - - S O U R C E _ I N P U T _ S T R E A M

enum OISFuncId {
	OISF_READ,
	OISF_WRITE,
	OISF_EOF,
	OISF_FLUSH,
	OISF_CLOSE,
	OISF_NONE
};

// Output stream based on calling objects as Functions.
struct ObjectInputStream : Object, InputStream {
	ASTExecuter localExecuter;
	Scope         *readScope, *eofScope, *closeScope;
	VirtualObject  *readFunc,  *eofFunc,  *closeFunc;
	// Id of assigned StackTrace. 
	OISFuncId assignedTraceId;
	
	ObjectInputStream();
	
	// Allows easier assignment of fields without calling put
	void setReadFunc(Scope *scope, VirtualObject *f);
	void setEofFunc(Scope *scope, VirtualObject *f);
	void setCloseFunc(Scope *scope, VirtualObject *f);
	
	void put(Scope *scope, string *name, VirtualObject *value);
	void remove(Scope *scope, string *name);
	VirtualObject *call(Scope *scope, int argc, VirtualObject **args);
	
	void mark();
	
	int read();
	int read(char *buff, int off, int len);
	int read(char *buff, int len);
	int eof();
	int close();
};

// ObjectInputStream prototype
struct ObjectInputStreamPrototype : Object {
	ObjectInputStreamPrototype();
	VirtualObject *call(Scope*, int, VirtualObject**);
};

// Input stream based on calling objects as Functions.
struct ObjectOutputStream : Object, OutputStream {
	ASTExecuter localExecuter;
	Scope         *writeScope, *closeScope, *flushScope;
	VirtualObject  *writeFunc,  *closeFunc,  *flushFunc;
	// Id of assigned StackTrace. 
	OISFuncId assignedTraceId;
	
	ObjectOutputStream();
	
	// Allows easier assignment of fields without calling put
	void setWriteFunc(Scope *scope, VirtualObject *f);
	void setCloseFunc(Scope *scope, VirtualObject *f);
	void setFlushFunc(Scope *scope, VirtualObject *f);
	
	void put(Scope *scope, string *name, VirtualObject *value);
	void remove(Scope *scope, string *name);
	VirtualObject *call(Scope *scope, int argc, VirtualObject **args);
	
	void mark();
	
	int write(char b);
	int write(char *buff, int off, int len);
	int write(char *buff, int len);
	int close();
	int flush();
};

// ObjectOutputStream prototype
struct ObjectOutputStreamPrototype : Object {
	ObjectOutputStreamPrototype();
	VirtualObject *call(Scope*, int, VirtualObject**);
};


// Called on start. Defines Input/Output Stream's prototype & type
void define_oio(Scope*);

extern ObjectInputStreamPrototype *OIstream_prototype;

extern ObjectOutputStreamPrototype *OOstream_prototype;

// - - - - - - - - - - - - - - - - - - - - B Y T E _ P T R _ T Y P E

enum SAPTRALLOCTYPE {
	SAPTR_NEW,
	SAPTR_ALLOC,
	SAPTR_UNDEF
};

// Pointer to BYTE sructure
struct BytePtr : Object {
	SAPTRALLOCTYPE allocType;
	char   *ptr;
	size_t size;
	
	// malloc some memory
	BytePtr(size_t size);
	// Default for malloc
	BytePtr(char *ptr, size_t size);
	// For any pointer type
	BytePtr(char *ptr, size_t size, SAPTRALLOCTYPE allocType);
	
	VirtualObject *get(Scope *scope, string *name);
	void put(Scope *scope, string *name, VirtualObject *value);
	VirtualObject *call(Scope *scope, int argc, VirtualObject **args);
	
	void finalize();
};

// Pointer to BYTE prototype's sructure
struct BytePtrPrototype : Object {
	BytePtrPrototype();
	VirtualObject *call(Scope*, int, VirtualObject**);
};

void define_byteptr(Scope*);

extern BytePtrPrototype *BytePtr_prototype;



#endif
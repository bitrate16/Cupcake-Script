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
 * This class defines Garbage Collector used over all VM Objects.
 * GC method is mark and <strike>sweep</strike> free.
 */

#ifndef GARBAGE_COLLECTOR_H
#define GARBAGE_COLLECTOR_H

#include <stdlib.h>

// #define GC_DEBUG
// #define GC_FULL_DEBUG

struct GC_Object;

struct GC_Chain {
	GC_Object     *object;
	GC_Chain        *next;
	// For stupid users, that decide to GC by themself & stack
	bool      deleted_ptr;
	
	GC_Chain();
};

struct GC_Object {
	bool     gc_record;
	bool       gc_root;
	bool  gc_reachable;
	bool       gc_lock;
	GC_Chain *gc_chain;
	GC_Chain *gc_lock_chain;
	GC_Chain *gc_root_chain;
	
	GC_Object();
	
	virtual ~GC_Object();
	
	// Called when GC indexes all reachable objects
	virtual void mark();
	
	// Called when GC destroyes current object
	virtual void finalize();
};

// Автаматик обжэкт флов контрол

struct GarbageCollector {
	// Prevent recursive garbage collection.
	int    gc_collecting;
	int          gc_size;
	int          gc_roots_size;
	GC_Chain   *gc_roots;
	GC_Chain   *gc_locks;
	GC_Chain *gc_objects;
	
	GarbageCollector();
	
	~GarbageCollector();
	
	// Attach an object to this instance of garbage collector.
	GC_Object *gc_attach(GC_Object *o);
	
	// Lock object from deletion on GC.collect.
	GC_Object *gc_lock(GC_Object *o);
	
	GC_Object *gc_unlock(GC_Object *o);
	
	// Attach object as reference root.
	GC_Object *gc_attach_root(GC_Object *o);
	
	void gc_deattach_root(GC_Object *o);
	
	void gc_collect(void);
	
	void gc_dispose(void);
};

// Global instance of the GarbageCollector
extern GarbageCollector GC;

#endif

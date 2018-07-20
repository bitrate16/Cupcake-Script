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


#include <sys/time.h>

#include "GarbageCollector.h"
#include "objects/VirtualObject.h"

#if defined GC_DEBUG || defined GC_FULL_DEBUG
#include <stdio.h>
#endif

GarbageCollector GC;

// GC_Chain
GC_Chain::GC_Chain() {
	next        = NULL;
	object      = NULL;
	deleted_ptr = 0;
};


// GC_Object
GC_Object::GC_Object() {
	gc_record     = 0;
	gc_root       = 0;
	gc_lock       = 0;
	gc_reachable  = 0;
	gc_chain      = NULL;
	gc_root_chain = NULL;
	gc_lock_chain = NULL;
	GC.gc_attach(this);
};

GC_Object::~GC_Object() {
	if (gc_chain)
		gc_chain->deleted_ptr = 1;
	if (gc_root_chain)
		gc_root_chain->deleted_ptr = 1;
	if (gc_lock_chain)
		gc_lock_chain->deleted_ptr = 1;
};

// Called when GC indexes all reachable objects
void GC_Object::mark() {
	gc_reachable = 1;
};

// Called when GC destroyes current object
void GC_Object::finalize() {};

// GarbageCollector
GarbageCollector::GarbageCollector() {
	gc_size    = 0;
	gc_roots   = NULL;
	gc_locks   = NULL;
	gc_objects = NULL;
	gc_collecting = 0;
};

GarbageCollector::~GarbageCollector() {
	GC_Chain *chain = gc_roots;
	while (chain) {			
		GC_Chain *tmp = chain;
		chain = chain->next;
		delete tmp;
	}
	
	chain = gc_objects;
	while (chain) {			
		GC_Chain *tmp = chain;
		chain = chain->next;
		delete tmp;
	}
	
	chain = gc_locks;
	while (chain) {			
		GC_Chain *tmp = chain;
		chain = chain->next;
		delete tmp;
	}
};

GC_Object *GarbageCollector::gc_attach(GC_Object *o) {
	if (!o)
		return o;
	
	if (o->gc_record)
		return o;
	
	GC_Chain *chain = new GC_Chain;
	if (!chain)
		return o;

#ifdef GC_FULL_DEBUG
	printf("GC_attach (0x%08x)\n",  ((int) o));
#endif
	
	o->gc_record    = 1;
	o->gc_reachable = 0;
	o->gc_chain     = chain;
	chain->next     = gc_objects;
	chain->object   = o;
	gc_objects      = chain;
	++gc_size;
	return o;
};

GC_Object *GarbageCollector::gc_lock(GC_Object *o) {
	if (!o)
		return o;
	
	if (o->gc_root)
		return o;
	
	GC_Chain *chain = new GC_Chain;
	if (!chain)
		return o;

#if defined GC_FULL_DEBUG
	printf("GC_lock (0x%08x)\n",  ((int) o));
#endif
	
	o->gc_lock       = 1;
	o->gc_reachable  = 0;
	o->gc_lock_chain = chain;
	chain->next      = gc_locks;
	chain->object    = o;
	gc_locks         = chain;
	return o;
};

GC_Object *GarbageCollector::gc_unlock(GC_Object *o) {
	if (!o)
		return o;
	
	if (!o->gc_lock)
		return o;

#ifdef GC_FULL_DEBUG
	printf("GC_unlock (0x%08x)\n",  ((int) o));
#endif
	
	o->gc_lock = 0;
	return o;
};

GC_Object *GarbageCollector::gc_attach_root(GC_Object *o) {
	if (!o)
		return o;
	
	if (o->gc_root)
		return o;
	
	GC_Chain *chain = new GC_Chain;
	if (!chain)
		return o;

#if defined GC_FULL_DEBUG
	printf("GC_attach_root (0x%08x)\n",  ((int) o));
#endif
	
	o->gc_root       = 1;
	o->gc_reachable  = 0;
	o->gc_root_chain = chain;
	chain->next      = gc_roots;
	chain->object    = o;
	gc_roots         = chain;
	gc_attach(o);
	return o;
};

void GarbageCollector::gc_deattach_root(GC_Object *o) {
	if (!o)
		return;
	
	if (!o->gc_root)
		return;

#ifdef GC_FULL_DEBUG
	printf("GC_deattach_root (0x%08x)\n",  ((int) o));
#endif
	
	o->gc_root = 0;
	return;
};

void GarbageCollector::gc_collect(void) {
	if (gc_collecting)
		return;
	
	gc_collecting = 1;

#if defined GC_DEBUG || defined GC_FULL_DEBUG
	struct timeval tp;
	gettimeofday(&tp, NULL);
	long int ms_start  = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	int deleted_amount = 0;
	int dead_amount    = 0;
	int lock_amount    = 0;
	int total_amount   = gc_size;
#endif

	if (!gc_roots) {
		
#if defined GC_DEBUG || defined GC_FULL_DEBUG
		printf("GC_collect problem: no roots\n");
#endif

	}
	
	if (!gc_objects) {
		
#if defined GC_DEBUG || defined GC_FULL_DEBUG
		printf("GC_collect aborted: no objects\n");
#endif

		gc_collecting = 0;
		return;
	}
	
	// Mark all roots & referenced objects
	
	GC_Chain *chain = gc_roots;
	GC_Chain *list  = NULL;
	while (chain) {
		if (chain->deleted_ptr) {
			GC_Chain *tmp = chain;
			chain         = chain->next;
			delete tmp;
		} else if (!chain->object->gc_root) {
			gc_attach(chain->object);
			chain->object->gc_root_chain = NULL;
			
			GC_Chain *tmp = chain;
			chain         = chain->next;
			delete tmp;
		} else {
			chain->object->mark();
			GC_Chain *tmp = chain;
			chain         = chain->next;
			
			tmp->next = list;
			list      = tmp;
		}
	}
	
	gc_roots = list;
	
	chain = gc_locks;
	list  = NULL;
	while (chain) {
		if (chain->deleted_ptr) {
			GC_Chain *tmp = chain;
			chain         = chain->next;
			delete tmp;
		} else if (!chain->object->gc_lock) {
			gc_attach(chain->object);
			chain->object->gc_lock_chain = NULL;
			
			GC_Chain *tmp = chain;
			chain         = chain->next;
			delete tmp;
		} else {
			
#if defined GC_DEBUG || defined GC_FULL_DEBUG
				++lock_amount;
#endif

			chain->object->mark();
			GC_Chain *tmp = chain;
			chain         = chain->next;
			
			tmp->next = list;
			list      = tmp;
		}
	}
	
	gc_locks = list;
	
	chain = gc_objects;
	list  = NULL;
	
	while (chain) {	
		if (chain->deleted_ptr) {
			GC_Chain *tmp = chain;
			chain = chain->next;
			--gc_size;
			
#if defined GC_DEBUG || defined GC_FULL_DEBUG
			++dead_amount;
#endif
			
			delete tmp;
		} else if (!chain->object->gc_reachable && !chain->object->gc_root && !chain->object->gc_lock) {			
			GC_Chain *tmp = chain;
			chain = chain->next;
			--gc_size;
		
#ifdef GC_DEBUG
			++deleted_amount;
#endif
			
			// printf("----------> deleting (0x%08x)", tmp);
			// printf(" (0x%08x)",  tmp->object);
			// int ptr = *(int*)(void*) &tmp->object->finalize;
			// // static_cast<int>(tmp->object->finalize)
			// printf(" (0x%08x)\n",  ptr);
			// tmp->object->test();
			tmp->object->finalize();

			delete tmp->object;
			delete tmp;			
		} else {
			// Reset
			chain->object->gc_reachable = 0;
			GC_Chain *tmp = chain;
			chain         = chain->next;
			
			tmp->next = list;
			list      = tmp;
		}
	};
	
	gc_objects = list;
	
	
#if defined GC_DEBUG || defined GC_FULL_DEBUG
	gettimeofday(&tp, NULL);
	long int ms_end = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	printf("GC_collect %d object(s) (%d dead) (%d locked) from total %d object(s) for %ld ms\n", deleted_amount, dead_amount, lock_amount, total_amount, ms_end - ms_start);
#endif

	gc_collecting = 0;	
};

void GarbageCollector::gc_dispose(void) {	
	if (gc_collecting)
		return;
	
	gc_collecting = 1;

#ifdef GC_DEBUG
	struct timeval tp;
	gettimeofday(&tp, NULL);
	long int ms_start        = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	int deleted_amount       = 0;
	int dead_amount          = 0;
	int total_amount         = gc_size;
#endif
	
	if (!gc_objects) {
		
#ifdef GC_DEBUG
	printf("GC_dispose aborted: no objects\n");
#endif

		gc_collecting = 0;
		return;
	}
	
	while (gc_roots) {
		GC_Chain *tmp = gc_roots->next;
		if (!gc_roots->deleted_ptr)
			gc_roots->object->gc_root_chain = NULL;
		delete gc_roots;
		gc_roots = tmp;
	}
	
	
	// Delete all unused objects
	while (gc_objects) {
		GC_Chain *tmp = gc_objects->next;
		if (gc_objects->deleted_ptr) {
			
#ifdef GC_DEBUG
			++dead_amount;
#endif

			delete gc_objects;
		} else {
			--gc_size;
			
#ifdef GC_DEBUG
			++deleted_amount;
#endif
			
			gc_objects->object->finalize();
			
			delete gc_objects->object;
			delete gc_objects;
		} 
		gc_objects = tmp;
	}

#ifdef GC_DEBUG
	gettimeofday(&tp, NULL);
	long int ms_end = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	printf("GC_dispose %d object(s) (%d dead) from total %d object(s) for %ld ms\n", deleted_amount, dead_amount, total_amount, ms_end - ms_start);
#endif
	
	gc_collecting = 0;
};


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
 * @Deprecated.
 * Linked Map for easier storing VM objects.
 */

#ifndef LINKED_OBJECT_MAP
#define LINKED_OBJECT_MAP

#include "Scope.h"
#include "Array.h"
#include "StringType.h"

// Linked Map for storing objects

#define DEFAULT_MAX_SIZE     4096

template<typename K>
struct LinkedObjectMapEntry {
	LinkedObjectMapEntry<K>  *next;
	K                 key;
	VirtualObject*  value;
	
	LinkedObjectMapEntry() {
		next  = NULL;
		value = NULL;
	};
};

template<typename K>
struct LinkedObjectMap {
	int                         size;
	LinkedObjectMapEntry<K> *entries;
	
	LinkedObjectMap() {
		size      = 0;
		entries   = NULL;
	};
	
	LinkedObjectMap(LinkedObjectMap *map) {
		size      = 0;
		entries   = NULL;
		putAll(map);
	};
	
	~LinkedObjectMap() {
		while (entries) {
			LinkedObjectMapEntry<K> *e = entries->next;
			delete entries;
			entries = e;
		}
	};
	
	void finalize() {
		while (entries) {
			LinkedObjectMapEntry<K> *e = entries->next;
			delete entries;
			entries = e;
		}
		
		delete(this);
	};
	
	int putAll(LinkedObjectMap<K> *map) {
		if (map == NULL)
			return 0;
		
		if (map->size == 0)
			return 1;
		
		LinkedObjectMapEntry<K> *e = map->entries;
		while (e) {
			put(e->key, e->value);
			e = e->next;
		}
	};
	
	int put(K key, VirtualObject* value) {
		if (value == NULL)
			return 0;
		
		LinkedObjectMapEntry<K> *e = entries;
		
		while (e)
			if (e->key == key) {
				if (e->value == value)
					return 1;
				
				e->value = value;
				return 1;
			} else 
				e = e->next;
		
		e = new LinkedObjectMapEntry<K>;
		e->key   = key;
		e->value = value;
		e->next  = entries;
		entries  = e;
		
		++size;
		return 1;
	};
	
	int contains(K key) {
		LinkedObjectMapEntry<K> *e = entries;
		
		while (e)
			if (e->key == key)
				return 1;
			else
				e = e->next;
			
		return 0;
	};
	
	LinkedObjectMapEntry<K> *findEntry(K key) {
		LinkedObjectMapEntry<K> *e = entries;
		
		while (e)
			if (e->key == key)
				return e;
			else
				e = e->next;
			
		return NULL;
	};
	
	VirtualObject* get(K key) {
		LinkedObjectMapEntry<K> *e = entries;
		
		while (e)
			if (e->key == key)
				return e->value;
			else
				e = e->next;
		return NULL;
	};
	
	VirtualObject* remove(K key) {
		
		if (entries == NULL)
			return NULL;
		
		LinkedObjectMapEntry<K> *e;
		if (entries->key == key) {
			VirtualObject* value = entries->value;
			
			e = entries->next;
			delete entries;
			entries = e;
			
			return value;
		}
		
		e = entries;
		while (e->next)
			if (e->next->key == key) {
				VirtualObject* value = e->next->value;
				
				LinkedObjectMapEntry<K> *f = e->next->next;
				delete e->next;
				e->next = f;
				
				return value;
			}
			
		return NULL;
	};

	void mark(void) {
		LinkedObjectMapEntry<string> *e = entries;
		
		while (e) {
			if (e->value && !e->value->gc_reachable && !e->value->gc_root) 
				e->value->mark();
			
			e = e->next;
		}
	};
	
	void keys(Array *a) {
		LinkedObjectMapEntry<string> *e = entries;
		
		while (e) {
			a->array->push(new String(e->key));
			
			e = e->next;
		}
	};
};

#endif
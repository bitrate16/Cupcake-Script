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

#ifndef MAP_H
#define MAP_H

#include <stdlib.h>

#define DEFAULT_MAX_SIZE     4096

template<typename K, typename V>
struct LinkedMapEntry {
	LinkedMapEntry<K, V>  *next;
	K                       key;
	V                     value;
	
	LinkedMapEntry() {
		next  = NULL;
		value = NULL;
	};
	
	~LinkedMapEntry() {
		
	};
};

template<typename K, typename V>
struct LinkedMap {
	int                      size;
	LinkedMapEntry<K, V> *entries;
	
	LinkedMap() {
		size      = 0;
		entries   = NULL;
	};
	
	LinkedMap(LinkedMap *map) {
		if (map == NULL) {
			size      = 0;
			entries   = NULL;
		} else {
			size      = map->size;
			entries   = NULL;
			putAll(map);
		}
	};
	
	~LinkedMap() {
		while (entries) {
			LinkedMapEntry<K, V> *e = entries->next;
			delete entries;
			entries = e;
		}
	};
	
	int putAll(LinkedMap *map) {
		if (map == NULL)
			return 0;
		
		if (map->size == 0)
			return 1;
		
		for (int i = 0; i < map->size; ++i)
			put(map->entries[i]->key, map->entries[i]->value);
	};
	
	int put(K key, V value) {
		LinkedMapEntry<K, V> *e = entries;
		printf("PUT\n");
		while (e)
			if (e->key == key) {
				entries->value = value;
				return 1;
			} else 
				e = e->next;
		
		printf("NEW INSERTED\n");
		e = new LinkedMapEntry<K, V>;
		e->key   = key;
		e->value = value;
		e->next  = entries;
		entries  = e;
		
		++size;
		return 1;
	};
	
	int contains(K key) {
		LinkedMapEntry<K, V> *e = entries;
		
		while (e)
			if (e->key == key)
				return 1;
			else
				e = e->next;
			
		return 0;
	};
	
	LinkedMapEntry<K, V> *findEntry(K key) {
		LinkedMapEntry<K, V> *e = entries;
		
		while (e)
			if (e->key == key)
				return e;
			else
				e = e->next;
			
		return NULL;
	};
	
	V get(K key) {
		LinkedMapEntry<K, V> *e = entries;
		
		while (e)
			if (e->key == key)
				return entries->value;
			else
				e = e->next;
		return NULL;
	};
	
	V remove(K key) {
		
		if (entries == NULL)
			return NULL;
		
		LinkedMapEntry<K, V> *e;
		if (entries->key == key) {
			V value = entries->value;
			
			e = entries->next;
			delete entries;
			entries = e;
			
			return value;
		}
		
		e = entries;
		while (e->next)
			if (e->next->key == key) {
				V value = e->next->value;
				
				LinkedMapEntry<K, V> *f = e->next->next;
				delete e->next;
				e->next = f;
				
				return value;
			}
			
		return NULL;
	};
};

#endif
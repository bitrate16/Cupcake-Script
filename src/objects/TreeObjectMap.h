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
 * Tree Map for storing objects
 * By default used instead of LinkedObjectMap.
 *
 * <copy>
 * Following source code for balancing trees
 * on insertation/removal was found on
 * (https://gist.github.com/Harish-R/097688ac7f48bcbadfa5)
 */

#ifndef TREE_OBJECT_MAP
#define TREE_OBJECT_MAP

#include "VirtualObject.h"
#include "../string.h"

// Tree Map for storing objects
// By default used instead of LinkedObjectMap

#define DEFAULT_MAX_SIZE     4096

struct Array;

struct TreeObjectMapEntry {
	TreeObjectMapEntry   *left;
	TreeObjectMapEntry  *right;
	string                 key;
	int                 height;
	VirtualObject*       value;
	
	TreeObjectMapEntry();
	
	~TreeObjectMapEntry();
};

struct TreeObjectMap {
	int                    size;
	TreeObjectMapEntry *entries;
	
	TreeObjectMap();
	
	TreeObjectMap(TreeObjectMap*);
	
	~TreeObjectMap();
	
	void finalize();
	
	int putAll(TreeObjectMap*);
	
	int put(string, VirtualObject*);
	
	int contains(string);
	
	TreeObjectMapEntry *findEntry(string);
	
	VirtualObject* get(string);

	VirtualObject* remove(string);
	
	// Called by handler object to mark all references
	void mark(void);
	
	// Called when requesting all keys 
	// of this map as scriptable Array
	void keys(Array *a);
};

#endif
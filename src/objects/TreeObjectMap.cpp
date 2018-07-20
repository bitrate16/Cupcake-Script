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


#include "TreeObjectMap.h"

#include "Array.h"
#include "StringType.h"
#include "ObjectConverter.h"

static int max(int i, int j) {
	return i > j ? i : j;
};

static int height(TreeObjectMapEntry *t) {
	return t ? t->height : -1;
};

static TreeObjectMapEntry *singleRightRotate(TreeObjectMapEntry* t) {
	TreeObjectMapEntry *u = t->left;
	t->left   = u->right;
	u->right  = t;
	t->height = max(height(t->left), height(t->right)) + 1;
	u->height = max(height(u->left), t->height) + 1;
	return u;
};

static TreeObjectMapEntry *singleLeftRotate(TreeObjectMapEntry* t) {
	TreeObjectMapEntry *u = t->right;
	t->right  = u->left;
	u->left   = t;
	t->height = max(height(t->left),  height(t->right)) + 1;
	u->height = max(height(u->right), t->height) + 1;
	return u;
};

static TreeObjectMapEntry *doubleRightRotate(TreeObjectMapEntry* t) {
	t->left = singleLeftRotate(t->left);
	return singleRightRotate(t);
};

static TreeObjectMapEntry *doubleLeftRotate(TreeObjectMapEntry* t) {
	t->right = singleRightRotate(t->right);
	return singleLeftRotate(t);
};

static TreeObjectMapEntry *insert_tree(string key, VirtualObject *value, TreeObjectMapEntry *t, bool *inserted) {
	if (t == NULL) {
		t = new TreeObjectMapEntry;
		t->key    = key;
		t->value  = value;
		t->height = 0;
		t->left   = NULL;
		t->right  = NULL;
		*inserted = 1;
		return t;
	} else {
		int cmp = key.compare(t->key);
		
		if (cmp < 0) {
			t->left = insert_tree(key, value, t->left, inserted);
			if (height(t->left) - height(t->right) == 2) {
				if (key.compare(t->left->key) < 0)
					t = singleRightRotate(t);
				else 
					t = doubleRightRotate(t);
			}
		} else if (cmp > 0) {
			t->right = insert_tree(key, value, t->right, inserted);
			if (height(t->right) - height(t->left) == 2) {
				if (key.compare(t->right->key) > 0)
					t = singleLeftRotate(t);
				else 
					t = doubleLeftRotate(t);
			}
		} else {
			t->value = value;
			return t;
		}
	}
	
	t->height = max(height(t->left), height(t->right)) + 1;
	return t;
};

static TreeObjectMapEntry *findMin(TreeObjectMapEntry *t) {
	if (t == NULL)
		return NULL;
	if (t->left == NULL)
		return t;
	return findMin(t->left);
}

static TreeObjectMapEntry *findMax(TreeObjectMapEntry *t) {
	if (t == NULL)
		return NULL;
	if (t->right == NULL)
		return t;
	return findMax(t->right);
}

static TreeObjectMapEntry *remove_tree(string key, TreeObjectMapEntry *t, bool *removed) {
	TreeObjectMapEntry *temp;
	
	// Not found
	if (t == NULL)
		return NULL;
	
	// Searching for element
	int cmp = key.compare(t->key);
	if (cmp < 0)
		t->left = remove_tree(key, t->right, removed);
	else if (cmp > 0)
		t->right = remove_tree(key, t->right, removed);
	
	// Found element with 2 children
	else if (t->left && t->right) {
		*removed = 1;
		temp     = findMin(t->right);
		t->key   = temp->key;
		t->value = temp->value;
		t->right = remove_tree(t->key, t->right, removed);
	}
	
	// With one or zero child
	else {
		temp = t;
		if (t->left == NULL)
			t = t->right;
		else if (t->right == NULL)
			t = t->left;
		delete temp;
	}
	
	if (t == NULL)
		return t;
	
	t->height = max(height(t->left), height(t->right)) + 1;
	
	// If node if unbalanced
	// If left node is deleted, right case
	if (height(t->left) - height(t->right) == 2) {
		// right right case
		if (height(t->left->left) - height(t->left->right) == 1)
			return singleLeftRotate(t);
		// right left case
		else
			return doubleLeftRotate(t);
	}
	
	// If right node is deleted, left case
	else if (height(t->right) - height(t->left) == 2) {
		// left left case
		if (height(t->right->right) - height(t->right->left) == 1)
			return singleRightRotate(t);
		// right right case
		else
			return doubleRightRotate(t);
	}
	
	return t;
};

// </copy>


// TreeObjectMapEntry
TreeObjectMapEntry::TreeObjectMapEntry() {
	left  = NULL;
	right = NULL;
	value = NULL;
};

TreeObjectMapEntry::~TreeObjectMapEntry() {
	delete  left;
	delete right;
};


// TreeObjectMap
TreeObjectMap::TreeObjectMap() {
	size      = 0;
	entries   = NULL;
};

TreeObjectMap::TreeObjectMap(TreeObjectMap *map) {
	size      = 0;
	entries   = NULL;
	putAll(map);
};

TreeObjectMap::~TreeObjectMap() {
	delete entries;
	entries = NULL;
};

void TreeObjectMap::finalize() {
	delete this;
};

static int putAll_tree(TreeObjectMap *m, TreeObjectMapEntry *t) {
	return t ? m->put(t->key, t->value) + putAll_tree(m, t->left) + putAll_tree(m, t->right) : 0;
};

int TreeObjectMap::putAll(TreeObjectMap *map) {
	if (map == NULL)
		return 0;
	
	if (map->size == 0)
		return 0;
	
	return putAll_tree(this, map->entries);
};

int TreeObjectMap::put(string key, VirtualObject *value) {
	if (value == NULL)
		return 0;
	
	bool inserted = 0;
	entries = insert_tree(key, value, entries, &inserted);
	if (inserted)
		++size;
	
	return inserted;
	/*TreeObjectMapEntry *e = entries;
	
	if (!e) {
		entries = new TreeObjectMapEntry;
		entries->key   =   key;
		entries->value = value;
		++size;
		return 1;
	}
	
	while (1) {
		int cmp = key.compare(e->key);
		if (!cmp) {
			e->value = value;
			return 1;
		}
		if (cmp < 0) {
			if (e->left == NULL) {
				e->left = new TreeObjectMapEntry;
				e->left->key   =   key;
				e->left->value = value;
				++size;
				// XXX: Balance tree
				return 1;
			}
			e = e->left;
			continue;
		}
		if (cmp > 0) {
			if (e->right == NULL) {
				e->right = new TreeObjectMapEntry;
				e->right->key   =   key;
				e->right->value = value;
				++size;
				// XXX: Balance tree
				return 1;
			}
			e = e->right;
			continue;
		}
	}*/
};

int TreeObjectMap::contains(string key) {
	TreeObjectMapEntry *e = entries;
	
	while (e) {
		int cmp = key.compare(e->key);
		
		if (!cmp) 
			return 1;
		
		if (cmp < 0) {
			e = e->left;
			continue;
		}
		if (cmp > 0) {
			e = e->right;
			continue;
		}
	}
		
	return 0;
};

TreeObjectMapEntry *TreeObjectMap::findEntry(string key) {
	TreeObjectMapEntry *e = entries;
	
	while (e) {
		int cmp = key.compare(e->key);
		
		if (!cmp) 
			return e;
		
		if (cmp < 0) {
			e = e->left;
			continue;
		}
		if (cmp > 0) {
			e = e->right;
			continue;
		}
	}
		
	return e;
};

VirtualObject* TreeObjectMap::get(string key) {
	TreeObjectMapEntry *e = entries;
	
	while (e) {
		int cmp = key.compare(e->key);
		
		if (!cmp) 
			return e->value;
		
		if (cmp < 0) {
			e = e->left;
			continue;
		}
		if (cmp > 0) {
			e = e->right;
			continue;
		}
	}
		
	return NULL;
};

VirtualObject* TreeObjectMap::remove(string key) {
	bool removed = 0;
	entries = remove_tree(key, entries, &removed);
	
	if (removed)
		--size;
	/*
	TreeObjectMapEntry *t = NULL;
	TreeObjectMapEntry *e = entries;
	bool direction;
	
	if (!e)
		return NULL;
	
	if (!key.compare(entries->key)) {
		TreeObjectMapEntry left  = entries->left;
		TreeObjectMapEntry right = entries->right;
		VirtualObject *o         = entries->value;
		
		delete entries;
		entries = NULL;
		putAll(left);
		putAll(right);
		return o;
	}
	
	while (e) {
		int cmp = key.compare(e->key);
		
		if (!cmp) {
			TreeObjectMapEntry left  = e->left;
			TreeObjectMapEntry right = e->right;
			VirtualObject *o         = e->value;
			
			if (direction) {
				e->right = NULL;
				delete e->right;
			} else {
				e->left  = NULL;
				delete e->left;
			}
			
			putAll(left);
			putAll(right);
			return o;
		}
		if (cmp < 0) {
			t = e;
			direction = 0;
			e = e->left;
			continue;
		}
		if (cmp > 0) {
			t = e;
			direction = 1;
			e = e->right;
			continue;
		}
	}*/
	
	return NULL;
};

static void mark_tree(TreeObjectMapEntry *e) {
	if (!e)
		return;
	
	if (e->value && !e->value->gc_reachable && !e->value->gc_root)
		e->value->mark();
	
	mark_tree(e->left);
	mark_tree(e->right);
};

void TreeObjectMap::mark(void) {
	mark_tree(entries);
};

static void rec_keys(Array *a, TreeObjectMapEntry *e) {
	if (!e)
		return;
	
	a->array->push(new String(e->key));
	
	rec_keys(a, e->left);
	rec_keys(a, e->right);
};

void TreeObjectMap::keys(Array *a) {
	rec_keys(a, entries);
};


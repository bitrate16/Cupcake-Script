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
 * Custom realisation of the std::vector for the Array type.
 */

#ifndef VECTOR_ARRAY_H
#define VECTOR_ARRAY_H

#define VECTOR_ARRAY_DEFAULT_SIZE 16

template<typename T>
struct VectorArray {
	// Size of allocated memory block
	int    size;
	// Size of used memory block
	int  length;
	// Allocated buffer
	T  **vector;
	
	// Creates new VectorArray instance with 
	// default buffer size of VECTOR_ARRAY_DEFAULT_SIZE
	VectorArray() {
		size   = VECTOR_ARRAY_DEFAULT_SIZE;
		length = 0;
		vector = (T**) calloc(VECTOR_ARRAY_DEFAULT_SIZE, sizeof(T*));
	};
	
	// Creates new VectorArray instance with 
	// buffer size of [log(2, size)] + 1
	VectorArray(int size) {
		this->size = 1;
		while (size > this->size)
			this->size <<= 1;
		length     = 0;
		vector     = (T**) calloc(this->size, sizeof(T*));
	};

	// Creates new VectorArray instance with 
	// buffer size of given object and copies 
	// all values into this
	VectorArray(VectorArray &other) {
		size   = other.size;
		length = other.length;
		vector = (T**) calloc(size, sizeof(T*));
		
		for (int i = 0; i < length; ++i)
			vector[i] = other.vector[i];
	};

	// Creates new VectorArray instance with 
	// buffer size of given object and copies 
	// all values into this
	VectorArray(VectorArray *other) {
		if (!other) {
			size   = VECTOR_ARRAY_DEFAULT_SIZE;
			length = 0;
			vector = (T**) calloc(VECTOR_ARRAY_DEFAULT_SIZE, sizeof(T*));
		} else {	
			size   = other->size;
			length = other->length;
			vector = (T**) calloc(size, sizeof(T*));
			
			for (int i = 0; i < length; ++i)
				vector[i] = other->vector[i];
		}
	};

	// Destroes this object by deleteing buffer
	~VectorArray() {
		free(vector);
	};
	
	// Returns object by index in bufer array.
	// If index > length or index < 0 returns NULL
	T *get(int index) {
		if (index < 0 || index >= length)
			return NULL;
		
		return vector[index];
	};
	
	// Puts object into buffer by given index.
	// Returns 1 if index is in bunds, 0 else.
	bool put(int index, T *object) {
		if (index < 0 || index >= length)
			return 0;
		
		vector[index] = object;
		
		return 1;
	};

	// Insert given value into buffer.
	// Returns index of inserted element.
	int push(T *object) {
		if (length == size) 
			vector = (T**) realloc(vector, (size <<= 1) * sizeof(T*));
		
		vector[length] = object;
		
		return length++;
	};
	
	// Remove last value. 
	// Returns NULL if length == 0
	T *pop() {
		if (length == 0)
			return NULL;
		
		T *temp = vector[length];
		vector[length] = NULL;
		--length;
		
		// Compact array
		if ((size >> 1) >= length && size > VECTOR_ARRAY_DEFAULT_SIZE) 
			vector = (T**) realloc(vector, (size >>= 1) * sizeof(T*));
		
		return temp;
	};

	// Reverse all array elements
	void reverse() {
		for (int i = 0; i < length; ++i) {
			T* tmp = vector[i];
			vector[i] = vector[length - 1 - i];
			vector[length - 1 - i] = tmp;
		}
	};
};

#endif

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


#include <stdlib.h>
#include "VectorArray.h"

VectorArray::VectorArray() {
	size   = VECTOR_ARRAY_DEFAULT_SIZE;
	length = 0;
	vector = (T**) malloc(VECTOR_ARRAY_DEFAULT_SIZE * sizeof(T*));
};
	
VectorArray::VectorArray(int size) {
	this->size = 1;
	while (size > this->size)
		this->size <<= 1;
	length     = 0;
	vector     = (T**) malloc(this->size * sizeof(T*));
};

VectorArray::VectorArray(VectorArray &other) {
	size   = other.size;
	length = other.length;
	vector = (T**) malloc(size * sizeof(T*));
	
	for (int i = 0; i < length; ++i)
		vector[i] = other.vector[i];
};

VectorArray::VectorArray(VectorArray *other) {
	if (!other) {
		size   = VECTOR_ARRAY_DEFAULT_SIZE;
		length = 0;
		vector = (T**) malloc(VECTOR_ARRAY_DEFAULT_SIZE * sizeof(T*));
	} else {	
		size   = other->size;
		length = other->length;
		vector = (T**) malloc(size * sizeof(T*));
		
		for (int i = 0; i < length; ++i)
			vector[i] = other->vector[i];
	}
};

VectorArray::~VectorArray() {
	delete vector;
};

T *VectorArray::get(int index) {
	if (index < 0 || index >= length)
		return NULL;
	
	return vector[index];
};

bool VectorArray::put(int index, T *object) {
	if (index < 0 || index >= length)
		return 0;
	
	vector[index] = object;
	
	return 1;
};

int VectorArray::push(T *object) {
	if (length == size) 
		vector = (T**) realloc(vector, (size <<= 1) * sizeof(T*));
	
	vector[length] = object;
	
	return length++;
};

bool VectorArray::pop() {
	if (length == 1)
		return 0;
	
	--length;
	
	// Compact array
	if ((size >> 1) >= length && (size >> 1) > VECTOR_ARRAY_DEFAULT_SIZE) 
		vector = (T**) realloc(vector, (size >>= 1) * sizeof(T*));
	
	return 1;
};
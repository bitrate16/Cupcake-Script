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
 * Wrapper object for safe deletion of 
 * pointers on incorrect program interruption.
 */

#ifndef PTR_WRAPPER
#define PTR_WRAPPER

enum PTR_TYPE {
	PTR_NEW,
	PTR_ALLOC,
	PTR_UNDEF
};

// Used for wrapping pointer in 
// program stack for further 
// safe memory deallocation.
struct ptr_wrapper {
	PTR_TYPE type;
	void *ptr;
	
	ptr_wrapper(void *ptr, PTR_TYPE type) {
		this->ptr  = ptr;
		this->type = type;
	};
	
	ptr_wrapper(void *ptr) {
		this->ptr  = ptr;
		this->type = PTR_ALLOC;
	};
	
	ptr_wrapper() {
		this->ptr  = NULL;
		this->type = PTR_UNDEF;
	};
	
	~ptr_wrapper() {
		switch (type) {
			case PTR_ALLOC:
				free(ptr);
				break;
				
			case PTR_NEW:
				free(ptr);
				break;
		};
	};
	
	void deattach() {
		this->type = PTR_UNDEF;
	};
};

#endif
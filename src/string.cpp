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


#include <cstdlib>
#include <cstdio>
#include "string.h"
#include <locale.h>
#include "ptr_wrapper.h"


static double apow(const float &x, int y) {
	double result = 1;
	if (y == 0)
		return result;
	if (y < 0) {
		y = -y;
		for (int i = 0; i < y; i++)
			result = result * x;
		return 1/result;
	}
	for (int i = 0; i < y; i++)
		result = result * x;
	return result;
};

/* wchar_t to char */
static char* wtoc(const wchar_t* w) {
    setlocale(LC_ALL, "");
	
	int i = (int) wcstombs(NULL, w, 0);
	
	if (i < 0)
		return NULL;
	
	char *d = (char*) malloc((i + 1) * sizeof(char));
	wcstombs(d, w, i);
	d[i] = 0;
	
	return d;
}

/* char to wchar_t */
static wchar_t* ctow(const char* c) {
	setlocale(LC_ALL, "");	
	
	int i = (int) mbstowcs(NULL, c, 0);
	
	if (i < 0)
		return NULL;
	
	wchar_t *d = (wchar_t*) malloc((i + 1) * sizeof(wchar_t));
	mbstowcs(d, c, i);
	d[i] = 0;
	
	return d;
}


// string
string::string() {
	this->buffer = NULL;
	this->length = 0;
	this->size   = 0;
};

string::string(int size) {
	length = 0;
	
	if (size == 0) {
		this->size   =    0;
		buffer       = NULL;
		return;
	}
	
	// size = 2^k
	this->size  = 1;
	while (size > this->size)
		this->size <<= 2;
	
	this->buffer = (wchar_t*) calloc(this->size, sizeof(wchar_t));
};

string::string(const char *cdata) {		
	ptr_wrapper wrapper(ctow(cdata), PTR_ALLOC);
	wchar_t *data = (wchar_t*) wrapper.ptr;
	
	if (data == NULL) { 
		buffer = NULL;
		size   = 0;
		length = 0;
		return;
	}
	
	this->length = 0;
	while (data[length]) length++;
	
	if (length == 0) {
		size   =    0;
		buffer = NULL;
		return;
	}
	
	// size = 2^k
	size    = 1;
	while (length > size)
		size <<= 2;
	
	buffer = (wchar_t*) calloc(size, sizeof(wchar_t));
	for (int i = 0; i < length; i++)
		buffer[i] = data[i];
};

string::string(const wchar_t *data) {		
	if (data == NULL) { 
		buffer = NULL;
		size   = 0;
		length = 0;
		return;
	}
	
	this->length = 0;
	while (data[length]) length++;
	
	if (length == 0) {
		size   =    0;
		buffer = NULL;
		return;
	}
	
	// size = 2^k
	size    = 1;
	while (length > size)
		size <<= 2;
	
	buffer = (wchar_t*) calloc(size, sizeof(wchar_t));
	for (int i = 0; i < length; i++)
		buffer[i] = data[i];
};

string::string(string *str) {
	if (str == NULL) {
		buffer = NULL;
		length = 0;
		size   = 0;
		return;
	}
	
	if (str->length == 0) {
		buffer = NULL;
		length = 0;
		size   = 0;
		return;
	}
	
	// size = 2^k
	length  = str->length;	
	size    = 1;
	while (length > size)
		size <<= 2;
	
	buffer = (wchar_t*) calloc(size, sizeof(wchar_t));
	
	for (int i = 0; i < length; i++)
		buffer[i] = str->buffer[i];
};

string::string(const string &str) {
	if (&str == this)
		return;
	
	if (str.length == 0) {
		buffer = NULL;
		length = 0;
		size   = 0;
		return;
	}
	
	// size = 2^k
	length  = str.length;
	size    = 1;
	while (length > size)
		size <<= 2;
	
	buffer = (wchar_t*) calloc(size, sizeof(wchar_t));
	
	for (int i = 0; i < length; i++)
		buffer[i] = str.buffer[i];
};

string::~string() {
	free(buffer);
};

// Accessor --> read
wchar_t string::operator[](int i) {
	return buffer[i];
};

// Mutator --> change
const wchar_t &string::operator[](int i) const {
	return buffer[i];
};

bool string::operator==(const string &s) const {		
	if (&s == this)
		return 1;
	
	if (s.length != length)
		return 0;
	
	for (int i = 0; i < length; i++)
		if (buffer[i] != s.buffer[i])
			return 0;
		
	return 1;
};

bool string::operator==(const char *cs) const {
	ptr_wrapper wrapper(ctow(cs), PTR_ALLOC);
	wchar_t *s = (wchar_t*) wrapper.ptr;
	
	int len = 0;
	while (s[len]) len++;
	
	if (len == 0 && length == 0)
		return 1;
	if (len != length)
		return 0;
	
	for (int i = 0; i < len; i++)
		if (buffer[i] != s[i])
			return 0;
	return 1;
};

bool string::operator==(const wchar_t *s) const {
	int len = 0;
	while (s[len]) len++;
	
	if (len == 0 && length == 0)
		return 1;
	if (len != length)
		return 0;
	
	for (int i = 0; i < len; i++)
		if (buffer[i] != s[i])
			return 0;
	return 1;
};

bool string::operator!=(const string &s) const {
	if (&s == this)
		return 0;
	
	if (s.length != length)
		return 1;
	
	for (int i = 0; i < length; i++)
		if (buffer[i] != s.buffer[i])
			return 1;
	return 0;
};

bool string::operator!=(const char *s) const {
	int len = 0;
	while (s[len]) len++;
	
	if (len == 0 && length == 0)
		return 0;
	if (len != length)
		return 1;
	
	for (int i = 0; i < len; i++)
		if (buffer[i] != s[i])
			return 1;
	return 0;
};

bool string::operator!=(const wchar_t *s) const {
	int len = 0;
	while (s[len]) len++;
	
	if (len == 0 && length == 0)
		return 0;
	if (len != length)
		return 1;
	
	for (int i = 0; i < len; i++)
		if (buffer[i] != s[i])
			return 1;
	return 0;
};

string &string::operator=(const char *s) {
	if (s == NULL) {
		free(buffer);
		buffer = NULL;
		length = 0;
		size   = 0;
		return *this;
	}
	
	int slength = 0;
	while (s[slength]) ++slength;
	length = slength;	
	
	if (length == 0) {
		free(buffer);
		buffer = NULL;
		size   = 0;
		return *this;
	}
	
	if (slength > size) {
		// size = 2^k
		size    = 1;
		while (length > size)
			size <<= 2;
		
		buffer = (wchar_t*) realloc(buffer, size * sizeof(wchar_t));
	}
	
	slength = 0;
	while (s[slength]) {
		buffer[slength] = s[slength];
		++slength;
	}
	
	return *this;
};

string &string::operator=(const wchar_t *s) {
	if (s == NULL) {
		free(buffer);
		buffer = NULL;
		length = 0;
		size   = 0;
		return *this;
	}
	
	int slength = 0;
	while (s[slength]) ++slength;
	length = slength;	
	
	if (length == 0) {
		free(buffer);
		buffer = NULL;
		size   = 0;
		return *this;
	}
	
	if (slength > size) {
		// size = 2^k
		size    = 1;
		while (length > size)
			size <<= 2;
		
		buffer = (wchar_t*) realloc(buffer, size * sizeof(wchar_t));
	}
	
	slength = 0;
	while (s[slength]) {
		buffer[slength] = s[slength];
		++slength;
	}
	
	return *this;
};

string &string::operator=(const string &s) {
	if (this == &s)
		return *this;
	
	free(buffer);
	
	length = s.length;
	
	if (length == 0) {
		buffer = NULL;
		size   = 0;
		return *this;
	}
	
	// size = 2^k
	size    = 1;
	while (length > size)
		size <<= 2;
	
	buffer = (wchar_t*) calloc(size, sizeof(wchar_t));
	
	for (int i = 0; i < length; ++i)
		buffer[i] = s.buffer[i];
	
	return *this;
};

string &string::operator+=(const char c) {
	if (size >= length + 1)
		buffer[length++] = c;
	else {
		if (buffer != NULL) 
			buffer = (wchar_t*) realloc(buffer, (size <<= 1) * sizeof(wchar_t));
		else
			buffer = (wchar_t*) calloc(size = 16, sizeof(wchar_t));
		
		buffer[length] = c;
		++length;
	};
	return *this;
};

string &string::operator+=(const wchar_t c) {
	if (size >= length + 1)
		buffer[length++] = c;
	else {
		if (buffer != NULL) 
			buffer = (wchar_t*) realloc(buffer, (size <<= 1) * sizeof(wchar_t));
		else
			buffer = (wchar_t*) calloc(size = 16, sizeof(wchar_t));
		
		buffer[length] = c;
		++length;
	};
	return *this;
};

string &string::operator+=(const wint_t c) {
	if (size >= length + 1)
		buffer[length++] = c;
	else {
		if (buffer != NULL) 
			buffer = (wchar_t*) realloc(buffer, (size <<= 1) * sizeof(wchar_t));
		else
			buffer = (wchar_t*) calloc(size = 16, sizeof(wchar_t));
		
		buffer[length] = c;
		++length;
	};
	return *this;
};

string &string::operator+=(const int i) {
	if (size >= length + 1)
		buffer[length++] = i;
	else {
		if (buffer != NULL) 
			buffer = (wchar_t*) realloc(buffer, (size <<= 1) * sizeof(wchar_t));
		else
			buffer = (wchar_t*) calloc(size = 16, sizeof(wchar_t));
		
		buffer[length] = i;
		++length;
	};
	return *this;
};

string &string::operator+=(const string &s) {
	if (s.buffer != NULL)
		if (s.length + length > size) {
			if (buffer != NULL) {
				int len = length + s.length;
				
				size = 1;
				while (len > size)
					size <<= 2;
				
				buffer = (wchar_t*) realloc(buffer, size * sizeof(wchar_t));
			} else {
				size = 1;
				while (s.length > size)
					size <<= 2;
					
				buffer = (wchar_t*) calloc(size, sizeof(wchar_t));
			}
			
			for (int i = 0; i < s.length; i++) 
				buffer[i + length] = s.buffer[i];
			
			length += s.length;
		} else {
			for (int i = 0; i < s.length; i++) 
				buffer[i + length] = s.buffer[i];
			
			length += s.length;
		}
	return *this;
};

string &string::operator+=(const char *cs) {
	ptr_wrapper wrapper(ctow(cs), PTR_ALLOC);
	wchar_t *s = (wchar_t*) wrapper.ptr;
	
	if (s != NULL) {
		int len = 0;
		while (s[len]) len++;
		
		if (len + length > size) {
			if (buffer != NULL) {
				int tlen = length + len;
				
				size = 1;
				while (tlen > size)
					size <<= 2;
				
				buffer = (wchar_t*) realloc(buffer, size * sizeof(wchar_t));
			} else {
				size = 1;
				while (len > size)
					size <<= 2;
					
				buffer = (wchar_t*) calloc(size, sizeof(wchar_t));
			}
			
			for (int i = 0; i < len; i++) 
				buffer[i + length] = s[i];
			
			length += len;
		} else {
			for (int i = 0; i < len; i++) 
				buffer[i + length] = s[i];
			
			length += len;
		}
	}
	return *this;
};

string &string::operator+=(const wchar_t *s) {
	if (s != NULL) {
		int len = 0;
		while (s[len]) len++;
		
		if (len + length > size) {
			if (buffer != NULL) {
				int tlen = length + len;
				
				size = 1;
				while (tlen > size)
					size <<= 2;
				
				buffer = (wchar_t*) realloc(buffer, size * sizeof(wchar_t));
			} else {
				size = 1;
				while (len > size)
					size <<= 2;
					
				buffer = (wchar_t*) calloc(size, sizeof(wchar_t));
			}
			
			for (int i = 0; i < len; i++) 
				buffer[i + length] = s[i];
			
			length += len;
		} else {
			for (int i = 0; i < len; i++) 
				buffer[i + length] = s[i];
			
			length += len;
		}
	}
	return *this;
};

string string::operator+(const char c) {
	string st = string(this);
	st       += (wchar_t) c;
	return st;
};

string string::operator+(const wchar_t c) {
	string st = string(this);
	st       += (wchar_t) c;
	return st;
};

string string::operator+(const int i) {
	string st = string(this);
	st       += i;
	return st;
};

string string::operator+(const string &s) {
	string st = string(this);
	st       += s;
	return st;
};

string string::operator+(const char *s) {
	string st = string(this);
	st       += s;
	return st;
};

string string::operator+(const wchar_t *s) {
	string st = string(this);
	st       += s;
	return st;
};

void string::reverse() {
	if (length < 2)
		return;
	
	for (int i = 0; i < length / 2; ++i) {
		int tmp = buffer[i];
		buffer[i] = buffer[length - 1 - i];
		buffer[length - 1 - i] = tmp;
	}
};

int string::compare(string *s) {
	if (s == NULL)
		return length;
	
	int minlen = length < s->length ? length : s->length;
	
	for (int i = 0; i < minlen; ++i)
		if (buffer[i] != s->buffer[i])
			return buffer[i] - s->buffer[i];
		
	return length - s->length;
};
	
int string::compare(string &s) {	
	int minlen = length < s.length ? length : s.length;
	
	for (int i = 0; i < minlen; ++i)
		if (buffer[i] != s.buffer[i])
			return buffer[i] - s.buffer[i];
		
	return length - s.length;
};
	
int string::compare(const char *s) {
	int i = 0;
	for (; i < length; ++i) {
		if (!s[i])
			return buffer[i];
		if (buffer[i] != s[i])
			return buffer[i] - s[i];
	}
	
	return -s[i];
};
	
int string::compare(const wchar_t *s) {
	int i = 0;
	for (; i < length; ++i) {
		if (!s[i])
			return buffer[i];
		if (buffer[i] != s[i])
			return buffer[i] - s[i];
	}
	
	return -s[i];
};

wchar_t string::toUpper(wchar_t c) {
	switch(c) {
		case 0x61: return 0x41;
		case 0x62: return 0x42;
		case 0x63: return 0x43;
		case 0x64: return 0x44;
		case 0x65: return 0x45;
		case 0x66: return 0x46;
		case 0x67: return 0x47;
		case 0x68: return 0x48;
		case 0x69: return 0x49;
		case 0x6A: return 0x4A;
		case 0x6B: return 0x4B;
		case 0x6C: return 0x4C;
		case 0x6D: return 0x4D;
		case 0x6E: return 0x4E;
		case 0x6F: return 0x4F;
		case 0x70: return 0x50;
		case 0x71: return 0x51;
		case 0x72: return 0x52;
		case 0x73: return 0x53;
		case 0x74: return 0x54;
		case 0x75: return 0x55;
		case 0x76: return 0x56;
		case 0x77: return 0x57;
		case 0x78: return 0x58;
		case 0x79: return 0x59;
		case 0x7A: return 0x5A;
		case 0xE0: return 0xC0;
		case 0xE1: return 0xC1;
		case 0xE2: return 0xC2;
		case 0xE3: return 0xC3;
		case 0xE4: return 0xC4;
		case 0xE5: return 0xC5;
		case 0xE6: return 0xC6;
		case 0xE7: return 0xC7;
		case 0xE8: return 0xC8;
		case 0xE9: return 0xC9;
		case 0xEA: return 0xCA;
		case 0xEB: return 0xCB;
		case 0xEC: return 0xCC;
		case 0xED: return 0xCD;
		case 0xEE: return 0xCE;
		case 0xEF: return 0xCF;
		case 0xF0: return 0xD0;
		case 0xF1: return 0xD1;
		case 0xF2: return 0xD2;
		case 0xF3: return 0xD3;
		case 0xF4: return 0xD4;
		case 0xF5: return 0xD5;
		case 0xF6: return 0xD6;
		case 0xF8: return 0xD8;
		case 0xF9: return 0xD9;
		case 0xFA: return 0xDA;
		case 0xFB: return 0xDB;
		case 0xFC: return 0xDC;
		case 0xFD: return 0xDD;
		case 0xFE: return 0xDE;
		case 0xFF: return 0x178;
		case 0x101: return 0x100;
		case 0x103: return 0x102;
		case 0x105: return 0x104;
		case 0x107: return 0x106;
		case 0x109: return 0x108;
		case 0x10B: return 0x10A;
		case 0x10D: return 0x10C;
		case 0x10F: return 0x10E;
		case 0x111: return 0x110;
		case 0x113: return 0x112;
		case 0x115: return 0x114;
		case 0x117: return 0x116;
		case 0x119: return 0x118;
		case 0x11B: return 0x11A;
		case 0x11D: return 0x11C;
		case 0x11F: return 0x11E;
		case 0x121: return 0x120;
		case 0x123: return 0x122;
		case 0x125: return 0x124;
		case 0x127: return 0x126;
		case 0x129: return 0x128;
		case 0x12B: return 0x12A;
		case 0x12D: return 0x12C;
		case 0x12F: return 0x12E;
		case 0x131: return 0x130;
		case 0x133: return 0x132;
		case 0x135: return 0x134;
		case 0x137: return 0x136;
		case 0x13A: return 0x139;
		case 0x13C: return 0x13B;
		case 0x13E: return 0x13D;
		case 0x140: return 0x13F;
		case 0x142: return 0x141;
		case 0x144: return 0x143;
		case 0x146: return 0x145;
		case 0x148: return 0x147;
		case 0x14B: return 0x14A;
		case 0x14D: return 0x14C;
		case 0x14F: return 0x14E;
		case 0x151: return 0x150;
		case 0x153: return 0x152;
		case 0x155: return 0x154;
		case 0x157: return 0x156;
		case 0x159: return 0x158;
		case 0x15B: return 0x15A;
		case 0x15D: return 0x15C;
		case 0x15F: return 0x15E;
		case 0x161: return 0x160;
		case 0x163: return 0x162;
		case 0x165: return 0x164;
		case 0x167: return 0x166;
		case 0x169: return 0x168;
		case 0x16B: return 0x16A;
		case 0x16D: return 0x16C;
		case 0x16F: return 0x16E;
		case 0x171: return 0x170;
		case 0x173: return 0x172;
		case 0x175: return 0x174;
		case 0x177: return 0x176;
		case 0x17A: return 0x179;
		case 0x17C: return 0x17B;
		case 0x17E: return 0x17D;
		case 0x183: return 0x182;
		case 0x185: return 0x184;
		case 0x188: return 0x187;
		case 0x18C: return 0x18B;
		case 0x192: return 0x191;
		case 0x199: return 0x198;
		case 0x1A1: return 0x1A0;
		case 0x1A3: return 0x1A2;
		case 0x1A5: return 0x1A4;
		case 0x1A8: return 0x1A7;
		case 0x1AD: return 0x1AC;
		case 0x1B0: return 0x1AF;
		case 0x1B4: return 0x1B3;
		case 0x1B6: return 0x1B5;
		case 0x1B9: return 0x1B8;
		case 0x1BD: return 0x1BC;
		case 0x1C6: return 0x1C4;
		case 0x1C9: return 0x1C7;
		case 0x1CC: return 0x1CA;
		case 0x1CE: return 0x1CD;
		case 0x1D0: return 0x1CF;
		case 0x1D2: return 0x1D1;
		case 0x1D4: return 0x1D3;
		case 0x1D6: return 0x1D5;
		case 0x1D8: return 0x1D7;
		case 0x1DA: return 0x1D9;
		case 0x1DC: return 0x1DB;
		case 0x1DF: return 0x1DE;
		case 0x1E1: return 0x1E0;
		case 0x1E3: return 0x1E2;
		case 0x1E5: return 0x1E4;
		case 0x1E7: return 0x1E6;
		case 0x1E9: return 0x1E8;
		case 0x1EB: return 0x1EA;
		case 0x1ED: return 0x1EC;
		case 0x1EF: return 0x1EE;
		case 0x1F3: return 0x1F1;
		case 0x1F5: return 0x1F4;
		case 0x1FB: return 0x1FA;
		case 0x1FD: return 0x1FC;
		case 0x1FF: return 0x1FE;
		case 0x201: return 0x200;
		case 0x203: return 0x202;
		case 0x205: return 0x204;
		case 0x207: return 0x206;
		case 0x209: return 0x208;
		case 0x20B: return 0x20A;
		case 0x20D: return 0x20C;
		case 0x20F: return 0x20E;
		case 0x211: return 0x210;
		case 0x213: return 0x212;
		case 0x215: return 0x214;
		case 0x217: return 0x216;
		case 0x253: return 0x181;
		case 0x254: return 0x186;
		case 0x257: return 0x18A;
		case 0x258: return 0x18E;
		case 0x259: return 0x18F;
		case 0x25B: return 0x190;
		case 0x260: return 0x193;
		case 0x263: return 0x194;
		case 0x268: return 0x197;
		case 0x269: return 0x196;
		case 0x26F: return 0x19C;
		case 0x272: return 0x19D;
		case 0x275: return 0x19F;
		case 0x283: return 0x1A9;
		case 0x288: return 0x1AE;
		case 0x28A: return 0x1B1;
		case 0x28B: return 0x1B2;
		case 0x292: return 0x1B7;
		case 0x3AC: return 0x386;
		case 0x3AD: return 0x388;
		case 0x3AE: return 0x389;
		case 0x3AF: return 0x38A;
		case 0x3B1: return 0x391;
		case 0x3B2: return 0x392;
		case 0x3B3: return 0x393;
		case 0x3B4: return 0x394;
		case 0x3B5: return 0x395;
		case 0x3B6: return 0x396;
		case 0x3B7: return 0x397;
		case 0x3B8: return 0x398;
		case 0x3B9: return 0x399;
		case 0x3BA: return 0x39A;
		case 0x3BB: return 0x39B;
		case 0x3BC: return 0x39C;
		case 0x3BD: return 0x39D;
		case 0x3BE: return 0x39E;
		case 0x3BF: return 0x39F;
		case 0x3C0: return 0x3A0;
		case 0x3C1: return 0x3A1;
		case 0x3C3: return 0x3A3;
		case 0x3C4: return 0x3A4;
		case 0x3C5: return 0x3A5;
		case 0x3C6: return 0x3A6;
		case 0x3C7: return 0x3A7;
		case 0x3C8: return 0x3A8;
		case 0x3C9: return 0x3A9;
		case 0x3CA: return 0x3AA;
		case 0x3CB: return 0x3AB;
		case 0x3CC: return 0x38C;
		case 0x3CD: return 0x38E;
		case 0x3CE: return 0x38F;
		case 0x3E3: return 0x3E2;
		case 0x3E5: return 0x3E4;
		case 0x3E7: return 0x3E6;
		case 0x3E9: return 0x3E8;
		case 0x3EB: return 0x3EA;
		case 0x3ED: return 0x3EC;
		case 0x3EF: return 0x3EE;
		case 0x430: return 0x410;
		case 0x431: return 0x411;
		case 0x432: return 0x412;
		case 0x433: return 0x413;
		case 0x434: return 0x414;
		case 0x435: return 0x415;
		case 0x436: return 0x416;
		case 0x437: return 0x417;
		case 0x438: return 0x418;
		case 0x439: return 0x419;
		case 0x43A: return 0x41A;
		case 0x43B: return 0x41B;
		case 0x43C: return 0x41C;
		case 0x43D: return 0x41D;
		case 0x43E: return 0x41E;
		case 0x43F: return 0x41F;
		case 0x440: return 0x420;
		case 0x441: return 0x421;
		case 0x442: return 0x422;
		case 0x443: return 0x423;
		case 0x444: return 0x424;
		case 0x445: return 0x425;
		case 0x446: return 0x426;
		case 0x447: return 0x427;
		case 0x448: return 0x428;
		case 0x449: return 0x429;
		case 0x44A: return 0x42A;
		case 0x44B: return 0x42B;
		case 0x44C: return 0x42C;
		case 0x44D: return 0x42D;
		case 0x44E: return 0x42E;
		case 0x44F: return 0x42F;
		case 0x451: return 0x401;
		case 0x452: return 0x402;
		case 0x453: return 0x403;
		case 0x454: return 0x404;
		case 0x455: return 0x405;
		case 0x456: return 0x406;
		case 0x457: return 0x407;
		case 0x458: return 0x408;
		case 0x459: return 0x409;
		case 0x45A: return 0x40A;
		case 0x45B: return 0x40B;
		case 0x45C: return 0x40C;
		case 0x45E: return 0x40E;
		case 0x45F: return 0x40F;
		case 0x461: return 0x460;
		case 0x463: return 0x462;
		case 0x465: return 0x464;
		case 0x467: return 0x466;
		case 0x469: return 0x468;
		case 0x46B: return 0x46A;
		case 0x46D: return 0x46C;
		case 0x46F: return 0x46E;
		case 0x471: return 0x470;
		case 0x473: return 0x472;
		case 0x475: return 0x474;
		case 0x477: return 0x476;
		case 0x479: return 0x478;
		case 0x47B: return 0x47A;
		case 0x47D: return 0x47C;
		case 0x47F: return 0x47E;
		case 0x481: return 0x480;
		case 0x491: return 0x490;
		case 0x493: return 0x492;
		case 0x495: return 0x494;
		case 0x497: return 0x496;
		case 0x499: return 0x498;
		case 0x49B: return 0x49A;
		case 0x49D: return 0x49C;
		case 0x49F: return 0x49E;
		case 0x4A1: return 0x4A0;
		case 0x4A3: return 0x4A2;
		case 0x4A5: return 0x4A4;
		case 0x4A7: return 0x4A6;
		case 0x4A9: return 0x4A8;
		case 0x4AB: return 0x4AA;
		case 0x4AD: return 0x4AC;
		case 0x4AF: return 0x4AE;
		case 0x4B1: return 0x4B0;
		case 0x4B3: return 0x4B2;
		case 0x4B5: return 0x4B4;
		case 0x4B7: return 0x4B6;
		case 0x4B9: return 0x4B8;
		case 0x4BB: return 0x4BA;
		case 0x4BD: return 0x4BC;
		case 0x4BF: return 0x4BE;
		case 0x4C2: return 0x4C1;
		case 0x4C4: return 0x4C3;
		case 0x4C8: return 0x4C7;
		case 0x4CC: return 0x4CB;
		case 0x4D1: return 0x4D0;
		case 0x4D3: return 0x4D2;
		case 0x4D5: return 0x4D4;
		case 0x4D7: return 0x4D6;
		case 0x4D9: return 0x4D8;
		case 0x4DB: return 0x4DA;
		case 0x4DD: return 0x4DC;
		case 0x4DF: return 0x4DE;
		case 0x4E1: return 0x4E0;
		case 0x4E3: return 0x4E2;
		case 0x4E5: return 0x4E4;
		case 0x4E7: return 0x4E6;
		case 0x4E9: return 0x4E8;
		case 0x4EB: return 0x4EA;
		case 0x4EF: return 0x4EE;
		case 0x4F1: return 0x4F0;
		case 0x4F3: return 0x4F2;
		case 0x4F5: return 0x4F4;
		case 0x4F9: return 0x4F8;
		case 0x561: return 0x531;
		case 0x562: return 0x532;
		case 0x563: return 0x533;
		case 0x564: return 0x534;
		case 0x565: return 0x535;
		case 0x566: return 0x536;
		case 0x567: return 0x537;
		case 0x568: return 0x538;
		case 0x569: return 0x539;
		case 0x56A: return 0x53A;
		case 0x56B: return 0x53B;
		case 0x56C: return 0x53C;
		case 0x56D: return 0x53D;
		case 0x56E: return 0x53E;
		case 0x56F: return 0x53F;
		case 0x570: return 0x540;
		case 0x571: return 0x541;
		case 0x572: return 0x542;
		case 0x573: return 0x543;
		case 0x574: return 0x544;
		case 0x575: return 0x545;
		case 0x576: return 0x546;
		case 0x577: return 0x547;
		case 0x578: return 0x548;
		case 0x579: return 0x549;
		case 0x57A: return 0x54A;
		case 0x57B: return 0x54B;
		case 0x57C: return 0x54C;
		case 0x57D: return 0x54D;
		case 0x57E: return 0x54E;
		case 0x57F: return 0x54F;
		case 0x580: return 0x550;
		case 0x581: return 0x551;
		case 0x582: return 0x552;
		case 0x583: return 0x553;
		case 0x584: return 0x554;
		case 0x585: return 0x555;
		case 0x586: return 0x556;
		case 0x10D0: return 0x10A0;
		case 0x10D1: return 0x10A1;
		case 0x10D2: return 0x10A2;
		case 0x10D3: return 0x10A3;
		case 0x10D4: return 0x10A4;
		case 0x10D5: return 0x10A5;
		case 0x10D6: return 0x10A6;
		case 0x10D7: return 0x10A7;
		case 0x10D8: return 0x10A8;
		case 0x10D9: return 0x10A9;
		case 0x10DA: return 0x10AA;
		case 0x10DB: return 0x10AB;
		case 0x10DC: return 0x10AC;
		case 0x10DD: return 0x10AD;
		case 0x10DE: return 0x10AE;
		case 0x10DF: return 0x10AF;
		case 0x10E0: return 0x10B0;
		case 0x10E1: return 0x10B1;
		case 0x10E2: return 0x10B2;
		case 0x10E3: return 0x10B3;
		case 0x10E4: return 0x10B4;
		case 0x10E5: return 0x10B5;
		case 0x10E6: return 0x10B6;
		case 0x10E7: return 0x10B7;
		case 0x10E8: return 0x10B8;
		case 0x10E9: return 0x10B9;
		case 0x10EA: return 0x10BA;
		case 0x10EB: return 0x10BB;
		case 0x10EC: return 0x10BC;
		case 0x10ED: return 0x10BD;
		case 0x10EE: return 0x10BE;
		case 0x10EF: return 0x10BF;
		case 0x10F0: return 0x10C0;
		case 0x10F1: return 0x10C1;
		case 0x10F2: return 0x10C2;
		case 0x10F3: return 0x10C3;
		case 0x10F4: return 0x10C4;
		case 0x10F5: return 0x10C5;
		case 0x1E01: return 0x1E00;
		case 0x1E03: return 0x1E02;
		case 0x1E05: return 0x1E04;
		case 0x1E07: return 0x1E06;
		case 0x1E09: return 0x1E08;
		case 0x1E0B: return 0x1E0A;
		case 0x1E0D: return 0x1E0C;
		case 0x1E0F: return 0x1E0E;
		case 0x1E11: return 0x1E10;
		case 0x1E13: return 0x1E12;
		case 0x1E15: return 0x1E14;
		case 0x1E17: return 0x1E16;
		case 0x1E19: return 0x1E18;
		case 0x1E1B: return 0x1E1A;
		case 0x1E1D: return 0x1E1C;
		case 0x1E1F: return 0x1E1E;
		case 0x1E21: return 0x1E20;
		case 0x1E23: return 0x1E22;
		case 0x1E25: return 0x1E24;
		case 0x1E27: return 0x1E26;
		case 0x1E29: return 0x1E28;
		case 0x1E2B: return 0x1E2A;
		case 0x1E2D: return 0x1E2C;
		case 0x1E2F: return 0x1E2E;
		case 0x1E31: return 0x1E30;
		case 0x1E33: return 0x1E32;
		case 0x1E35: return 0x1E34;
		case 0x1E37: return 0x1E36;
		case 0x1E39: return 0x1E38;
		case 0x1E3B: return 0x1E3A;
		case 0x1E3D: return 0x1E3C;
		case 0x1E3F: return 0x1E3E;
		case 0x1E41: return 0x1E40;
		case 0x1E43: return 0x1E42;
		case 0x1E45: return 0x1E44;
		case 0x1E47: return 0x1E46;
		case 0x1E49: return 0x1E48;
		case 0x1E4B: return 0x1E4A;
		case 0x1E4D: return 0x1E4C;
		case 0x1E4F: return 0x1E4E;
		case 0x1E51: return 0x1E50;
		case 0x1E53: return 0x1E52;
		case 0x1E55: return 0x1E54;
		case 0x1E57: return 0x1E56;
		case 0x1E59: return 0x1E58;
		case 0x1E5B: return 0x1E5A;
		case 0x1E5D: return 0x1E5C;
		case 0x1E5F: return 0x1E5E;
		case 0x1E61: return 0x1E60;
		case 0x1E63: return 0x1E62;
		case 0x1E65: return 0x1E64;
		case 0x1E67: return 0x1E66;
		case 0x1E69: return 0x1E68;
		case 0x1E6B: return 0x1E6A;
		case 0x1E6D: return 0x1E6C;
		case 0x1E6F: return 0x1E6E;
		case 0x1E71: return 0x1E70;
		case 0x1E73: return 0x1E72;
		case 0x1E75: return 0x1E74;
		case 0x1E77: return 0x1E76;
		case 0x1E79: return 0x1E78;
		case 0x1E7B: return 0x1E7A;
		case 0x1E7D: return 0x1E7C;
		case 0x1E7F: return 0x1E7E;
		case 0x1E81: return 0x1E80;
		case 0x1E83: return 0x1E82;
		case 0x1E85: return 0x1E84;
		case 0x1E87: return 0x1E86;
		case 0x1E89: return 0x1E88;
		case 0x1E8B: return 0x1E8A;
		case 0x1E8D: return 0x1E8C;
		case 0x1E8F: return 0x1E8E;
		case 0x1E91: return 0x1E90;
		case 0x1E93: return 0x1E92;
		case 0x1E95: return 0x1E94;
		case 0x1EA1: return 0x1EA0;
		case 0x1EA3: return 0x1EA2;
		case 0x1EA5: return 0x1EA4;
		case 0x1EA7: return 0x1EA6;
		case 0x1EA9: return 0x1EA8;
		case 0x1EAB: return 0x1EAA;
		case 0x1EAD: return 0x1EAC;
		case 0x1EAF: return 0x1EAE;
		case 0x1EB1: return 0x1EB0;
		case 0x1EB3: return 0x1EB2;
		case 0x1EB5: return 0x1EB4;
		case 0x1EB7: return 0x1EB6;
		case 0x1EB9: return 0x1EB8;
		case 0x1EBB: return 0x1EBA;
		case 0x1EBD: return 0x1EBC;
		case 0x1EBF: return 0x1EBE;
		case 0x1EC1: return 0x1EC0;
		case 0x1EC3: return 0x1EC2;
		case 0x1EC5: return 0x1EC4;
		case 0x1EC7: return 0x1EC6;
		case 0x1EC9: return 0x1EC8;
		case 0x1ECB: return 0x1ECA;
		case 0x1ECD: return 0x1ECC;
		case 0x1ECF: return 0x1ECE;
		case 0x1ED1: return 0x1ED0;
		case 0x1ED3: return 0x1ED2;
		case 0x1ED5: return 0x1ED4;
		case 0x1ED7: return 0x1ED6;
		case 0x1ED9: return 0x1ED8;
		case 0x1EDB: return 0x1EDA;
		case 0x1EDD: return 0x1EDC;
		case 0x1EDF: return 0x1EDE;
		case 0x1EE1: return 0x1EE0;
		case 0x1EE3: return 0x1EE2;
		case 0x1EE5: return 0x1EE4;
		case 0x1EE7: return 0x1EE6;
		case 0x1EE9: return 0x1EE8;
		case 0x1EEB: return 0x1EEA;
		case 0x1EED: return 0x1EEC;
		case 0x1EEF: return 0x1EEE;
		case 0x1EF1: return 0x1EF0;
		case 0x1EF3: return 0x1EF2;
		case 0x1EF5: return 0x1EF4;
		case 0x1EF7: return 0x1EF6;
		case 0x1EF9: return 0x1EF8;
		case 0x1F00: return 0x1F08;
		case 0x1F01: return 0x1F09;
		case 0x1F02: return 0x1F0A;
		case 0x1F03: return 0x1F0B;
		case 0x1F04: return 0x1F0C;
		case 0x1F05: return 0x1F0D;
		case 0x1F06: return 0x1F0E;
		case 0x1F07: return 0x1F0F;
		case 0x1F10: return 0x1F18;
		case 0x1F11: return 0x1F19;
		case 0x1F12: return 0x1F1A;
		case 0x1F13: return 0x1F1B;
		case 0x1F14: return 0x1F1C;
		case 0x1F15: return 0x1F1D;
		case 0x1F20: return 0x1F28;
		case 0x1F21: return 0x1F29;
		case 0x1F22: return 0x1F2A;
		case 0x1F23: return 0x1F2B;
		case 0x1F24: return 0x1F2C;
		case 0x1F25: return 0x1F2D;
		case 0x1F26: return 0x1F2E;
		case 0x1F27: return 0x1F2F;
		case 0x1F30: return 0x1F38;
		case 0x1F31: return 0x1F39;
		case 0x1F32: return 0x1F3A;
		case 0x1F33: return 0x1F3B;
		case 0x1F34: return 0x1F3C;
		case 0x1F35: return 0x1F3D;
		case 0x1F36: return 0x1F3E;
		case 0x1F37: return 0x1F3F;
		case 0x1F40: return 0x1F48;
		case 0x1F41: return 0x1F49;
		case 0x1F42: return 0x1F4A;
		case 0x1F43: return 0x1F4B;
		case 0x1F44: return 0x1F4C;
		case 0x1F45: return 0x1F4D;
		case 0x1F51: return 0x1F59;
		case 0x1F53: return 0x1F5B;
		case 0x1F55: return 0x1F5D;
		case 0x1F57: return 0x1F5F;
		case 0x1F60: return 0x1F68;
		case 0x1F61: return 0x1F69;
		case 0x1F62: return 0x1F6A;
		case 0x1F63: return 0x1F6B;
		case 0x1F64: return 0x1F6C;
		case 0x1F65: return 0x1F6D;
		case 0x1F66: return 0x1F6E;
		case 0x1F67: return 0x1F6F;
		case 0x1F80: return 0x1F88;
		case 0x1F81: return 0x1F89;
		case 0x1F82: return 0x1F8A;
		case 0x1F83: return 0x1F8B;
		case 0x1F84: return 0x1F8C;
		case 0x1F85: return 0x1F8D;
		case 0x1F86: return 0x1F8E;
		case 0x1F87: return 0x1F8F;
		case 0x1F90: return 0x1F98;
		case 0x1F91: return 0x1F99;
		case 0x1F92: return 0x1F9A;
		case 0x1F93: return 0x1F9B;
		case 0x1F94: return 0x1F9C;
		case 0x1F95: return 0x1F9D;
		case 0x1F96: return 0x1F9E;
		case 0x1F97: return 0x1F9F;
		case 0x1FA0: return 0x1FA8;
		case 0x1FA1: return 0x1FA9;
		case 0x1FA2: return 0x1FAA;
		case 0x1FA3: return 0x1FAB;
		case 0x1FA4: return 0x1FAC;
		case 0x1FA5: return 0x1FAD;
		case 0x1FA6: return 0x1FAE;
		case 0x1FA7: return 0x1FAF;
		case 0x1FB0: return 0x1FB8;
		case 0x1FB1: return 0x1FB9;
		case 0x1FD0: return 0x1FD8;
		case 0x1FD1: return 0x1FD9;
		case 0x1FE0: return 0x1FE8;
		case 0x1FE1: return 0x1FE9;
		case 0x24D0: return 0x24B6;
		case 0x24D1: return 0x24B7;
		case 0x24D2: return 0x24B8;
		case 0x24D3: return 0x24B9;
		case 0x24D4: return 0x24BA;
		case 0x24D5: return 0x24BB;
		case 0x24D6: return 0x24BC;
		case 0x24D7: return 0x24BD;
		case 0x24D8: return 0x24BE;
		case 0x24D9: return 0x24BF;
		case 0x24DA: return 0x24C0;
		case 0x24DB: return 0x24C1;
		case 0x24DC: return 0x24C2;
		case 0x24DD: return 0x24C3;
		case 0x24DE: return 0x24C4;
		case 0x24DF: return 0x24C5;
		case 0x24E0: return 0x24C6;
		case 0x24E1: return 0x24C7;
		case 0x24E2: return 0x24C8;
		case 0x24E3: return 0x24C9;
		case 0x24E4: return 0x24CA;
		case 0x24E5: return 0x24CB;
		case 0x24E6: return 0x24CC;
		case 0x24E7: return 0x24CD;
		case 0x24E8: return 0x24CE;
		case 0x24E9: return 0x24CF;
		case 0xFF41: return 0xFF21;
		case 0xFF42: return 0xFF22;
		case 0xFF43: return 0xFF23;
		case 0xFF44: return 0xFF24;
		case 0xFF45: return 0xFF25;
		case 0xFF46: return 0xFF26;
		case 0xFF47: return 0xFF27;
		case 0xFF48: return 0xFF28;
		case 0xFF49: return 0xFF29;
		case 0xFF4A: return 0xFF2A;
		case 0xFF4B: return 0xFF2B;
		case 0xFF4C: return 0xFF2C;
		case 0xFF4D: return 0xFF2D;
		case 0xFF4E: return 0xFF2E;
		case 0xFF4F: return 0xFF2F;
		case 0xFF50: return 0xFF30;
		case 0xFF51: return 0xFF31;
		case 0xFF52: return 0xFF32;
		case 0xFF53: return 0xFF33;
		case 0xFF54: return 0xFF34;
		case 0xFF55: return 0xFF35;
		case 0xFF56: return 0xFF36;
		case 0xFF57: return 0xFF37;
		case 0xFF58: return 0xFF38;
		case 0xFF59: return 0xFF39;
		case 0xFF5A: return 0xFF3A;
		case 0x0:    return 0x0;
		default:     return c;
	};
};

wchar_t string::toLower(wchar_t c) {
	switch(c) {
		case 0x41: return 0x61;
		case 0x42: return 0x62;
		case 0x43: return 0x63;
		case 0x44: return 0x64;
		case 0x45: return 0x65;
		case 0x46: return 0x66;
		case 0x47: return 0x67;
		case 0x48: return 0x68;
		case 0x49: return 0x69;
		case 0x4A: return 0x6A;
		case 0x4B: return 0x6B;
		case 0x4C: return 0x6C;
		case 0x4D: return 0x6D;
		case 0x4E: return 0x6E;
		case 0x4F: return 0x6F;
		case 0x50: return 0x70;
		case 0x51: return 0x71;
		case 0x52: return 0x72;
		case 0x53: return 0x73;
		case 0x54: return 0x74;
		case 0x55: return 0x75;
		case 0x56: return 0x76;
		case 0x57: return 0x77;
		case 0x58: return 0x78;
		case 0x59: return 0x79;
		case 0x5A: return 0x7A;
		case 0xC0: return 0xE0;
		case 0xC1: return 0xE1;
		case 0xC2: return 0xE2;
		case 0xC3: return 0xE3;
		case 0xC4: return 0xE4;
		case 0xC5: return 0xE5;
		case 0xC6: return 0xE6;
		case 0xC7: return 0xE7;
		case 0xC8: return 0xE8;
		case 0xC9: return 0xE9;
		case 0xCA: return 0xEA;
		case 0xCB: return 0xEB;
		case 0xCC: return 0xEC;
		case 0xCD: return 0xED;
		case 0xCE: return 0xEE;
		case 0xCF: return 0xEF;
		case 0xD0: return 0xF0;
		case 0xD1: return 0xF1;
		case 0xD2: return 0xF2;
		case 0xD3: return 0xF3;
		case 0xD4: return 0xF4;
		case 0xD5: return 0xF5;
		case 0xD6: return 0xF6;
		case 0xD8: return 0xF8;
		case 0xD9: return 0xF9;
		case 0xDA: return 0xFA;
		case 0xDB: return 0xFB;
		case 0xDC: return 0xFC;
		case 0xDD: return 0xFD;
		case 0xDE: return 0xFE;
		case 0x178: return 0xFF;
		case 0x100: return 0x101;
		case 0x102: return 0x103;
		case 0x104: return 0x105;
		case 0x106: return 0x107;
		case 0x108: return 0x109;
		case 0x10A: return 0x10B;
		case 0x10C: return 0x10D;
		case 0x10E: return 0x10F;
		case 0x110: return 0x111;
		case 0x112: return 0x113;
		case 0x114: return 0x115;
		case 0x116: return 0x117;
		case 0x118: return 0x119;
		case 0x11A: return 0x11B;
		case 0x11C: return 0x11D;
		case 0x11E: return 0x11F;
		case 0x120: return 0x121;
		case 0x122: return 0x123;
		case 0x124: return 0x125;
		case 0x126: return 0x127;
		case 0x128: return 0x129;
		case 0x12A: return 0x12B;
		case 0x12C: return 0x12D;
		case 0x12E: return 0x12F;
		case 0x130: return 0x131;
		case 0x132: return 0x133;
		case 0x134: return 0x135;
		case 0x136: return 0x137;
		case 0x139: return 0x13A;
		case 0x13B: return 0x13C;
		case 0x13D: return 0x13E;
		case 0x13F: return 0x140;
		case 0x141: return 0x142;
		case 0x143: return 0x144;
		case 0x145: return 0x146;
		case 0x147: return 0x148;
		case 0x14A: return 0x14B;
		case 0x14C: return 0x14D;
		case 0x14E: return 0x14F;
		case 0x150: return 0x151;
		case 0x152: return 0x153;
		case 0x154: return 0x155;
		case 0x156: return 0x157;
		case 0x158: return 0x159;
		case 0x15A: return 0x15B;
		case 0x15C: return 0x15D;
		case 0x15E: return 0x15F;
		case 0x160: return 0x161;
		case 0x162: return 0x163;
		case 0x164: return 0x165;
		case 0x166: return 0x167;
		case 0x168: return 0x169;
		case 0x16A: return 0x16B;
		case 0x16C: return 0x16D;
		case 0x16E: return 0x16F;
		case 0x170: return 0x171;
		case 0x172: return 0x173;
		case 0x174: return 0x175;
		case 0x176: return 0x177;
		case 0x179: return 0x17A;
		case 0x17B: return 0x17C;
		case 0x17D: return 0x17E;
		case 0x182: return 0x183;
		case 0x184: return 0x185;
		case 0x187: return 0x188;
		case 0x18B: return 0x18C;
		case 0x191: return 0x192;
		case 0x198: return 0x199;
		case 0x1A0: return 0x1A1;
		case 0x1A2: return 0x1A3;
		case 0x1A4: return 0x1A5;
		case 0x1A7: return 0x1A8;
		case 0x1AC: return 0x1AD;
		case 0x1AF: return 0x1B0;
		case 0x1B3: return 0x1B4;
		case 0x1B5: return 0x1B6;
		case 0x1B8: return 0x1B9;
		case 0x1BC: return 0x1BD;
		case 0x1C4: return 0x1C6;
		case 0x1C7: return 0x1C9;
		case 0x1CA: return 0x1CC;
		case 0x1CD: return 0x1CE;
		case 0x1CF: return 0x1D0;
		case 0x1D1: return 0x1D2;
		case 0x1D3: return 0x1D4;
		case 0x1D5: return 0x1D6;
		case 0x1D7: return 0x1D8;
		case 0x1D9: return 0x1DA;
		case 0x1DB: return 0x1DC;
		case 0x1DE: return 0x1DF;
		case 0x1E0: return 0x1E1;
		case 0x1E2: return 0x1E3;
		case 0x1E4: return 0x1E5;
		case 0x1E6: return 0x1E7;
		case 0x1E8: return 0x1E9;
		case 0x1EA: return 0x1EB;
		case 0x1EC: return 0x1ED;
		case 0x1EE: return 0x1EF;
		case 0x1F1: return 0x1F3;
		case 0x1F4: return 0x1F5;
		case 0x1FA: return 0x1FB;
		case 0x1FC: return 0x1FD;
		case 0x1FE: return 0x1FF;
		case 0x200: return 0x201;
		case 0x202: return 0x203;
		case 0x204: return 0x205;
		case 0x206: return 0x207;
		case 0x208: return 0x209;
		case 0x20A: return 0x20B;
		case 0x20C: return 0x20D;
		case 0x20E: return 0x20F;
		case 0x210: return 0x211;
		case 0x212: return 0x213;
		case 0x214: return 0x215;
		case 0x216: return 0x217;
		case 0x181: return 0x253;
		case 0x186: return 0x254;
		case 0x18A: return 0x257;
		case 0x18E: return 0x258;
		case 0x18F: return 0x259;
		case 0x190: return 0x25B;
		case 0x193: return 0x260;
		case 0x194: return 0x263;
		case 0x197: return 0x268;
		case 0x196: return 0x269;
		case 0x19C: return 0x26F;
		case 0x19D: return 0x272;
		case 0x19F: return 0x275;
		case 0x1A9: return 0x283;
		case 0x1AE: return 0x288;
		case 0x1B1: return 0x28A;
		case 0x1B2: return 0x28B;
		case 0x1B7: return 0x292;
		case 0x386: return 0x3AC;
		case 0x388: return 0x3AD;
		case 0x389: return 0x3AE;
		case 0x38A: return 0x3AF;
		case 0x391: return 0x3B1;
		case 0x392: return 0x3B2;
		case 0x393: return 0x3B3;
		case 0x394: return 0x3B4;
		case 0x395: return 0x3B5;
		case 0x396: return 0x3B6;
		case 0x397: return 0x3B7;
		case 0x398: return 0x3B8;
		case 0x399: return 0x3B9;
		case 0x39A: return 0x3BA;
		case 0x39B: return 0x3BB;
		case 0x39C: return 0x3BC;
		case 0x39D: return 0x3BD;
		case 0x39E: return 0x3BE;
		case 0x39F: return 0x3BF;
		case 0x3A0: return 0x3C0;
		case 0x3A1: return 0x3C1;
		case 0x3A3: return 0x3C3;
		case 0x3A4: return 0x3C4;
		case 0x3A5: return 0x3C5;
		case 0x3A6: return 0x3C6;
		case 0x3A7: return 0x3C7;
		case 0x3A8: return 0x3C8;
		case 0x3A9: return 0x3C9;
		case 0x3AA: return 0x3CA;
		case 0x3AB: return 0x3CB;
		case 0x38C: return 0x3CC;
		case 0x38E: return 0x3CD;
		case 0x38F: return 0x3CE;
		case 0x3E2: return 0x3E3;
		case 0x3E4: return 0x3E5;
		case 0x3E6: return 0x3E7;
		case 0x3E8: return 0x3E9;
		case 0x3EA: return 0x3EB;
		case 0x3EC: return 0x3ED;
		case 0x3EE: return 0x3EF;
		case 0x410: return 0x430;
		case 0x411: return 0x431;
		case 0x412: return 0x432;
		case 0x413: return 0x433;
		case 0x414: return 0x434;
		case 0x415: return 0x435;
		case 0x416: return 0x436;
		case 0x417: return 0x437;
		case 0x418: return 0x438;
		case 0x419: return 0x439;
		case 0x41A: return 0x43A;
		case 0x41B: return 0x43B;
		case 0x41C: return 0x43C;
		case 0x41D: return 0x43D;
		case 0x41E: return 0x43E;
		case 0x41F: return 0x43F;
		case 0x420: return 0x440;
		case 0x421: return 0x441;
		case 0x422: return 0x442;
		case 0x423: return 0x443;
		case 0x424: return 0x444;
		case 0x425: return 0x445;
		case 0x426: return 0x446;
		case 0x427: return 0x447;
		case 0x428: return 0x448;
		case 0x429: return 0x449;
		case 0x42A: return 0x44A;
		case 0x42B: return 0x44B;
		case 0x42C: return 0x44C;
		case 0x42D: return 0x44D;
		case 0x42E: return 0x44E;
		case 0x42F: return 0x44F;
		case 0x401: return 0x451;
		case 0x402: return 0x452;
		case 0x403: return 0x453;
		case 0x404: return 0x454;
		case 0x405: return 0x455;
		case 0x406: return 0x456;
		case 0x407: return 0x457;
		case 0x408: return 0x458;
		case 0x409: return 0x459;
		case 0x40A: return 0x45A;
		case 0x40B: return 0x45B;
		case 0x40C: return 0x45C;
		case 0x40E: return 0x45E;
		case 0x40F: return 0x45F;
		case 0x460: return 0x461;
		case 0x462: return 0x463;
		case 0x464: return 0x465;
		case 0x466: return 0x467;
		case 0x468: return 0x469;
		case 0x46A: return 0x46B;
		case 0x46C: return 0x46D;
		case 0x46E: return 0x46F;
		case 0x470: return 0x471;
		case 0x472: return 0x473;
		case 0x474: return 0x475;
		case 0x476: return 0x477;
		case 0x478: return 0x479;
		case 0x47A: return 0x47B;
		case 0x47C: return 0x47D;
		case 0x47E: return 0x47F;
		case 0x480: return 0x481;
		case 0x490: return 0x491;
		case 0x492: return 0x493;
		case 0x494: return 0x495;
		case 0x496: return 0x497;
		case 0x498: return 0x499;
		case 0x49A: return 0x49B;
		case 0x49C: return 0x49D;
		case 0x49E: return 0x49F;
		case 0x4A0: return 0x4A1;
		case 0x4A2: return 0x4A3;
		case 0x4A4: return 0x4A5;
		case 0x4A6: return 0x4A7;
		case 0x4A8: return 0x4A9;
		case 0x4AA: return 0x4AB;
		case 0x4AC: return 0x4AD;
		case 0x4AE: return 0x4AF;
		case 0x4B0: return 0x4B1;
		case 0x4B2: return 0x4B3;
		case 0x4B4: return 0x4B5;
		case 0x4B6: return 0x4B7;
		case 0x4B8: return 0x4B9;
		case 0x4BA: return 0x4BB;
		case 0x4BC: return 0x4BD;
		case 0x4BE: return 0x4BF;
		case 0x4C1: return 0x4C2;
		case 0x4C3: return 0x4C4;
		case 0x4C7: return 0x4C8;
		case 0x4CB: return 0x4CC;
		case 0x4D0: return 0x4D1;
		case 0x4D2: return 0x4D3;
		case 0x4D4: return 0x4D5;
		case 0x4D6: return 0x4D7;
		case 0x4D8: return 0x4D9;
		case 0x4DA: return 0x4DB;
		case 0x4DC: return 0x4DD;
		case 0x4DE: return 0x4DF;
		case 0x4E0: return 0x4E1;
		case 0x4E2: return 0x4E3;
		case 0x4E4: return 0x4E5;
		case 0x4E6: return 0x4E7;
		case 0x4E8: return 0x4E9;
		case 0x4EA: return 0x4EB;
		case 0x4EE: return 0x4EF;
		case 0x4F0: return 0x4F1;
		case 0x4F2: return 0x4F3;
		case 0x4F4: return 0x4F5;
		case 0x4F8: return 0x4F9;
		case 0x531: return 0x561;
		case 0x532: return 0x562;
		case 0x533: return 0x563;
		case 0x534: return 0x564;
		case 0x535: return 0x565;
		case 0x536: return 0x566;
		case 0x537: return 0x567;
		case 0x538: return 0x568;
		case 0x539: return 0x569;
		case 0x53A: return 0x56A;
		case 0x53B: return 0x56B;
		case 0x53C: return 0x56C;
		case 0x53D: return 0x56D;
		case 0x53E: return 0x56E;
		case 0x53F: return 0x56F;
		case 0x540: return 0x570;
		case 0x541: return 0x571;
		case 0x542: return 0x572;
		case 0x543: return 0x573;
		case 0x544: return 0x574;
		case 0x545: return 0x575;
		case 0x546: return 0x576;
		case 0x547: return 0x577;
		case 0x548: return 0x578;
		case 0x549: return 0x579;
		case 0x54A: return 0x57A;
		case 0x54B: return 0x57B;
		case 0x54C: return 0x57C;
		case 0x54D: return 0x57D;
		case 0x54E: return 0x57E;
		case 0x54F: return 0x57F;
		case 0x550: return 0x580;
		case 0x551: return 0x581;
		case 0x552: return 0x582;
		case 0x553: return 0x583;
		case 0x554: return 0x584;
		case 0x555: return 0x585;
		case 0x556: return 0x586;
		case 0x10A0: return 0x10D0;
		case 0x10A1: return 0x10D1;
		case 0x10A2: return 0x10D2;
		case 0x10A3: return 0x10D3;
		case 0x10A4: return 0x10D4;
		case 0x10A5: return 0x10D5;
		case 0x10A6: return 0x10D6;
		case 0x10A7: return 0x10D7;
		case 0x10A8: return 0x10D8;
		case 0x10A9: return 0x10D9;
		case 0x10AA: return 0x10DA;
		case 0x10AB: return 0x10DB;
		case 0x10AC: return 0x10DC;
		case 0x10AD: return 0x10DD;
		case 0x10AE: return 0x10DE;
		case 0x10AF: return 0x10DF;
		case 0x10B0: return 0x10E0;
		case 0x10B1: return 0x10E1;
		case 0x10B2: return 0x10E2;
		case 0x10B3: return 0x10E3;
		case 0x10B4: return 0x10E4;
		case 0x10B5: return 0x10E5;
		case 0x10B6: return 0x10E6;
		case 0x10B7: return 0x10E7;
		case 0x10B8: return 0x10E8;
		case 0x10B9: return 0x10E9;
		case 0x10BA: return 0x10EA;
		case 0x10BB: return 0x10EB;
		case 0x10BC: return 0x10EC;
		case 0x10BD: return 0x10ED;
		case 0x10BE: return 0x10EE;
		case 0x10BF: return 0x10EF;
		case 0x10C0: return 0x10F0;
		case 0x10C1: return 0x10F1;
		case 0x10C2: return 0x10F2;
		case 0x10C3: return 0x10F3;
		case 0x10C4: return 0x10F4;
		case 0x10C5: return 0x10F5;
		case 0x1E00: return 0x1E01;
		case 0x1E02: return 0x1E03;
		case 0x1E04: return 0x1E05;
		case 0x1E06: return 0x1E07;
		case 0x1E08: return 0x1E09;
		case 0x1E0A: return 0x1E0B;
		case 0x1E0C: return 0x1E0D;
		case 0x1E0E: return 0x1E0F;
		case 0x1E10: return 0x1E11;
		case 0x1E12: return 0x1E13;
		case 0x1E14: return 0x1E15;
		case 0x1E16: return 0x1E17;
		case 0x1E18: return 0x1E19;
		case 0x1E1A: return 0x1E1B;
		case 0x1E1C: return 0x1E1D;
		case 0x1E1E: return 0x1E1F;
		case 0x1E20: return 0x1E21;
		case 0x1E22: return 0x1E23;
		case 0x1E24: return 0x1E25;
		case 0x1E26: return 0x1E27;
		case 0x1E28: return 0x1E29;
		case 0x1E2A: return 0x1E2B;
		case 0x1E2C: return 0x1E2D;
		case 0x1E2E: return 0x1E2F;
		case 0x1E30: return 0x1E31;
		case 0x1E32: return 0x1E33;
		case 0x1E34: return 0x1E35;
		case 0x1E36: return 0x1E37;
		case 0x1E38: return 0x1E39;
		case 0x1E3A: return 0x1E3B;
		case 0x1E3C: return 0x1E3D;
		case 0x1E3E: return 0x1E3F;
		case 0x1E40: return 0x1E41;
		case 0x1E42: return 0x1E43;
		case 0x1E44: return 0x1E45;
		case 0x1E46: return 0x1E47;
		case 0x1E48: return 0x1E49;
		case 0x1E4A: return 0x1E4B;
		case 0x1E4C: return 0x1E4D;
		case 0x1E4E: return 0x1E4F;
		case 0x1E50: return 0x1E51;
		case 0x1E52: return 0x1E53;
		case 0x1E54: return 0x1E55;
		case 0x1E56: return 0x1E57;
		case 0x1E58: return 0x1E59;
		case 0x1E5A: return 0x1E5B;
		case 0x1E5C: return 0x1E5D;
		case 0x1E5E: return 0x1E5F;
		case 0x1E60: return 0x1E61;
		case 0x1E62: return 0x1E63;
		case 0x1E64: return 0x1E65;
		case 0x1E66: return 0x1E67;
		case 0x1E68: return 0x1E69;
		case 0x1E6A: return 0x1E6B;
		case 0x1E6C: return 0x1E6D;
		case 0x1E6E: return 0x1E6F;
		case 0x1E70: return 0x1E71;
		case 0x1E72: return 0x1E73;
		case 0x1E74: return 0x1E75;
		case 0x1E76: return 0x1E77;
		case 0x1E78: return 0x1E79;
		case 0x1E7A: return 0x1E7B;
		case 0x1E7C: return 0x1E7D;
		case 0x1E7E: return 0x1E7F;
		case 0x1E80: return 0x1E81;
		case 0x1E82: return 0x1E83;
		case 0x1E84: return 0x1E85;
		case 0x1E86: return 0x1E87;
		case 0x1E88: return 0x1E89;
		case 0x1E8A: return 0x1E8B;
		case 0x1E8C: return 0x1E8D;
		case 0x1E8E: return 0x1E8F;
		case 0x1E90: return 0x1E91;
		case 0x1E92: return 0x1E93;
		case 0x1E94: return 0x1E95;
		case 0x1EA0: return 0x1EA1;
		case 0x1EA2: return 0x1EA3;
		case 0x1EA4: return 0x1EA5;
		case 0x1EA6: return 0x1EA7;
		case 0x1EA8: return 0x1EA9;
		case 0x1EAA: return 0x1EAB;
		case 0x1EAC: return 0x1EAD;
		case 0x1EAE: return 0x1EAF;
		case 0x1EB0: return 0x1EB1;
		case 0x1EB2: return 0x1EB3;
		case 0x1EB4: return 0x1EB5;
		case 0x1EB6: return 0x1EB7;
		case 0x1EB8: return 0x1EB9;
		case 0x1EBA: return 0x1EBB;
		case 0x1EBC: return 0x1EBD;
		case 0x1EBE: return 0x1EBF;
		case 0x1EC0: return 0x1EC1;
		case 0x1EC2: return 0x1EC3;
		case 0x1EC4: return 0x1EC5;
		case 0x1EC6: return 0x1EC7;
		case 0x1EC8: return 0x1EC9;
		case 0x1ECA: return 0x1ECB;
		case 0x1ECC: return 0x1ECD;
		case 0x1ECE: return 0x1ECF;
		case 0x1ED0: return 0x1ED1;
		case 0x1ED2: return 0x1ED3;
		case 0x1ED4: return 0x1ED5;
		case 0x1ED6: return 0x1ED7;
		case 0x1ED8: return 0x1ED9;
		case 0x1EDA: return 0x1EDB;
		case 0x1EDC: return 0x1EDD;
		case 0x1EDE: return 0x1EDF;
		case 0x1EE0: return 0x1EE1;
		case 0x1EE2: return 0x1EE3;
		case 0x1EE4: return 0x1EE5;
		case 0x1EE6: return 0x1EE7;
		case 0x1EE8: return 0x1EE9;
		case 0x1EEA: return 0x1EEB;
		case 0x1EEC: return 0x1EED;
		case 0x1EEE: return 0x1EEF;
		case 0x1EF0: return 0x1EF1;
		case 0x1EF2: return 0x1EF3;
		case 0x1EF4: return 0x1EF5;
		case 0x1EF6: return 0x1EF7;
		case 0x1EF8: return 0x1EF9;
		case 0x1F08: return 0x1F00;
		case 0x1F09: return 0x1F01;
		case 0x1F0A: return 0x1F02;
		case 0x1F0B: return 0x1F03;
		case 0x1F0C: return 0x1F04;
		case 0x1F0D: return 0x1F05;
		case 0x1F0E: return 0x1F06;
		case 0x1F0F: return 0x1F07;
		case 0x1F18: return 0x1F10;
		case 0x1F19: return 0x1F11;
		case 0x1F1A: return 0x1F12;
		case 0x1F1B: return 0x1F13;
		case 0x1F1C: return 0x1F14;
		case 0x1F1D: return 0x1F15;
		case 0x1F28: return 0x1F20;
		case 0x1F29: return 0x1F21;
		case 0x1F2A: return 0x1F22;
		case 0x1F2B: return 0x1F23;
		case 0x1F2C: return 0x1F24;
		case 0x1F2D: return 0x1F25;
		case 0x1F2E: return 0x1F26;
		case 0x1F2F: return 0x1F27;
		case 0x1F38: return 0x1F30;
		case 0x1F39: return 0x1F31;
		case 0x1F3A: return 0x1F32;
		case 0x1F3B: return 0x1F33;
		case 0x1F3C: return 0x1F34;
		case 0x1F3D: return 0x1F35;
		case 0x1F3E: return 0x1F36;
		case 0x1F3F: return 0x1F37;
		case 0x1F48: return 0x1F40;
		case 0x1F49: return 0x1F41;
		case 0x1F4A: return 0x1F42;
		case 0x1F4B: return 0x1F43;
		case 0x1F4C: return 0x1F44;
		case 0x1F4D: return 0x1F45;
		case 0x1F59: return 0x1F51;
		case 0x1F5B: return 0x1F53;
		case 0x1F5D: return 0x1F55;
		case 0x1F5F: return 0x1F57;
		case 0x1F68: return 0x1F60;
		case 0x1F69: return 0x1F61;
		case 0x1F6A: return 0x1F62;
		case 0x1F6B: return 0x1F63;
		case 0x1F6C: return 0x1F64;
		case 0x1F6D: return 0x1F65;
		case 0x1F6E: return 0x1F66;
		case 0x1F6F: return 0x1F67;
		case 0x1F88: return 0x1F80;
		case 0x1F89: return 0x1F81;
		case 0x1F8A: return 0x1F82;
		case 0x1F8B: return 0x1F83;
		case 0x1F8C: return 0x1F84;
		case 0x1F8D: return 0x1F85;
		case 0x1F8E: return 0x1F86;
		case 0x1F8F: return 0x1F87;
		case 0x1F98: return 0x1F90;
		case 0x1F99: return 0x1F91;
		case 0x1F9A: return 0x1F92;
		case 0x1F9B: return 0x1F93;
		case 0x1F9C: return 0x1F94;
		case 0x1F9D: return 0x1F95;
		case 0x1F9E: return 0x1F96;
		case 0x1F9F: return 0x1F97;
		case 0x1FA8: return 0x1FA0;
		case 0x1FA9: return 0x1FA1;
		case 0x1FAA: return 0x1FA2;
		case 0x1FAB: return 0x1FA3;
		case 0x1FAC: return 0x1FA4;
		case 0x1FAD: return 0x1FA5;
		case 0x1FAE: return 0x1FA6;
		case 0x1FAF: return 0x1FA7;
		case 0x1FB8: return 0x1FB0;
		case 0x1FB9: return 0x1FB1;
		case 0x1FD8: return 0x1FD0;
		case 0x1FD9: return 0x1FD1;
		case 0x1FE8: return 0x1FE0;
		case 0x1FE9: return 0x1FE1;
		case 0x24B6: return 0x24D0;
		case 0x24B7: return 0x24D1;
		case 0x24B8: return 0x24D2;
		case 0x24B9: return 0x24D3;
		case 0x24BA: return 0x24D4;
		case 0x24BB: return 0x24D5;
		case 0x24BC: return 0x24D6;
		case 0x24BD: return 0x24D7;
		case 0x24BE: return 0x24D8;
		case 0x24BF: return 0x24D9;
		case 0x24C0: return 0x24DA;
		case 0x24C1: return 0x24DB;
		case 0x24C2: return 0x24DC;
		case 0x24C3: return 0x24DD;
		case 0x24C4: return 0x24DE;
		case 0x24C5: return 0x24DF;
		case 0x24C6: return 0x24E0;
		case 0x24C7: return 0x24E1;
		case 0x24C8: return 0x24E2;
		case 0x24C9: return 0x24E3;
		case 0x24CA: return 0x24E4;
		case 0x24CB: return 0x24E5;
		case 0x24CC: return 0x24E6;
		case 0x24CD: return 0x24E7;
		case 0x24CE: return 0x24E8;
		case 0x24CF: return 0x24E9;
		case 0xFF21: return 0xFF41;
		case 0xFF22: return 0xFF42;
		case 0xFF23: return 0xFF43;
		case 0xFF24: return 0xFF44;
		case 0xFF25: return 0xFF45;
		case 0xFF26: return 0xFF46;
		case 0xFF27: return 0xFF47;
		case 0xFF28: return 0xFF48;
		case 0xFF29: return 0xFF49;
		case 0xFF2A: return 0xFF4A;
		case 0xFF2B: return 0xFF4B;
		case 0xFF2C: return 0xFF4C;
		case 0xFF2D: return 0xFF4D;
		case 0xFF2E: return 0xFF4E;
		case 0xFF2F: return 0xFF4F;
		case 0xFF30: return 0xFF50;
		case 0xFF31: return 0xFF51;
		case 0xFF32: return 0xFF52;
		case 0xFF33: return 0xFF53;
		case 0xFF34: return 0xFF54;
		case 0xFF35: return 0xFF55;
		case 0xFF36: return 0xFF56;
		case 0xFF37: return 0xFF57;
		case 0xFF38: return 0xFF58;
		case 0xFF39: return 0xFF59;
		case 0xFF3A: return 0xFF5A;
		case 0x0:    return 0x0;
		default:     return c;
	};
};

string string::toUpperCase() {
	string s;
	
	for (int i = 0; i < length; ++i)
		s += string::toUpper(buffer[i]);
	
	return s;
};

string string::toLowerCase() {
	string s;
	
	for (int i = 0; i < length; ++i)
		s += string::toLower(buffer[i]);
	
	return s;
};

string string::fromCString(const char *s) {
	return string(s);
};

string string::fromString(const wchar_t *s) {
	return string() + s;
};

string string::toString(int i) {
	char buffer [64];
	sprintf(buffer, "%d", i);
	
	string s(buffer);
	
	return s;
};

string string::toString(long l) {
	char buffer [64];
	sprintf(buffer, "%ld", l);
	
	string s(buffer);
	
	return s;
};

string string::toString(long long l, int b) {
	if (b < 2 || b > 36)
		return string();
	
	string s;
	int tmp  = l;
	int tmp2 = l;
	
	do {
		tmp = l;
		l  /= b;
		
		if (tmp || tmp == 0 && s.length == 0)
			s += "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[35 + (tmp - l * b)];
	} while(tmp);
	
	if (tmp2 < 0)
		s += '-';
	
	s.reverse();
	
	return s;
};

string string::toString(double d) {
	char buffer [64];
	sprintf(buffer, "%d", d);
	
	string s(buffer);
	
	return s;
};

long long string::toInt(int base, int def) {
	if (base < 2)
		return def;
	if (length == 0)
		return def;
	
	long long ret = 0;
	bool neg = buffer[0] == '-';
	int i = 0;
	
	if (buffer[0] == '-' || buffer[0] == '+')
		if (length == 1)
			return def;
		else
			i++;
	
	for (; i < length; i++) {
		if (base <= 10 && (buffer[i] < '0' || buffer[i] > '0' + base - 1))
			return def;
		else if ((base > 10 && (buffer[i] < '0' || buffer[i] > '9'))
				&&
				(buffer[i] < 'a' || buffer[i] > 'a' + base - 11)
				&&
				(buffer[i] < 'A' || buffer[i] > 'A' + base - 11))
			return def;
		else if (buffer[i] >= '0' && buffer[i] <= '9' && base > buffer[i] - '0') {
			ret *= base;
			ret += buffer[i] - '0';
		} else if (buffer[i] >= 'a' && buffer[i] <= 'z' && base > buffer[i] - 'a') {
			ret *= base;
			ret += buffer[i] - 'a' + 10;
		} else if (buffer[i] >= 'A' && buffer[i] <= 'Z' && base > buffer[i] - 'A') {
			ret *= base;
			ret += buffer[i] - 'A' + 10;
		} else
			return def;
	}
	
	if (neg)
		ret = -ret;
	
	return ret;
};

double string::toDouble(double def) {
	double dbl_one = 0;
	double dbl_two = 0;
	double dbl_final = 0;
	bool dec_pt = false;
	int nums_before_dec = 0;
	int nums_after_dec = 0;
	
	if (buffer[0] == '0' && buffer[1] == '0') {
		return def;
	}
	
	for (int i = 0; i < length; ++i) {
		if (buffer[i] >= '0' && buffer[i] <= '9');
		else if (buffer[i] == '.') {
			if (dec_pt) {	
				return def;
			} else {
				dec_pt = true; 
			}
		} else {
			return def;
		}
	}
	
	if (dec_pt) {
		for (int i = 0; i < length; ++i) {
			if (buffer[i + 1] != '.') {
				++nums_before_dec;
			} else {
				++nums_before_dec;
				break;
			}
		}
		nums_after_dec = length - nums_before_dec;
		--nums_after_dec;
	} else {
		nums_after_dec = 0;
		nums_before_dec = length;
	}
	
	for (int i = 0; i < nums_before_dec; ++i) {
		dbl_one += (buffer[i] - '0') * apow(10, (nums_before_dec - i));
	}
	
	dbl_one = dbl_one / 10; 
	for (int i = 0; i < nums_after_dec; i++) {
		dbl_two += (buffer[i] - '0') / apow(10, i + 1);
	}
	
	dbl_final = dbl_one + dbl_two;
	return dbl_final;
}

void string::print(void) {
	if (buffer == NULL)
		return;
	
	if (toCharSequence())
		printf("%ls", buffer);
	else
		printf("");
};

wchar_t *string::toCharSequence() {
	if (!length)
		return L"";
	
	if (length == size) {		
		*this += L'\0';
		--length;
		return buffer;
	} else
		buffer[length] = 0;
	
	return buffer;
};

char *string::toCString() {
    return wtoc(toCharSequence());
};

string *string::copy() {
	return new string(this);
};

void string::dispose(void) {
	delete this;
};


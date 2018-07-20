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
 * Stream wrapper for:
 * STDIN
 * File read
 * String argument
 */

#ifndef FAKESTREAM_H
#define FAKESTREAM_H

#include <cstdio>
#include <cstdlib>
#include <wchar.h>

#define INVALID (-2)

enum FAKESTREAM_TYPE {
	IN_STRING,
	IN_FILE,
	IN_STDIN
};

static wchar_t *charToWChar(const char *c) {
	int len = 0;
	while (c[len]) ++len;
	
	wchar_t *wc = (wchar_t*) calloc(len + 1, sizeof(wchar_t));
	mbstowcs(wc, c, len);
	
	return wc;
};

/*

		int i = 0;
		printf("%d\n", this->string[i]);
		while (this->string[i])
			printf("%08X : '%lc'\n", this->string[i], this->string[i]), ++i;
*/

#ifdef _WIN32
#include <fcntl.h>  
#include <io.h>  
#include <wchar.h>

#define _O_U16TEXT 0x00020000

static int fgetutf8c(FILE* f) {
    int result = 0;
    int input[6] = {};

    input[0] = fgetc(f);
    // printf("(i[0] = %d) ", input[0]);
    if (input[0] == EOF) {
        // The EOF was hit by the first character.
        result = EOF;
    }
    else if (input[0] < 0x80) {
        // the first character is the only 7 bit sequence...
        result = input[0];
    }
    else if ((input[0] & 0xC0) == 0x80) {
        // This is not the beginning of the multibyte sequence.
        return INVALID;
    }
    else if ((input[0] & 0xfe) == 0xfe) {
        // This is not a valid UTF-8 stream.
        return INVALID;
    }
    else {
        int sequence_length;
        for (sequence_length = 1; input[0] & (0x80 >> sequence_length); ++sequence_length);
        result = input[0] & ((1 << sequence_length) - 1);
        // printf("squence length = %d ", sequence_length);
        int index;
        for(index = 1; index < sequence_length; ++index) {
            input[index] = fgetc(f);
            // printf("(i[%d] = %d) ", index, input[index]);
            if (input[index] == EOF) {
                return WEOF;
            }
            result = (result << 6) | (input[index] & 0x30);
        }
    }
    return result;
};

#else
	
static int fgetutf8c(FILE* f) {
    return fgetwc(f);
};

#endif

struct FAKESTREAM {
	FAKESTREAM_TYPE type;
	
	wchar_t       *string;
	FILE            *file;
	
	int cursor;
	bool eof_;
	
	FAKESTREAM() {
		eof_ = 0;
		this->type = IN_STDIN;
	};
	
	FAKESTREAM(FILE *file) {
		eof_ = 0;
		this->type = IN_FILE;
		this->file = file;
#ifdef _WIN32
		//_setmode(_fileno(file),  _O_U16TEXT);
#endif
	};
	
	FAKESTREAM(const char *string) {
		eof_ = 0;
		this->type   = IN_STRING;
		this->string = charToWChar(string);
		this->cursor = 0;
	};
	
	~FAKESTREAM() {
		switch (type) {
			case IN_STRING:
				free(string);
				break;
				
			case IN_FILE:
				fclose(file);
				break;
		};
	};
	
	int getc() {
		if (eof_)
			return WEOF;
		
		switch(type) {
			case IN_STRING: {
				int c = string[cursor];
				if (!c) {
					eof_ = 1;
					return WEOF;
				}
				++cursor;
				return c;
			}
			
			case IN_FILE: {
				int c = fgetwc(file);
				eof_  = c == WEOF;
				return c;
			}
			
			case IN_STDIN: {
				int c = getwchar();
				if (c == WEOF || c == '\n') {
					eof_ = 1;
					return WEOF;
				}
				return c;
			}
		};
	};
};

#endif
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
 * Defines TokenStream class, that's used for tokenizing input source code.
 */

#ifndef TOKENSTREAM_H
#define TOKENSTREAM_H

#include "FakeStream.h"
#include "RawToken.h"

struct TokenStream {
	RawToken    *token;
	
	FAKESTREAM *source;
	
	int      buffer[9] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	
	int           eof_ = 0;
	int         error_ = 0;
	
	int lineno;
	
	TokenStream();
	
	void init(FAKESTREAM *source);
	
	~TokenStream();
	
	int get(int off);
	
	int next();
	
	void clear();
	
	int put(int token);
	
	int match(int charcode0);
	
	int match(int charcode0, int charcode1);
	
	int match(int charcode0, int charcode1, int charcode2);
	
	int match(int charcode0, int charcode1, int charcode2, int charcode3);
	
	int eof();
	
	int tokenizer_error(int lineno, const char *msg);
	
	// Fur strings aka 'unexpected char 'c' at line...'
	int tokenizer_error(int lineno, const char *msg1, int c, const char *msg2);
	
	int nextToken();
};


#endif
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
 * Defines representation of the single token produced by Tokenizer
 */

#ifndef RAWTOKEN_H
#define RAWTOKEN_H

#include "TokenNamespace.h"
#include "string.h"

struct RawToken {
	int       token;
	int    integerv;
	int       longv;
	int       bytev;
	int    booleanv;
	double  doublev;
	string *stringv;
	int      lineno;
	
	~RawToken() {
		delete stringv;
	};
	
	RawToken *copy() {
		RawToken *cpy = new RawToken;
		cpy->token    = this->token;
		cpy->integerv = this->integerv;
		cpy->longv    = this->longv;
		cpy->bytev    = this->bytev;
		cpy->booleanv = this->booleanv;
		cpy->doublev  = this->doublev;
		cpy->stringv  = this->stringv->copy();
		cpy->lineno   = this->lineno;
		return cpy;
	};
};

#endif
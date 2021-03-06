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


#include "TokenStream.h"
#include "FakeStream.h"
#include "RawToken.h"
#include "string.h"
#include "TokenNamespace.h"
#include "DebugUtils.h"
#include "ColoredOutput.h"

#include "exec_state.h"

static int white(int c) {
	return c == '\n' || c == '\r' || c == '\t' || c == ' ' || c == 0x0A || c == 0x0D;
};

static int alpha(int c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
};

static int digit(int c) {
	return c >= '0' && c <= '9';
};

static char strtlc(char c) {
	return (c >= 'a' && c <= 'z') ? c + 'A' - 'a' : c;
};

TokenStream::TokenStream() {
	buffer[0] = buffer[1] = buffer[2] = buffer[3] = buffer[4] = buffer[5] = buffer[6] = buffer[7] = buffer[8] = 0;
};

void TokenStream::init(FAKESTREAM *source) {
	eof_   = 0;
	error_ = 0;
	lineno = 1;
	this->source = source;
	
	this->token          = new RawToken;
	this->token->lineno  = 1;
	this->token->stringv = NULL;
	next(); next(); next(); next(); next(); 
};

TokenStream::~TokenStream() {
	delete token;
};

int TokenStream::get(int off) {
	if (off > 4 || off < -4)
		Kit_bug(__LINE__);
	return buffer[4 + off];
};

int TokenStream::next() {
	for (int i = 1; i < 9; i++)
		buffer[i - 1] = buffer[i];
	
	if (buffer[7] == TEOF)
		buffer[8] = TEOF;
	else {
		int c = source->getc();
		
		if (c == WEOF)
			buffer[8] = TEOF;
		else
			buffer[8] = c;
	}
	
	#ifdef PRINT_FILE
	if (buffer[8] != TEOF) {			
		putchar((char) buffer[8]);
	} else if (eof())
		putchar(10);
	#endif
	
	if (buffer[4] == '\n') {
		++lineno;
		return TEOL;
	}
	
	return buffer[4];
};

void TokenStream::clear() {
	delete token->stringv;
	token->token    = NONE;
	token->integerv = 0;
	token->longv    = 0;
	token->bytev    = 0;
	token->doublev  = 0.0;
	token->booleanv = false;
	token->stringv  = new string;
};

int TokenStream::put(int token) {
	//printf("put line: %d, token: %d\n", lineno, token);
	this->token->token = token;
	if (token == TRUE || token == FALSE) {
		this->token->booleanv = token == TRUE;
		this->token->token = BOOLEAN;
	}
	
	if (token == TEOF)
		return 0;
	
	return 1;
};

int TokenStream::match(int charcode0) {
	if (get(0) != charcode0)
		return false;
	next();
	return true;
};

int TokenStream::match(int charcode0, int charcode1) {
	if (get(0) != charcode0 || get(1) != charcode1)
		return false;
	next(); next();
	return true;
};

int TokenStream::match(int charcode0, int charcode1, int charcode2) {
	if (get(0) != charcode0 || get(1) != charcode1 || get(2) != charcode2)
		return false;
	next(); next(); next();
	return true;
};

int TokenStream::match(int charcode0, int charcode1, int charcode2, int charcode3) {
	if (get(0) != charcode0 || get(1) != charcode1 || get(2) != charcode2 || get(3) != charcode3)
		return false;
	next(); next(); next(); next();
	return true;
};

int TokenStream::eof() {
	return eof_ || error_;
};

int TokenStream::tokenizer_error(int lineno, const char *msg) {
	chighred;
	printf("Tokenizer error at %d : %s\n", lineno, msg);
	creset;
	
	error_ = true;
	token->token = TERR;
	return TERR;
};

// Fur strings aka 'unexpected char 'c' at line...'
int TokenStream::tokenizer_error(int lineno, const char *msg1, int c, const char *msg2) {
	printf("Tokenizer error at %d : %s '%c' (%d) %s\n", lineno, msg1, c, c, msg2);
	
	error_ = true;
	token->token = TERR;
	return TERR;
};

int TokenStream::nextToken() {
	if (eof() || !_global_exec_state)
		return put(TEOF);
	
	clear();
	
	if (get(0) == TEOF) {
		token->lineno = lineno;
		this->eof_ = 1;
		return put(TEOF);
	}

	int c  = get(0);
	int c1 = get(1);

	// Remove all whitescapes and comments
	while (white(c) || (c == '/' && ((c1 = get(1)) == '/' || c1 == '*'))) {
		if (white(c)) {
			c = next();
			c1 = get(1);
		} else if (c == '/' && c1 == '/') {
			// Skip till newline || TEOF
			int c0 = c;
			while (c0 != TEOL && c != TEOF) {
				c0 = c;
				c = next();
				c1 = get(1);
			}
			//if(c0 == TEOL)
			//	token->lineno++;
		} else if (c == '/' && c1 == '*') {
			// Skip all code till '*' '/' sequence
			// Warning: '*/' may be hidden in string. That must be ignored
			int q0 = false; // ' - quote
			int q1 = false; // " - quote
			int closed = false;
			// \/--- !(c == '*' && c1 == '/' && !q0 && !q1) &&
			while (c != TEOF) {
				c = next();
				c1 = get(1);
				if (c1 == '\'' && !q1)
					q0 = !q0;
				if (c1 == '\"' && !q0)
					q1 = !q1;

				if (c == '*' && c1 == '/' && !q0 && !q1) {
					closed = true;
					c = next();
					c = next();
					c1 = get(1);
					break;
				}
			}

			if (!closed)
				return tokenizer_error(lineno, "block comment expected to be closed");
		}
	}

	if (get(0) == TEOF) {
		token->lineno = lineno;
		this->eof_ = 1;
		return put(TEOF);
	}

	token->lineno = lineno;
	
	/* K E Y W O R D S */ {
		// Parse keywords|names
		if (alpha(c)) {
			string &stringvref = *token->stringv;
			do {
				stringvref += (char) c;
				c = next();
			} while((alpha(c) || digit(c)) && c != TEOF);

			if (stringvref == "true")
				return put(TRUE);
			if (stringvref == "false")
				return put(FALSE);
			if (stringvref == "null")
				return put(TNULL);
			if (stringvref == "undefined")
				return put(UNDEFINED);

			if (stringvref == "this")
				return put(THIS);
			if (stringvref == "self")
				return put(SELF);
			if (stringvref == "try")
				return put(TRY);
			if (stringvref == "expect")
				return put(EXPECT);
			if (stringvref == "raise")
				return put(RAISE);
			if (stringvref == "if")
				return put(IF);
			if (stringvref == "else")
				return put(ELSE);
			if (stringvref == "for")
				return put(FOR);
			if (stringvref == "switch")
				return put(SWITCH);
			if (stringvref == "case")
				return put(CASE);
			if (stringvref == "default")
				return put(DEFAULT);
			if (stringvref == "while")
				return put(WHILE);
			if (stringvref == "do")
				return put(DO);
			if (stringvref == "break")
				return put(BREAK);
			if (stringvref == "continue")
				return put(CONTINUE);
			if (stringvref == "return")
				return put(RETURN);
			if (stringvref == "function")
				return put(FUNCTION);
			if (stringvref == "prototype")
				return put(PROTOTYPE);
			if (stringvref == "var")
				return put(VAR);
			if (stringvref == "const")
				return put(CONST);
			if (stringvref == "safe")
				return put(SAFE);
			if (stringvref == "local")
				return put(LOCAL);
			if (stringvref == "new")
				return put(NEW);

			return put(NAME);
		}
	}
	
	/* S T R I N G S */ {
		// Parse string ("|')
		if (c == '\'' || c == '\"') {
			int quote = c;
			string &stringvref = *token->stringv;

			c = next();
			while (c != TEOF) {
				if (c == TEOL || c == TEOF)
					return tokenizer_error(lineno, "string expected to be closed");
				if (c == quote)
					break;
				if (c == '\\') {
					c1 = next();
					if (c1 == 't')
						stringvref += '\t';
					else if (c1 == 'b')
						stringvref += '\b';
					else if (c1 == 'n')
						stringvref += '\n';
					else if (c1 == 'r')
						stringvref += '\r';
					else if (c1 == 'f')
						stringvref += '\f';
					else if (c1 == '\\')
						stringvref += '\\';
					else if (c1 == '\'')
						stringvref += '\'';
					else if (c1 == '\"')
						stringvref += '\"';
					else if (c1 == '0')
						stringvref += '\0';
					else if (c1 == 'u') {
						c1 = next();
						// Expect 4-digit hex number
						int chpoint = 0;
						int point   = 0;
						for (int i = 0; i < 8; i++) {
							c1 = strtlc(c1);
							int cp = -1;
							if (c1 >= 'a' && c1 <= 'f')
								cp = 10 + c1 - 'a';
							if (c1 >= '0' && c1 <= '9')
								cp = c1 - '0';
							if (cp == -1 && !point)
								return tokenizer_error(lineno, "expected hexadecimal character code point", c1, "");
							if (cp == -1)
								break;
							++point;
							chpoint = chpoint * 16 + cp;
							c1 = next();
						}
					} else
						return tokenizer_error(lineno, "unexpected character after escape point", c1, "");
				} else
					stringvref += (wchar_t) c;
				c = next();
			}
			next();
			return put(STRING);
		}
	}
	
	/* N U M B E R S */ {
		if (digit(c) || (c == '.' && digit(c1))) {
			
			
			// XXX: Rewrite this code part to optimize/fix parser bugs
			
			
			int type = INTEGER;
			int hasPoint = false;
			int scientific = false;
			int base = 10;

			string &stringvref = *token->stringv;
			
			if (c == '.') {
				type = DOUBLE;
				hasPoint = true;
			}

			stringvref += (char) c;

			c = next();
			c1 = get(1);
			while (c != TEOF) {
				if (scientific)
					return tokenizer_error(lineno, "double can't be parsed after scientific notation");
				else if (c == '.') {
					if (hasPoint)
						return tokenizer_error(lineno, "double can't have more than one point");
					// if (base != 10)
					//	return tokenizer_error(lineno, "double can't be not decimal");
					if (!digit(c1))
						return tokenizer_error(lineno, "unexpected fraction of double nomber");
					type = DOUBLE;
					hasPoint = true;
					stringvref += '.';
				} else if ((c == 'x' || c == 'X') && base == 10) {
					if (type == DOUBLE)
						return tokenizer_error(lineno, "double can't be not decimal");
					// if (base != 10)
					//	return tokenizer_error(lineno, "one token.integerv can't have multiple numerical bases");
					if (stringvref != "0")
						return tokenizer_error(lineno, "base notation starts with 0");
					stringvref = "";
					base = 16;
				} else if ((c == 'o' || c == 'O') && base == 10) {
					if (type == DOUBLE)
						return tokenizer_error(lineno, "double can't be not decimal");
					// if (base != 10)
					//	return tokenizer_error(lineno, "one token.integerv can't have multiple numerical bases");
					if (stringvref != "0")
						return tokenizer_error(lineno, "base notation starts with 0");
					stringvref = "";
					base = 8;
				} else if ((c == 'b' || c == 'B') && base == 10) {
					if (type == DOUBLE)
						return tokenizer_error(lineno, "double can't be not decimal");
					// if (base != 10)
					//	return tokenizer_error(lineno, "integer can't have multiple numerical bases");
					if (stringvref != "0")
						return tokenizer_error(lineno, "base notation starts with 0");
					stringvref = "";
					base = 2;
				} else if ((c == 'e' || c == 'E') && base != 16) {
					if (base != 10)
						return tokenizer_error(lineno, "double can't be not decimal");
					type = DOUBLE;
					// Start reading scientific notation
					// double + E + (+ or -) + integer
					stringvref += 'e';
					c = next();
					if ((c == '-' || c == '+')) {
						// 12.34E26 is the same as 12.34E+26
						stringvref += (wchar_t) c;
						c = next();
					}
					string *integer = new string();
					while (digit(c)) {
						*integer += (wchar_t) c;
						c = next();
					}
					if (*integer == "")
						return tokenizer_error(lineno, "expected exponent in scientific notation");
					if (alpha(c))
						return tokenizer_error(lineno, "unexpected character in scientific notation");
					stringvref += *integer;
					delete integer;
					scientific = true;
					break;
				} else if ((c == 'l' || c == 'L' || (c == 'b' || c == 'B') && base != 16) && !digit(c1)) {
					if (c == 'l' || c == 'L')
						type = LONG;
					else
						type = BYTE;
					c = next();
					break;
				} else if (base == 16 && (('A' <= c && c <= 'F') || ('a' <= c && 'f' <= c) || digit(c)))
					stringvref += (wchar_t) c;
				else if (base == 8 && '0' <= c && c <= '7')
					stringvref += (wchar_t) c;
				else if (base == 2 && (c == '0' || c == '1'))
					stringvref += (wchar_t) c;
				else if (base == 10 && digit(c))
					stringvref += (wchar_t) c;
				else if (alpha(c) || (base == 8 && '8' <= c && c <= '9')
							      || (base == 2 && '3' <= c && c <= '9'))
					return tokenizer_error(lineno, "unexpected character in number", c, "");
				else
					break;

				c = next();
				c1 = get(1);
			}
			
			if (type == DOUBLE) {
				token->doublev = stringvref.toDouble(0);
				return put(DOUBLE);
			}
			if (type == INTEGER) {
				token->integerv = (int) stringvref.toInt(base, 0);
				return put(INTEGER);
			}
			if (type == BYTE) {
				token->longv = (unsigned char) stringvref.toInt(base, 0);
				return put(BYTE);
			}
			if (type == LONG) {
				token->longv =  (long) stringvref.toInt(base, 0);
				return put(LONG);
			}
		}
	}
	
	/* D E L I M I T E R S */ {
		// Parse delimiters
		if (match('>', '>', '>', '='))
			return put(ASSIGN_BITURSH);
		if (match('>', '>', '>'))
			return put(BITURSH);
		if (match('>', '>', '='))
			return put(ASSIGN_BITRSH);
		if (match('<', '<', '='))
			return put(ASSIGN_BITLSH);
		if (match('>', '>'))
			return put(BITRSH);
		if (match('<', '<'))
			return put(BITLSH);
		if (match('>', '='))
			return put(GE);
		if (match('<', '='))
			return put(LE);
		if (match('&', '&'))
			return put(AND);
		if (match('|', '|'))
			return put(OR);
		if (match('=', '='))
			return put(EQ);
		if (match('!', '='))
			return put(NEQ);
		if (match('-', '>'))
			return put(PUSH);
		if (match('=', '>'))
			return put(LAMBDA);
		if (match('/', '/'))
			return put(MDIV);
		if (match('+', '+'))
			return put(INC);
		if (match('-', '-'))
			return put(DEC);
		if (match('+', '='))
			return put(ASSIGN_ADD);
		if (match('-', '='))
			return put(ASSIGN_SUB);
		if (match('*', '='))
			return put(ASSIGN_MUL);
		if (match('/', '='))
			return put(ASSIGN_DIV);
		if (match('%', '='))
			return put(ASSIGN_MOD);
		if (match('|', '='))
			return put(ASSIGN_BITOR);
		if (match('&', '='))
			return put(ASSIGN_BITAND);
		if (match('^', '='))
			return put(ASSIGN_BITXOR);
		if (match('~', '='))
			return put(ASSIGN_BITNOT);
		if (match('='))
			return put(ASSIGN);
		if (match('>'))
			return put(GT);
		if (match('<'))
			return put(LT);
		if (match('+'))
			return put(PLUS);
		if (match('-'))
			return put(MINUS);
		if (match('*'))
			return put(MUL);
		if (match('/'))
			return put(DIV);
		if (match('%'))
			return put(MOD);
		if (match('~'))
			return put(BITNOT);
		if (match('|'))
			return put(BITOR);
		if (match('&'))
			return put(BITAND);
		if (match('^'))
			return put(BITXOR);
		if (match('!'))
			return put(NOT);
		if (match('.'))
			return put(DOT);
		if (match('?'))
			return put(HOOK);
		if (match(':'))
			return put(COLON);
		if (match('\\'))
			return put(PATH);
		if (match('['))
			return put(LB);
		if (match(']'))
			return put(RB);
		if (match('{'))
			return put(LC);
		if (match('}'))
			return put(RC);
		if (match('('))
			return put(LP);
		if (match(')'))
			return put(RP);
		if (match(','))
			return put(COMMA);
		if (match(';'))
			return put(SEMICOLON);
		if (match('$'))
			return put(SELF);

		if (c == '@' && alpha(c1)) {
			// Read flag token
			c = next();
			while (alpha(c) || digit(c) || c == '_') {
				token->stringv += (char) c;
				c = next();
			}
			return put(NAME);
		}
	}
	
	this->error_ = 1;
	
	printf("%c %c %c %c %c\n", c, get(-2), get(-1), get(0), get(1));
	
	return tokenizer_error(lineno, "unexpected character", c, "");
};


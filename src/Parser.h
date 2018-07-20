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
 * Defines Parser that's used for parsing all token strem into the final AST tree.
 */
 
 // XXX: Generate AVL-tree for sub-tree of operators

#ifndef PARSER_H
#define PARSER_H

#include "TokenStream.h"
#include "ASTNode.h"

struct Parser {
	TokenStream *source;
	int           eof_ = 0;
	int         error_ = 0;
	
	RawToken *buffer[7] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	
	void init(TokenStream *source);
	
	~Parser();
	
	RawToken *get(int off);
	
	RawToken *next();
	
	bool match(int token);
	
	ASTNode *noline_parser_error(const char *msg);
	
	ASTNode *parser_error(const char *msg);
	
	ASTNode *primaryexp();
	
	ASTNode *member_expression();
	
	ASTNode *unary_expression();
	
	ASTNode *multiplication_expression();
	
	ASTNode *addiction_expression();
	
	ASTNode *bitwise_shift_expression();
	
	ASTNode *comparison_expression();
	
	ASTNode *equality_expression();
	
	ASTNode *bitwise_and_expression();
	
	ASTNode *bitwise_xor_exppression();
	
	ASTNode *bitwise_or_expression();
	
	ASTNode *and_expression();
	
	ASTNode *or_expression();
	
	ASTNode *condition_expression();
	
	ASTNode *assign_expression();
	
	ASTNode *expression();
	
	ASTNode *checkNotNullExpression();
	
	bool checkNullExpression(ASTNode *exp);
	
	ASTNode *initializerstatement();
	
	bool peekStatementWithoutSemicolon();
	
	ASTNode *statement();
	
	ASTNode *statement_with_semicolons();
	
	ASTNode *parse();
	
	int lineno();
	
	int eof();
};



#endif
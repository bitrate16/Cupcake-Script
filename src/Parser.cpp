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


#include "Parser.h"
#include "TokenNamespace.h"
#include "DebugUtils.h"
#include "ColoredOutput.h"

#include "exec_state.h"


void Parser::init(TokenStream *source) {
	this->eof_ = 0;
	this->error_ = 0;
	this->source = source;
	next(); next(); next(); next();
};

Parser::~Parser() {
	for (int i = 0; i < 7; i++)
		delete this->buffer[i];
	// delete buffer;
};

RawToken *Parser::get(int off) {
	if (off > 3 || off < -3)
		Kit_bug(__LINE__);
	return buffer[3 + off];
};

RawToken *Parser::next() {
	if (this->buffer[0] != NULL)
		delete this->buffer[0];
	for (int i = 1; i < 7; i++)
		this->buffer[i - 1] = this->buffer[i];
	this->source->nextToken();
	if (this->source->token->token == TERR)
		noline_parser_error("TS Error");
	this->buffer[6] = this->source->token->copy();
	
	if (buffer[3] != NULL && buffer[3]->token == TEOF)
		eof_ = 1;
	
	return buffer[3];
};

bool Parser::match(int token) {
	if (error_)
		return 0;
	
	if (get(0)->token != token)
		return 0;
	
	next();
	return 1;	
};

ASTNode *Parser::noline_parser_error(const char *msg) {
	chighred;
	printf("Parser error : %s\n", msg);
	cwhite;
	
	error_ = true;
	return NULL;
};

ASTNode *Parser::parser_error(const char *msg) {
	chighred;
	printf("Parser error at %d : %s\n", get(0)->lineno, msg);
	creset;
	
	error_ = true;
	return NULL;
};

ASTNode *Parser::primaryexp() {
	if (match(TEOF)) 
		return parser_error("EOF Expression");
	
	if (match(INTEGER)) {
		
		// OBJECTS:
		// value
		
		DEBUG("INTEGER expression")
		ASTNode *integerexp = new ASTNode(get(-1)->lineno, INTEGER);
		
		int *n = new int;
		*n = get(-1)->integerv;
		integerexp->addLastObject(n);
		return integerexp;
	}
	
	if (match(LONG)) {
		
		// OBJECTS:
		// value
		DEBUG("LONG expression")
		ASTNode *longexp = new ASTNode(get(-1)->lineno, LONG);
		
		long *n = new long;
		*n = get(-1)->longv;
		longexp->addLastObject(n);
		
		return longexp;
	}
	
	if (match(BYTE)) {
		
		// OBJECTS:
		// value
		DEBUG("BYTE expression")
		ASTNode *byteexp = new ASTNode(get(-1)->lineno, BYTE);
		
		char *n = new char;
		*n = get(-1)->bytev;
		byteexp->addLastObject(n);
		
		return byteexp;
	}
	
	if (match(BOOLEAN)) {
		
		// OBJECTS:
		// value
		DEBUG("BOOLEAN expression")
		ASTNode *booleanexp = new ASTNode(get(-1)->lineno, BOOLEAN);
		
		bool *n = new bool;
		*n = get(-1)->booleanv;
		booleanexp->addLastObject(n);
		
		return booleanexp;
	}
	
	if (match(DOUBLE)) {
		
		// OBJECTS:
		// value
		DEBUG("DOUBLE expression")
		ASTNode *doubleexp = new ASTNode(get(-1)->lineno, DOUBLE);
		
		double *n = new double;
		*n = get(-1)->doublev;
		doubleexp->addLastObject(n);
		
		return doubleexp;
	}
	
	if (match(NAME)) {
		
		// OBJECTS:
		// value
		DEBUG("NAME expression")
		ASTNode *nameexp = new ASTNode(get(-1)->lineno, NAME);
		
		nameexp->addLastObject(get(-1)->stringv->copy());
		
		return nameexp;
	}
	
	if (match(STRING)) {
		
		// OBJECTS:
		// value
		DEBUG("STRING expression")
		ASTNode *stringexp = new ASTNode(get(-1)->lineno, STRING);
		
		stringexp->addLastObject(get(-1)->stringv->copy());
		
		return stringexp;
	}
	
	if (match(THIS)) {
		DEBUG("THIS expression")
		ASTNode *thisexp = new ASTNode(get(-1)->lineno, THIS);
		
		return thisexp;
	}
	
	if (match(SELF)) {
		DEBUG("SELF expression")
		ASTNode *selfexp = new ASTNode(get(-1)->lineno, SELF);
		
		return selfexp;
	}
	
	if (match(TNULL)) {
		DEBUG("NULL expression")
		ASTNode *nullexp = new ASTNode(get(-1)->lineno, TNULL);
		
		return nullexp;
	}
	
	if (match(UNDEFINED)) {
		DEBUG("NULL expression")
		ASTNode *undefinedexp = new ASTNode(get(-1)->lineno, UNDEFINED);
		
		return undefinedexp;
	}
	
	if (match(LP)) {
		DEBUG("(<exp>)")
		ASTNode *exp = expression();
		
		if (exp == NULL)
			return NULL;

		if (!match(RP)) {
			delete exp;
			return parser_error("Expected )");
		}
		
		return exp;
	}
	
	if (match(LB)) {
		// FRAME:
		// elem0
		// ...
		// elemN
		// OBJECTS:
		// element count
		//   _      _      _
        // >(.)__ <(.)__ =(.)__
        //  (___/  (___/  (___/  krya
		
		DEBUG("ARRAY expression")
		
		ASTNode *array = new ASTNode(get(-1)->lineno, ARRAY);
		int *length = new int;
		array->addLastObject(length);
		*length = 0;
		
		if (match(RB))
			return array;
		
		while (true) {
			ASTNode *elem = expression();
			if (checkNullExpression(elem))  {
				delete array;
				return NULL;
			}
			
			array->addLastChild(elem);
			++*length;
			
			if (match(TEOF)) {
				delete array;
				return parser_error("Expected ]");
			}
			if (!match(COMMA)) {
				if (!match(RB)) {
					delete array;
					return parser_error("Expected ]");
				} else
					break;
			}
		}
		
		return array;
	}
	
	if (match(LC)) {
		// FRAME:
		// object0
		// ...
		// objectN
		// OBJECTS:
		// nameN
		// ...
		// name0
		
		DEBUG("OBJECT expression")
		
		ASTNode *object = new ASTNode(get(-1)->lineno, OBJECT);
		
		if (match(RC))
			return object;
		
		while (true) {
		if (!(match(NAME) || match(STRING))) {
				delete object;
				return parser_error("Expected name or string");
			}
			
			string *name = get(-1)->stringv->copy();
			
			if (!match(COLON)) {
				delete name;
				delete object;
				return parser_error("Expected :");
			}
			
			ASTNode *elem = expression();
			if (checkNullExpression(elem))  {
				delete name;
				delete object;
				return NULL;
			}
			
			object->addLastObject(name);
			object->addChild(elem);
			
			if (match(TEOF)) {
				delete object;
				return parser_error("Expected }");
			}
			if (!match(COMMA)) {
				if (!match(RC)) {
					delete object;
					return parser_error("Expected }");
				} else
					break;
			}
		}
		
		return object;
	}
	
	if (match(FUNCTION)) {
		// FRAME:
		// block
		// OBJECTS:
		// arg0
		// ...
		// argN
		
		DEBUG("FUNCTION node")
		
		ASTNode *function = new ASTNode(get(-1)->lineno, FUNCTION);
		
		if (match(LP)) 
			if (!match(RP))
				while (true) {
					if (!match(NAME)) {
						delete function;
						return parser_error("Expected name");
					}
					
					function->addLastObject(get(-1)->stringv->copy());
					
					if (match(TEOF)) {
						delete function;
						return parser_error("Expected )");
					}
					if (!match(COMMA)) {
						if (!match(RP)) {
							delete function;
							return parser_error("Expected )");
						} else
							break;
					}
				}
		
		function->addChild(statement());
		
		return function;
	}
	
	printf("Parser error at %d : Unexpected token %d (%s)\n", get(0)->lineno, get(0)->token, tokenToString(get(0)->token));
	error_ = true;
	return NULL;
};

ASTNode *Parser::member_expression() {
	if (match(NEW)) {
		
		// DEPRECATED
		
		// FRAME
		// ARGUMENTS_NODE
		// \ arg0
		// | arg1
		// | ...
		// | argn
		// INITIALIZER_OBJECT
		
		return parser_error("DEPRECATED NEW");
	}

	// Stack all <primary_exp>[]..[]()..()
	
	ASTNode *exp = primaryexp();
	if (checkNullExpression(exp)) 
		return NULL;
	
	while (match(LB) || match(LP) || match(DOT)) {
		int lineno = get(-1)->lineno;
		
		switch (get(-1)->token) {
			case LB: {
				// FRAME:
				// reference
				// index expression
				
				ASTNode *ind = expression();
				if (checkNullExpression(ind)) {
					delete exp;
					return NULL;
				}
				
				ASTNode *node = new ASTNode(lineno, MEMBER);
				node->addChild(exp);
				node->addChild(ind);
				exp = node;
				
				if (!match(RB)) {
					delete exp;
					return parser_error("Expected ]");
				}
				
				break;
			}
			
			case DOT: {
				// FRAME:
				// reference
				// OBJECTS:
				// field_name
				
				ASTNode *node = new ASTNode(lineno, FIELD);
				node->addChild(exp);
				exp = node;
				
				if (!match(NAME)) {
					delete exp;
					return parser_error("Expected name");
				}
				node->addLastObject(get(-1)->stringv->copy());
				
				break;
			}
			
			case LP: {
				// FRAME:
				// reference
				// arg0
				// ...
				// argn
				
				ASTNode *node = new ASTNode(lineno, CALL);
				node->addChild(exp);
				exp = node;
				
				if (match(RP))
					break;
				
				while (true) {
					ASTNode *arg = expression();
					if (checkNullExpression(arg))  {
						delete exp;
						return NULL;
					}
					
					exp->addChild(arg);
					
					if (match(TEOF)) {
						delete exp;
						return parser_error("Expected )");
					}
					if (!match(COMMA)) {
						if (!match(RP)) {
							delete exp;
							return parser_error("Expected )");
						} else
							break;
					}
				}
				
				break;
			}
		}
	}
	
	return exp;
};

ASTNode *Parser::unary_expression() {
	
	DEBUG("UNARY expression")
	
	// FRAME:
	// exp
	
	if (match(NOT) || match(BITNOT) || match(PLUS) || match(MINUS)) {
		// ! EXP | ~EXP | -EXP | +EXP
		
		int token = get(-1)->token;
		int lineno = get(-1)->lineno;
		
		ASTNode *exp = unary_expression();
		if (checkNullExpression(exp)) 
			return NULL;
		
		ASTNode *expr = new ASTNode(lineno, token == PLUS ? POS : token == MINUS ? NEG : token);
		expr->addChild(exp);
		
		return expr;						
	} else if (match(INC) || match(DEC)) {
		// ++ EXP
		
		int token = get(-1)->token;
		int lineno = get(-1)->lineno;
		
		ASTNode *exp = member_expression();
		if (checkNullExpression(exp)) 
			return NULL;
		
		if (exp->type == NAME
			||
			exp->type == FIELD
			||
			exp->type == MEMBER) 
		{
			ASTNode *expr = new ASTNode(lineno, token == INC ? PRE_INC : PRE_DEC);
			expr->addChild(exp);
			
			return expr;			
		} else {
			delete exp;
			return parser_error("Left side of the increment expected to be field");
		}
	} else {
		// EXP | EXP ++
		
		ASTNode *exp = member_expression();
		if (checkNullExpression(exp)) 
			return NULL;
		
		if (match(INC) || match(DEC)) {
			
			if (exp->type == NAME
				||
				exp->type == FIELD
				||
				exp->type == MEMBER) 
			{
				ASTNode *expr = new ASTNode(get(-1)->lineno, get(-1)->token == INC ? POS_INC : POS_DEC);
				expr->addChild(exp);
				
				return expr;
			} else {
				delete exp;
				return parser_error("Right side of the increment expected to be field");
			}
		} return exp;
	}
};

ASTNode *Parser::multiplication_expression() {
	
	DEBUG("MULTIPLICATION expression")
	
	// FRAME:
	// exp1
	// exp2
	
	ASTNode *left = unary_expression();
	if (checkNullExpression(left)) 
		return NULL;
	
	while (1) {
		if (match(MUL) || match(DIV) || match(MDIV) || match(MOD) || match(HASH)) {
			ASTNode *exp   = new ASTNode(get(-1)->lineno, get(-1)->token);
			exp->addChild(left);
			ASTNode *right = unary_expression();
			
			if (checkNullExpression(right)) {
				delete exp;
				return NULL;
			}
			
			exp->addChild(right);
			left = exp;
			continue;
		}
		break;
	}
	
	return left;
};

ASTNode *Parser::addiction_expression() {
	
	DEBUG("ADDICTION expression")
	
	// FRAME:
	// exp1
	// exp2
	
	ASTNode *left = multiplication_expression();
	if (checkNullExpression(left)) 
		return NULL;
	
	while (1) {
		if (match(PLUS) || match(MINUS)) {
			ASTNode *exp   = new ASTNode(get(-1)->lineno, get(-1)->token);
			exp->addChild(left);
			ASTNode *right = multiplication_expression();
			
			if (checkNullExpression(right)) {
				delete exp;
				return NULL;
			}
			
			exp->addChild(right);
			left = exp;
			continue;
		}
		break;
	}
	
	return left;
};

ASTNode *Parser::bitwise_shift_expression() {
	
	DEBUG("BITWISE SHIFT expression")
	
	// FRAME:
	// exp1
	// exp2
	
	ASTNode *left = addiction_expression();
	if (checkNullExpression(left)) 
		return NULL;
	
	while (1) {
		if (match(BITRSH) || match(BITLSH) || match(BITURSH)) {
			ASTNode *exp   = new ASTNode(get(-1)->lineno, get(-1)->token);
			exp->addChild(left);
			ASTNode *right = addiction_expression();
			
			if (checkNullExpression(right)) {
				delete exp;
				return NULL;
			}
			
			exp->addChild(right);
			left = exp;
			continue;
		}
		break;
	}
	
	return left;
};

ASTNode *Parser::comparison_expression() {
	
	DEBUG("COMPARISON expression")
	
	// FRAME:
	// exp1
	// exp2
	
	ASTNode *left = bitwise_shift_expression();
	if (checkNullExpression(left)) 
		return NULL;
	
	while (1) {
		if (match(GT) || match(GE) || match(LT) || match(LE)) {
			ASTNode *exp   = new ASTNode(get(-1)->lineno, get(-1)->token);
			exp->addChild(left);
			ASTNode *right = bitwise_shift_expression();
			
			if (checkNullExpression(right)) {
				delete exp;
				return NULL;
			}
			
			exp->addChild(right);
			left = exp;
			continue;
		}
		break;
	}
	
	return left;
};

ASTNode *Parser::equality_expression() {
	
	DEBUG("EQUALITY expression")
	
	// FRAME:
	// exp1
	// exp2
	
	ASTNode *left = comparison_expression();
	if (checkNullExpression(left)) 
		return NULL;
	
	while (1) {
		if (match(EQ) || match(NEQ)) {
			ASTNode *exp   = new ASTNode(get(-1)->lineno, get(-1)->token);
			exp->addChild(left);
			ASTNode *right = comparison_expression();
			
			if (checkNullExpression(right)) {
				delete exp;
				return NULL;
			}
			
			exp->addChild(right);
			left = exp;
			continue;
		}
		break;
	}
	
	return left;
};

ASTNode *Parser::bitwise_and_expression() {
	
	DEBUG("BITWISE AND expression")
	
	// FRAME:
	// exp1
	// exp2
	
	ASTNode *left = equality_expression();
	if (checkNullExpression(left)) 
		return NULL;
	
	while (1) {
		if (match(BITAND)) {
			ASTNode *exp   = new ASTNode(get(-1)->lineno, get(-1)->token);
			exp->addChild(left);
			ASTNode *right = equality_expression();
			
			if (checkNullExpression(right)) {
				delete exp;
				return NULL;
			}
			
			exp->addChild(right);
			left = exp;
			continue;
		}
		break;
	}
	
	return left;
};

ASTNode *Parser::bitwise_xor_exppression() {
	
	DEBUG("BITWISE XOR expression")
	
	// FRAME:
	// exp1
	// exp2
	
	ASTNode *left = bitwise_and_expression();
	if (checkNullExpression(left)) 
		return NULL;
	
	while (1) {
		if (match(BITXOR)) {
			ASTNode *exp   = new ASTNode(get(-1)->lineno, get(-1)->token);
			exp->addChild(left);
			ASTNode *right = bitwise_and_expression();
			
			if (checkNullExpression(right)) {
				delete exp;
				return NULL;
			}
			
			exp->addChild(right);
			left = exp;
			continue;
		}
		break;
	}
	
	return left;
};

ASTNode *Parser::bitwise_or_expression() {
	
	DEBUG("BITWISE OR expression")
	
	// FRAME:
	// exp1
	// exp2
	
	ASTNode *left = bitwise_xor_exppression();
	if (checkNullExpression(left)) 
		return NULL;
	
	while (1) {
		if (match(BITOR)) {
			ASTNode *exp   = new ASTNode(get(-1)->lineno, get(-1)->token);
			exp->addChild(left);
			ASTNode *right = bitwise_xor_exppression();
			
			if (checkNullExpression(right)) {
				delete exp;
				return NULL;
			}
			
			exp->addChild(right);
			left = exp;
			continue;
		}
		break;
	}
	
	return left;
};

ASTNode *Parser::and_expression() {
	
	DEBUG("AND expression")
	
	// FRAME:
	// exp1
	// exp2
	
	ASTNode *left = bitwise_or_expression();
	if (checkNullExpression(left)) 
		return NULL;
	
	while (1) {
		if (match(AND)) {
			ASTNode *exp   = new ASTNode(get(-1)->lineno, get(-1)->token);
			exp->addChild(left);
			ASTNode *right = bitwise_or_expression();
			
			if (checkNullExpression(right)) {
				delete exp;
				return NULL;
			}
			
			exp->addChild(right);
			left = exp;
			continue;
		}
		break;
	}
	
	return left;
};

ASTNode *Parser::or_expression() {
	
	DEBUG("OR expression")
	
	// FRAME:
	// exp1
	// exp2
	
	ASTNode *left = and_expression();
	if (checkNullExpression(left)) 
		return NULL;
	
	while (1) {
		if (match(OR)) {
			ASTNode *exp   = new ASTNode(get(-1)->lineno, get(-1)->token);
			exp->addChild(left);
			ASTNode *right = and_expression();
			
			if (checkNullExpression(right)) {
				delete exp;
				return NULL;
			}
			
			exp->addChild(right);
			left = exp;
			continue;
		}
		break;
	}
	
	return left;
};

ASTNode *Parser::condition_expression() {
	
	DEBUG("CONDITIONAL expression")
	
	// FRAME:
	// condition
	// true-exp
	// false-exp
	
	ASTNode *or_exp = or_expression();
	if (checkNullExpression(or_exp)) 
		return NULL;
	
	ASTNode *condition_exp;
	
	if (match(HOOK))
		condition_exp = new ASTNode(get(-1)->lineno, CONDITION);
	else 
		return or_exp;
	
	condition_exp->addChild(or_exp);
	
	ASTNode *true_exp = assign_expression();
	if (checkNullExpression(true_exp)) {
		delete condition_exp;
		return NULL;
	}
	
	condition_exp->addChild(true_exp);
	
	if (!match(COLON)) {
		delete condition_exp;
		return parser_error("Expected :");
	}
	
	ASTNode *false_exp = assign_expression();
	if (checkNullExpression(false_exp)) {
		delete condition_exp;
		return NULL;
	}
	
	condition_exp->addChild(false_exp);
	
	return condition_exp;
};

ASTNode *Parser::assign_expression() {
	
	DEBUG("ASSIGN expression")
	
	// FRAME:
	// exp1
	// exp2
	
	ASTNode *condition_exp = condition_expression();
	if (checkNullExpression(condition_exp)) 
		return NULL;
	
	ASTNode *assign_exp;
	
	if (match(ASSIGN)
		||
		match(ASSIGN_ADD)
		||
		match(ASSIGN_SUB)
		||
		match(ASSIGN_MUL)
		||
		match(ASSIGN_DIV)
		||
		match(ASSIGN_BITRSH)
		||
		match(ASSIGN_BITLSH)
		||
		match(ASSIGN_BITURSH)
		||
		match(ASSIGN_BITNOT)
		||
		match(ASSIGN_MDIV)
		||
		match(ASSIGN_MOD)
		||
		match(ASSIGN_BITOR)
		||
		match(ASSIGN_BITAND)
		||
		match(ASSIGN_BITXOR)) 
	{
		if (condition_exp->type == NAME
			||
			condition_exp->type == FIELD
			||
			condition_exp->type == MEMBER)
			assign_exp = new ASTNode(get(-1)->lineno, get(-1)->token);
		else {
			delete condition_exp;
			return parser_error("Left side of the assignment expected to be field");
		}
	} else 
		return condition_exp;
	
	assign_exp->addChild(condition_exp);
	
	ASTNode *assign_exp1 = assign_expression();
	if (checkNullExpression(assign_exp1)) {
		delete assign_exp;
		return NULL;
	}
	
	assign_exp->addChild(assign_exp1);
	
	return assign_exp;
};

ASTNode *Parser::expression() {
	return assign_expression();
};

ASTNode *Parser::checkNotNullExpression() {
	ASTNode *n = expression();
	if (n == NULL && !error_)
		parser_error("NULL-pointer expression");
	return n;
};

bool Parser::checkNullExpression(ASTNode *exp) {
	if (exp == NULL && !error_) {
		parser_error("NULL-pointer expression");
		return 1;
	}
	return 0;
};

ASTNode *Parser::initializerstatement() {
	
	// Initilizer statement used by FOR initializer
	
	if (match(TEOF))
		return parser_error("EOF Statement");
	
	if (error_)
		return NULL;
	
	if (match(VAR) || match(SAFE) || match(LOCAL) || match(CONST)) {
		DEBUG("DEFINE statement")
		
		// DEFINE statement
		
		// FRAME:
		// value1
		// value2
		// ...
		// OBJECTS:
		// ...
		// type3 (4bit integer)
		// name3 (string)
		// type2 (4bit integer)
		// name2 (string)
		// type1 (4bit integer)
		// name1 (string)
		
		// type = has_value | safe  | local | const
		//        1000/0000   0100    0010    0001
		
		// if (varN has_value = 0) => valueN miss
		
		// FRAME example
		// 10
		// "hello"
		// OBJECTS example
		// 1100
		// my_number                     -> 10
		// 0100
		// var_with_no_initial_value
		// 1100
		// my_string                     -> "hello"
		
		// const safe local var a = 10;
		//            ^ not used
		
		ASTNode *definenode = new ASTNode(get(-1)->lineno, DEFINE);
		
		// Modifiers
		bool _var   = (get(-1)->token == VAR);
		bool _safe  = (get(-1)->token == SAFE);
		bool _local = (get(-1)->token == LOCAL);
		bool _const = (get(-1)->token == CONST);
		
		while (match(VAR) || match(SAFE) || match(LOCAL) || match(CONST)) {
			_var   |= (get(-1)->token == VAR);
			_safe  |= (get(-1)->token == SAFE);
			_local |= (get(-1)->token == LOCAL);
			_const |= (get(-1)->token == CONST);			
		}
		
		while (true) {
			// if (match(SEMICOLON))
			// 	break;
			
			if (!match(NAME)) {
				delete definenode;
				return parser_error("Expected name");
			}
			
			string *name = get(-1)->stringv->copy();
			
			if (match(ASSIGN)) {
				_var = 1;
				
				ASTNode *exp = checkNotNullExpression();
				
				// Check for NULL expressin and ommit memory leak
				if (error_) {
					delete name;
					delete definenode;
					return NULL;
				}
				definenode->addChild(exp);
			} else 
				_var = 0;
			
				
			int *type = new int;
			*type = (_var << 3) | (_safe << 2) | (_local << 1) | _const;
			definenode->addFirstObject(name);
			definenode->addFirstObject(type);
			
			if (!match(COMMA))
				break;
		}
		
		return definenode;
	}
	
	else {
		// Expression statement
		
		// FRAME:
		// expression
		
		DEBUG("\\/ EXPRESSION STATEMENT node")
		ASTNode *expst = new ASTNode(get(0)->lineno, EXPRESSION);
		
		// Check for NULL expressin and ommit memory leak
		ASTNode *exp = checkNotNullExpression();
		if (error_) {
			delete expst;
			return NULL;
		}
		
		expst->addChild(exp);
		
		return expst;
	};
};

bool Parser::peekStatementWithoutSemicolon() {
	
	// Used to check if next node if statement
	
	return 
			get(0)->token == LC
			||
			get(0)->token == IF
			||
			get(0)->token == SWITCH
			||
			get(0)->token == WHILE
			||
			get(0)->token == DO
			||
			get(0)->token == FOR
			||
			get(0)->token == BREAK
			||
			get(0)->token == CONTINUE
			||
			get(0)->token == RETURN
			||
			get(0)->token == RAISE
			||
			get(0)->token == TRY
			||
			get(0)->token == VAR
			||
			get(0)->token == SAFE
			||
			get(0)->token == CONST
			||
			get(0)->token == LOCAL;
};

ASTNode *Parser::statement() {
	
	// Parse statement & consume all semicolons after it
	
	if (match(TEOF))
		return parser_error("EOF Statement");
	
	if (error_)
		return NULL;
	
	ASTNode *s = statement_with_semicolons();
	
	while (match(SEMICOLON));
	
	return s;
};

ASTNode *Parser::statement_with_semicolons() {
	
	if (match(TEOF))
		return parser_error("EOF Statement");
	
	if (error_)
		return NULL;
	
	else if (match(SEMICOLON)) {
		// printf("____ %d %d %d\n", get(-1)->token, get(0)->token, get(1)->token);
		DEBUG("EMPTY node")
		ASTNode *empty = new ASTNode(get(-1)->lineno, EMPTY);
		
		return empty;
	}
	
	else if (match(IF)) {
		// FRAME:
		// condition
		// IF node
		// ELSE node
		
		DEBUG("IF node")
		ASTNode *ifelse = new ASTNode(get(-1)->lineno, IF);
		// printf("lo lineno: %d %d\n", get(-1)->lineno);
		int lp = match(LP);
		
		// Check for NULL expressin and ommit memory leak
		ASTNode *condition = checkNotNullExpression();
		if (error_) {
			delete ifelse;
			return NULL;
		}
		
		ifelse->addChild(condition);
		
		if (lp && !match(RP)) {
			delete ifelse;
			return parser_error("Expected RP");
		}
		
		// Parse if & else nodes
		ASTNode *ifnode   = statement();
		ASTNode *elsenode = NULL;
		
		if (match(ELSE))
			elsenode = statement();
		else {
			elsenode = new ASTNode(-1, EMPTY);
		}
		
		// Add nodes
		ifelse->addChild(ifnode);
		ifelse->addChild(elsenode);
		
		return ifelse;
	}
	
	else if (match(SWITCH)) {
		// FRAME:
		// condition
		// case1
		// ...
		// caseN
		
		DEBUG("SWITCH node")
		ASTNode *switchcase = new ASTNode(get(-1)->lineno, SWITCH);
		int lp = match(LP);
		
		// Check for NULL expressin and ommit memory leak
		ASTNode *condition = checkNotNullExpression();
		if (error_) {
			delete switchcase;
			return NULL;
		}
		
		switchcase->addChild(condition);
		
		if (lp && !match(RP)) {
			delete switchcase;
			return parser_error("Expected RP");
		}
		
		if (match(SEMICOLON));
		else {
			if (!match(LC)) {
				delete switchcase;
				return parser_error("Expected {");
			}
			
			bool matched_default = 0;
			
			while (match(CASE) || match(DEFAULT)) {
				int lineno = get(-1)->lineno;
				
				if (get(-1)->token == CASE) { 
					// CASE node
					
					// FRAME:
					// condition
					// node
				
					DEBUG("CASE node")
					// Check for NULL expressin and ommit memory leak
					ASTNode *condition = checkNotNullExpression();
					if (error_) {
						delete switchcase;
						return NULL;
					}
					
					ASTNode *casenode = new ASTNode(lineno, CASE);
					casenode->addChild(condition);
					switchcase->addChild(casenode);
					
					if (!match(COLON)) {
						delete switchcase;
						return parser_error("Expected :");
					}
					
					while (true) {
						if (error_)  {
							delete switchcase;
							return NULL;
						}
						if (match(TEOF)) {
							delete switchcase;
							return parser_error("Expected }");
						}
						if (get(0)->token == CASE || get(0)->token == DEFAULT || get(0)->token == RC)
							break;
						casenode->addChild(statement());
					}
					
				} else { 
					// DEFAULT node
					
					// FRAME:
					// node
				
					DEBUG("DEFAULT node")
					if (matched_default) {
						delete switchcase;
						return parser_error("Duplicate default case");
					}
					matched_default = 1;
					
					if (!match(COLON)) {
						delete switchcase;
						return parser_error("Expected :");
					}
					
					ASTNode *defaultnode = new ASTNode(lineno, DEFAULT);
					switchcase->addChild(defaultnode);
					
					while (true) {
						if (error_)  {
							delete switchcase;
							return NULL;
						}
						if (match(TEOF)) {
							delete switchcase;
							return parser_error("Expected }");
						}
						if (get(0)->token == CASE || get(0)->token == DEFAULT || get(0)->token == RC)
							break;
						defaultnode->addChild(statement());
					}
				}
			}
			
			if (!match(RC)) {
				delete switchcase;
				return parser_error("Expected }");
			}
		}
		
		/*
		// Insert default after condition
		if (defaultnode == NULL) {
			defaultnode = new ASTNode(-1, EMPTY);
		
			// Add visitor function for this node
			defaultnode->node_visit = NULL; // TODO: NODE_VISITOR
		}
		ASTNode *tmp = switchcase->left->next;
		switchcase->left = defaultnode;
		defaultnode->next = tmp;
		*/
		
		return switchcase;
	}
	
	else if (match(WHILE)) {
		// FRAME:
		// condition
		// BODY node
		
		DEBUG("WHILE node")
		ASTNode *whileloop = new ASTNode(get(-1)->lineno, WHILE);
		int lp = match(LP);
		
		// Check for NULL expressin and ommit memory leak
		ASTNode *condition = checkNotNullExpression();
		if (error_) {
			delete whileloop;
			return NULL;
		}
		
		whileloop->addChild(condition);
		
		if (lp && !match(RP)) {
			delete whileloop;
			return parser_error("Expected RP");
		}
		
		// Parse if & else nodes
		ASTNode *bodynode   = statement();
		
		// Add nodes
		whileloop->addChild(bodynode);
		
		return whileloop;
	}
	
	else if (match(DO)) {
		// FRAME:
		// condition
		// BODY node
		
		DEBUG("DO WHILE node")
		ASTNode *doloop = new ASTNode(get(-1)->lineno, DO);
		ASTNode *body   = statement();
		
		if (!match(WHILE)) {
			delete doloop;
			delete body;
			return parser_error("Expected while");
		}
		
		int lp = match(LP);
		
		// Check for NULL expressin and ommit memory leak
		ASTNode *condition = checkNotNullExpression();
		if (error_) {
			delete doloop;
			delete body;
			return NULL;
		}
		
		doloop->addChild(condition);
		doloop->addChild(body);
		
		if (lp && !match(RP)) {
			delete doloop;
			return parser_error("Expected RP");
		}
		
		return doloop;
	}
	
	else if (match(FOR)) {
		// () expected for (;;)
		
		// FRAME:
		// INITIALIZATION node
		// CONDITION node
		// INCREMENT node
		// BODY node
		
		DEBUG("FOR node")
		ASTNode *forloop = new ASTNode(get(-1)->lineno, FOR);
		
		if (!match(LP)) {
			delete forloop;
			return parser_error("Expected LP");				
		}
		
		if (!match(SEMICOLON)) {
			ASTNode *initialization = initializerstatement();
			forloop->addChild(initialization);
			
			if (!match(SEMICOLON)) {
				delete forloop;
				return parser_error("Expected ;");
			}
		} else {
			ASTNode *empty = new ASTNode(-1, EMPTY);
			forloop->addChild(empty);
		}
		
		if (!match(SEMICOLON)) {
			// Check for NULL expressin and ommit memory leak
			ASTNode *condition = checkNotNullExpression();
			if (error_) {
				delete forloop;
				return NULL;
			}
			forloop->addChild(condition);
			
			if (!match(SEMICOLON)) {
				delete forloop;
				return parser_error("Expected ;");
			}
		} else {
			ASTNode *empty = new ASTNode(-1, EMPTY);
			forloop->addChild(empty);
		}
		
		if (!match(RP)) {
			// Check for NULL expressin and ommit memory leak
			ASTNode *increment = checkNotNullExpression();
			if (error_) {
				delete forloop;
				return NULL;
			}
			forloop->addChild(increment);

			if (!match(RP)) {
				delete forloop;
				return parser_error("Expected RP");
			}
		} else {
			ASTNode *empty = new ASTNode(-1, EMPTY);
			forloop->addChild(empty);
		}
		
		// Parse if & else nodes
		ASTNode *bodynode   = statement();
		
		// Add nodes
		forloop->addChild(bodynode);
		
		return forloop;
	}
	
	else if (match(LC)) { 
		ASTNode *blocknode = new ASTNode(get(-1)->lineno, BLOCK);
		
		while (true) {
			if (error_) {
				delete blocknode;
				return NULL;
			}
			if (match(TEOF)) {
				delete blocknode;
				return parser_error("Expected }");
			}
			if (match(RC))
				break;
			
			blocknode->addChild(statement());
		}
		
		return blocknode;
	}
	
	else if (match(BREAK)) {
		
		DEBUG("BREAK node");
		ASTNode *breaknode = new ASTNode(get(-1)->lineno, BREAK);
		
		return breaknode;
	}
	
	else if (match(CONTINUE)) {
		
		DEBUG("CONTINUE node");
		ASTNode *coontinuenode = new ASTNode(get(-1)->lineno, CONTINUE);
		
		return coontinuenode;
	}
	
	else if (match(RETURN)) {
		
		DEBUG("RETURN node");
		// FREAME:
		// value / EMPTY
		
		ASTNode *returnnode = new ASTNode(get(-1)->lineno, RETURN);
		
		if (!peekStatementWithoutSemicolon() && get(0)->token != SEMICOLON) {
			ASTNode *exp = checkNotNullExpression();
			
			// Check for NULL expressin and ommit memory leak
			if (!exp) {
				delete returnnode;
				return NULL;
			}
			returnnode->addChild(exp);
		}
		
		return returnnode;
	}
	
	else if (match(RAISE)) {
		
		DEBUG("RAISE node");
		// FREAME:
		// value / EMPTY
		
		ASTNode *raisenode = new ASTNode(get(-1)->lineno, RAISE);
		
		if (peekStatementWithoutSemicolon()) {
			ASTNode *empty = new ASTNode(-1, EMPTY);
			raisenode->addChild(empty);
		} else {
			ASTNode *exp = checkNotNullExpression();
			
			// Check for NULL expressin and ommit memory leak
			if (error_) {
				delete raisenode;
				return NULL;
			}
			raisenode->addChild(exp);
		}
		
		return raisenode;
	}
	
	else if (match(TRY)) {
		// FRAME
		// try node
		// expect node
		// OBJECTS:
		// handler name / null
		
		DEBUG("TRY node")
		ASTNode *tryexpect = new ASTNode(get(-1)->lineno, TRY);
		tryexpect->addChild(statement());
		
		if (match(EXPECT)) {
			if (match(LP)) {
				if (!match(NAME)) {
					delete tryexpect;
					return parser_error("Expected name");
				}
				
				string *name = get(-1)->stringv->copy();
				
				if (!match(RP)) {
					delete name;
					delete tryexpect;
					return parser_error("Expected )");
				}
				
				tryexpect->addLastObject(name);
			}
			
			tryexpect->addChild(statement());
		} else {
			ASTNode *empty = new ASTNode(-1, EMPTY);
			tryexpect->addChild(empty);
		}
		
		return tryexpect;
	}
	
	else
		return initializerstatement();
};

ASTNode *Parser::parse() {
	ASTNode *root = new ASTNode(0, ASTROOT);
	
	
	//while (this->source->nextToken())
	//	printf("line: %d, token: %d\n", source->token.lineno, source->token.token);
	
	//if (false)
	while (!eof()) {
		ASTNode *node = statement();
		
		if (error_ || !_global_exec_state)
			break;
		
		if (node != NULL)
			root->addChild(node);
		else {
			printf("NULL node");
			eof_ = 1;
		}
	}
	
	// printf("after: %d %d %d %d\n", eof(), eof_, error_, get(0)->token);
	
	if (error_) {
		delete root;
		parser_error("Parser error");
		return NULL;
	}
	
	return root;
};

int Parser::lineno() {
	return get(0)->lineno;
};

int Parser::eof() {
	return // source->eof() || 
		eof_ || error_;
};


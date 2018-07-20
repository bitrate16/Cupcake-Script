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


#include "TokenNamespace.h"

const char *tokenToString(int token) {
	switch (token) {
		case INTEGER:
			return "@integer";
		case DOUBLE:
			return "@double";
		case BOOLEAN:
			return "@boolean";
		case STRING:
			return "\"@string\"";
		case NAME:
			return "@name";
		case FLAG:
			return "@flag";
		case LONG:
			return "@long";
		case BYTE:
			return "@byte";
			
		case IF:
			return "if";
		case ELSE:
			return "else";
		case SWITCH:
			return "switch";
		case CASE:
			return "case";
		case DEFAULT:
			return "default";
		case BREAK:
			return "break";
		case CONTINUE:
			return "cuntinue";
		case RETURN:
			return "return";
		case WHILE:
			return "while";
		case DO:
			return "do";
		case FOR:
			return "for";
		case IN:
			return "in";
		case FUNCTION:
			return "function";
		case VAR:
			return "var";
		case TRY:
			return "try";
		case EXPECT:
			return "expect";
		case RAISE:
			return "raise";
		case PROTOTYPE:
			return "prototype";
		case CONST:
			return "const";
		case SAFE:
			return "fase";
		case NEW:
			return "new";
		case LOCAL:
			return "local";
		case TRUE:
			return "true";
		case FALSE:
			return "false";
		case TNULL:
			return "null";
		case UNDEFINED:
			return "undefined";
		case SELF:
			return "self";
		case THIS:
			return "this";
			
		case ASSIGN:
			return "=";
		case HOOK:
			return "?";
		case COLON:
			return ":";
		case DOT:
			return ".";
		case COMMA:
			return ",";
		case SEMICOLON:
			return ";";
		case LP:
			return "(";
		case RP:
			return ")";
		case LB:
			return "[";
		case RB:
			return "]";
		case LC:
			return "{";
		case RC:
			return "}";
		case EQ:
			return "==";
		case NEQ:
			return "!=";
		case OR:
			return "||";
		case AND:
			return "&&";
		case BITOR:
			return "|";
		case BITAND:
			return "&";
		case BITXOR:
			return "^";
		case GT:
			return ">";
		case GE:
			return ">=";
		case LT:
			return "<";
		case LE:
			return "<=";
		case BITRSH:
			return ">>";
		case BITLSH:
			return "<<";
		case BITURSH:
			return ">>>";
		case PLUS:
			return "+";
		case MINUS:
			return "-";
		case MUL:
			return "*";
		case DIV:
			return "/";
		case MDIV:
			return "\\\\";
		case MOD:
			return "%";
		case HASH:
			return "#";
		case NOT:
			return "!";
		case BITNOT:
			return "~";
		case INC:
			return "++";
		case DEC:
			return "--";
		case ASSIGN_ADD:
			return "+=";
		case ASSIGN_SUB:
			return "-=";
		case ASSIGN_MUL:
			return "*=";
		case ASSIGN_DIV:
			return "/=";
		case ASSIGN_BITRSH:
			return ">>=";
		case ASSIGN_BITLSH:
			return "<<=";
		case ASSIGN_BITURSH:
			return ">>>=";
		case ASSIGN_BITNOT:
			return "~=";
		case ASSIGN_MDIV:
			return "\\\\=";
		case ASSIGN_MOD:
			return "%=";
		case ASSIGN_BITOR:
			return "|=";
		case ASSIGN_BITAND:
			return "&=";
		case ASSIGN_BITXOR:
			return "^=";
		case PATH:
			return "\\";
		case PUSH:
			return "->";
		case LAMBDA:
			return "=>";
		case PRE_INC:
			return "++";
		case PRE_DEC:
			return "--";
		case POS_INC:
			return "++";
		case POS_DEC:
			return "--";
		case POS:
			return "+";
		case NEG:
			return "-";
			
		case TEOF:
			return "@eof";
		case TEOL:
			return "@eol";
		case TERR:
			return "@err";
		case NONE:
			return "@none";
		case EMPTY:
			return "@empty";
		case EXPRESSION:
			return "@expression";
		case BLOCK:
			return "@block";
		case ASSIGN_DEFINE:
			return "@assign_define";
		case DEFINE:
			return "@define";
		case ASTROOT:
			return "ast@root";
		case IMPORTED_SCRIPT:
			return "@mported_script";
		case CONDITION:
			return "@condition";
		case MEMBER:
			return "@member";
		case FIELD:
			return "@field";
		case CALL:
			return "@call";
		case NATIVE_CALL:
			return "native@call";
		case ARRAY:
			return "@array";
		case OBJECT:
			return "@object";
		case SCOPE:
			return "@scope";
		case FUNCTIONROOT:
			return "fucntion@root";
		default:
			return "";
	}
};

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
 * Namespace-header file that lists:
 * + All available AST tokens.
 * + Interpreter-predefined type tokens.
 * + Few ducks at the end of file.
 */

#ifndef TOKEN_NAMESPACE_H
#define TOKEN_NAMESPACE_H


// Token types

#define INTEGER	         12
#define DOUBLE           13
#define BOOLEAN          14
#define STRING           15
#define NAME             16
#define FLAG             17
#define LONG             18
#define BYTE             19
#define ARRAY            20
#define OBJECT           21
#define SCOPE            22
#define STRING_PROTOTYPE           23
#define DOUBLE_PROTOTYPE           24
#define NULL_PROTOTYPE             25
#define OBJECT_PROTOTYPE           26
#define SCOPE_PROTOTYPE            27
#define PROXY_SCOPE                28
#define CODE_FUNCTION_PROTOTYPE    29
#define CODE_FUNCTION              30
#define NATIVE_FUNCTION_PROTOTYPE  31
#define NATIVE_FUNCTION            32
#define INTEGER_PROTOTYPE          33
#define BOOLEAN_PROTOTYPE          34
#define UNDEFINED_PROTOTYPE        35
#define TNULL                      36
#define UNDEFINED                  37
#define ARRAY_PROTOTYPE            38
#define ERROR                      39
#define ERROR_PROTOTYPE            40

// Keywords

#define IF               47
#define ELSE             48
#define SWITCH           49
#define CASE             50
#define DEFAULT          74
#define BREAK            51
#define CONTINUE         52
#define RETURN           53
#define WHILE            54
#define DO               55
#define FOR              56
#define IN               57
#define FUNCTION         58
#define VAR              59
#define TRY              60
#define EXPECT           61
#define RAISE            62
#define PROTOTYPE        63
#define CONST            64
#define SAFE             65
#define NEW              66
#define LOCAL            67
#define TRUE             68
#define FALSE            69
#define SELF             72
#define THIS             73

// Operators

#define ASSIGN           80
#define HOOK             81
#define COLON            82
#define DOT              83
#define COMMA            84
#define SEMICOLON        85
#define LP               86
#define RP               87
#define LB               88
#define RB               89
#define LC               90
#define RC               91
#define EQ               92
#define NEQ              93
#define OR               94
#define AND              95
#define BITOR            96
#define BITAND           97
#define BITXOR           98
#define GT               99
#define GE              100
#define LT              101
#define LE              102
#define BITRSH          103
#define BITLSH          104
#define BITURSH         105
#define PLUS            106
#define MINUS           107
#define MUL             108
#define DIV             109
#define MDIV            110
#define MOD             111
#define HASH            112
#define NOT             113
#define BITNOT          114
#define INC             115
#define DEC             116
#define ASSIGN_ADD      117
#define ASSIGN_SUB      118
#define ASSIGN_MUL      119
#define ASSIGN_DIV      120
#define ASSIGN_BITRSH   121
#define ASSIGN_BITLSH   122
#define ASSIGN_BITURSH  123
#define ASSIGN_BITNOT   124
#define ASSIGN_MDIV     125
#define ASSIGN_MOD      126
#define ASSIGN_BITOR    127
#define ASSIGN_BITAND   128
#define ASSIGN_BITXOR   129
#define PATH             130
#define PUSH            131
#define LAMBDA          132
#define PRE_INC         133
#define PRE_DEC         134
#define POS_INC         135
#define POS_DEC         136
#define POS             157
#define NEG             158

// Technical tokens

#define TEOF            137
#define TEOL            '\n'
#define TERR            139
#define NONE            140


#define EMPTY           201
#define EXPRESSION      202
#define BLOCK           203
#define ASSIGN_DEFINE   204
#define DEFINE          205
#define ASTROOT         206
#define CONDITION       207
#define MEMBER          208
#define FIELD           209
#define CALL            210
#define NATIVE_CALL     211
#define IMPORTED_SCRIPT 212
#define FUNCTIONROOT    213

// Returns consta char* string representation of given token type
const char *tokenToString(int);

//     _      _      _
//  __(.)< __(.)> __(.)=
//  \___)  \___)  \___)   krya-krya
#endif
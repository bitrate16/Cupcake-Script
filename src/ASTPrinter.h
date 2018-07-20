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
 * Utility for printing out parsed AST tree.
 */

#ifndef ASTPRINTER_H
#define ASTPRINTER_H

#include "DebugUtils.h"

void printAST(ASTNode *localroot) {
	if (localroot == NULL)
		return;
	
	switch (localroot->type) {
		case ASTROOT:
			ASTNode *tmp;
			tmp = localroot->left;
			while (tmp) {
				printAST(tmp);
				putchar(10);
				tmp = tmp->next;
			}
			break;
		
		case INTEGER:
			printf("%d", *(int*) (localroot->objectlist->object));
			break;
			
		case LONG:
			printf("%l", *(long*)localroot->objectlist->object);
			break;
			
		case BYTE:
			printf("%d", *(unsigned char*)localroot->objectlist->object);
			break;
			
		case BOOLEAN:
			printf(*(bool*)localroot->objectlist->object ? "true" : "false");
			break;
			
		case DOUBLE:
			printf("%f", *(double*)localroot->objectlist->object);
			break;
			
		case NAME:
			((string*)localroot->objectlist->object)->print();
			break;
			
		case STRING:
			putchar('\"');
			((string*)localroot->objectlist->object)->print();
			putchar('\"');
			break;
			
		case ARRAY: {
			ASTNode *p = localroot->left;
			
			printf("[ ");
			
			while (p) {
				printAST(p);
				p = p->next;
				if (p)
					printf(", ");
			}
			printf("]");
			break;
		}
			
		case OBJECT: {
			ASTNode *p       = localroot->left;
			ASTObjectList *o = localroot->objectlist;
			
			printf("{ ");
			
			while (p) {
				((string*) (o->object))->print();
				printf(": ");
				printAST(p);
				p = p->next;
				o = o->next;
				if (p)
					printf(", ");
			}
			printf("}");
			break;
		}
			
		case FUNCTION: {
			ASTObjectList *o = localroot->objectlist;
			
			printf("function (");
			
			while (o) {
				((string*) (o->object))->print();
				o = o->next;
				if (o)
					printf(", ");
			}
			printf(") ");
			printAST(localroot->left);
			break;
		}
			
		case THIS:
			printf("this");
			break;
			
		case TNULL:
			printf("null");
			break;
			
		case UNDEFINED:
			printf("undefined");
			break;
			
		case CONDITION:
			printAST(localroot->left);
			printf("? ");
			printAST(localroot->left->next);
			printf(": ");
			printAST(localroot->right);
			break;
			
		case POS:
			printf("+ ");
			printAST(localroot->left);
			break;
			
		case NEG:
			printf("- ");
			printAST(localroot->left);
			break;
			
		case POS_INC:
			printAST(localroot->left);
			printf("++");
			break;
			
		case POS_DEC:
			printAST(localroot->left);
			printf("--");
			break;
			
		case PRE_INC:
			printf("++");
			printAST(localroot->left);
			break;
			
		case PRE_DEC:
			printf("--");
			printAST(localroot->left);
			break;
			
		case NOT:
			printf("!");
			printAST(localroot->left);
			break;
			
		case BITNOT:
			printf("~");
			printAST(localroot->left);
			break;
			
		case PLUS:
			printAST(localroot->left);
			printf("+ ");
			printAST(localroot->right);
			break;
			
		case MINUS:
			printAST(localroot->left);
			printf("- ");
			printAST(localroot->right);
			break;
			
		case MUL:
			printAST(localroot->left);
			printf("* ");
			printAST(localroot->right);
			break;
			
		case DIV:
			printAST(localroot->left);
			printf("/ ");
			printAST(localroot->right);
			break;
			
		case MDIV:
			printAST(localroot->left);
			printf("\\\\ ");
			printAST(localroot->right);
			break;
			
		case MOD:
			printAST(localroot->left);
			printf("% ");
			printAST(localroot->right);
			break;
			
		case HASH:
			printAST(localroot->left);
			printf("# ");
			printAST(localroot->right);
			break;
			
		case AND:
			printAST(localroot->left);
			printf("&& ");
			printAST(localroot->right);
			break;
			
		case OR:
			printAST(localroot->left);
			printf("|| ");
			printAST(localroot->right);
			break;
			
		case BITAND:
			printAST(localroot->left);
			printf("& ");
			printAST(localroot->right);
			break;
			
		case BITOR:
			printAST(localroot->left);
			printf("| ");
			printAST(localroot->right);
			break;
			
		case BITXOR:
			printAST(localroot->left);
			printf("^ ");
			printAST(localroot->right);
			break;
			
		case BITRSH:
			printAST(localroot->left);
			printf(">> ");
			printAST(localroot->right);
			break;
			
		case BITLSH:
			printAST(localroot->left);
			printf("<< ");
			printAST(localroot->right);
			break;
			
		case BITURSH:
			printAST(localroot->left);
			printf(">>> ");
			printAST(localroot->right);
			break;
			
		case GT:
			printAST(localroot->left);
			printf("> ");
			printAST(localroot->right);
			break;
			
		case GE:
			printAST(localroot->left);
			printf(">= ");
			printAST(localroot->right);
			break;
			
		case LT:
			printAST(localroot->left);
			printf("< ");
			printAST(localroot->right);
			break;
			
		case LE:
			printAST(localroot->left);
			printf("<= ");
			printAST(localroot->right);
			break;
			
		case EQ:
			printAST(localroot->left);
			printf("== ");
			printAST(localroot->right);
			break;
			
		case NEQ:
			printAST(localroot->left);
			printf("!= ");
			printAST(localroot->right);
			break;
			
		case PATH:
			printAST(localroot->left);
			printf("\\ ");
			printAST(localroot->right);
			break;
			
		case PUSH:
			printAST(localroot->left);
			printf("-> ");
			printAST(localroot->right);
			break;
			
		case LAMBDA:
			printAST(localroot->left);
			printf("=> ");
			printAST(localroot->right);
			break;
			
		case ASSIGN:
			printAST(localroot->left);
			printf("= ");
			printAST(localroot->right);
			break;
			
		case ASSIGN_ADD:
			printAST(localroot->left);
			printf("+= ");
			printAST(localroot->right);
			break;
			
		case ASSIGN_SUB:
			printAST(localroot->left);
			printf("-= ");
			printAST(localroot->right);
			break;
			
		case ASSIGN_MUL:
			printAST(localroot->left);
			printf("*= ");
			printAST(localroot->right);
			break;
			
		case ASSIGN_DIV:
			printAST(localroot->left);
			printf("/= ");
			printAST(localroot->right);
			break;
			
		case ASSIGN_MOD:
			printAST(localroot->left);
			printf("%= ");
			printAST(localroot->right);
			break;
			
		case ASSIGN_BITRSH:
			printAST(localroot->left);
			printf(">>= ");
			printAST(localroot->right);
			break;
			
		case ASSIGN_BITLSH:
			printAST(localroot->left);
			printf("<<= ");
			printAST(localroot->right);
			break;
			
		case ASSIGN_BITURSH:
			printAST(localroot->left);
			printf(">>>= ");
			printAST(localroot->right);
			break;
			
		case ASSIGN_BITNOT:
			printAST(localroot->left);
			printf("~= ");
			printAST(localroot->right);
			break;
			
		case ASSIGN_BITXOR:
			printAST(localroot->left);
			printf("^= ");
			printAST(localroot->right);
			break;
			
		case ASSIGN_BITOR:
			printAST(localroot->left);
			printf("|= ");
			printAST(localroot->right);
			break;
			
		case ASSIGN_BITAND:
			printAST(localroot->left);
			printf("&= ");
			printAST(localroot->right);
			break;
			
		case FIELD: 
			printAST(localroot->left);
			printf(". ");
			((string*) (localroot->objectlist->object))->print();
			break;
			
		case MEMBER: 
			printAST(localroot->left);
			printf("[ ");
			printAST(localroot->right);
			putchar(']');
			break;
			
		case CALL: {
			ASTNode *p = localroot->left;
			
			printAST(p);
			
			p = p->next;
			
			printf("( ");
			
			while (p) {
				printAST(p);
				p = p->next;
				if (p)
					printf(", ");
			}
			printf(")");
			
			break;
		}
			
		case EMPTY:
		default:
			printf("{ }");
			break;
			
		case EXPRESSION:
			if (localroot->left != NULL)
				printAST(localroot->left);
			
			putchar(';');
			break;
			
		case DEFINE: {
			tmp = localroot->left;
			ASTObjectList *list = localroot->objectlist;
			
			while (list) {
				int type = *(int*)list->object;
				list = list->next;
				string *name = (string*)list->object;
				list = list->next;
				
				if (type & 0b0100) printf("safe ");
				if (type & 0b0010) printf("local ");
				if (type & 0b0001) printf("const ");
				printf("var ");
				name->print();
				
				if (type & 0b1000) {
					printf(" = ");
					printAST(tmp);
					tmp = tmp->next;
				}
			};
			
			putchar(';');
			break;
		}
		
		case IF:
			printf("if ( ");
			printAST(localroot->left);
			printf(") ");
			printAST(localroot->left->next);
			if (localroot->left->next->next->type != EMPTY) {
				printf("else ");
				printAST(localroot->left->next->next);
			}
			break;
		
		case SWITCH:
			printf("switch ( ");
			printAST(localroot->left);
			printf(") ");
			tmp = localroot->left->next;
			printf("{ ");
			while (tmp) {
				if (tmp->type == CASE) {
					printf("case ");
					printAST(tmp->left);
					printf(": ");
					printAST(tmp->left->next);
				} else {
					printf("default: ");
					printAST(tmp->left);
				}
				tmp = tmp->next;
			}
			printf("}");
			break;
			
		case BREAK:
			printf("break");
			break;
			
		case CONTINUE:
			printf("break");
			break;
			
		case RETURN:
			printf("return");
			if (localroot->left) {
				printf(" ");
				printAST(localroot->left);
			}
			
			putchar(';');
			break;
			
		case RAISE:
			printf("raise");
			if (localroot->left) {
				printf(" ");
				printAST(localroot->left);
			}
			
			putchar(';');
			break;
			
		case WHILE:
			printf("while ( ");
			printAST(localroot->left);
			printf(") ");
			printAST(localroot->left->next);
			break;
			
		case DO:
			printf("do ");
			printAST(localroot->left->next);
			printf("while ( ");
			printAST(localroot->left);
			printf(")");
			break;
			
		case FOR:
			printf("for ( ");
			if (localroot->left->type != EMPTY)
				printAST(localroot->left);
	
			printf("; ");
			if (localroot->left->next->type != EMPTY)
				printAST(localroot->left->next);
	
			printf("; ");
			if (localroot->left->next->next->type != EMPTY)
				printAST(localroot->left->next->next);
			
			printf(") ");
			printAST(localroot->left->next->next->next);
			break;
			
		case BLOCK:
			tmp = localroot->left;
			printf("{ ");
			while (tmp) {
				printAST(tmp);
				tmp = tmp->next;
			}
			printf("}");
			break;
			
		case TRY:
			printf("try ");
			printAST(localroot->left);
			if (localroot->left->next->type != EMPTY) {
				printf("expect ");
				if (localroot->objectlist != NULL) {
					printf("( ");
					((string*)localroot->objectlist->object)->print();
					printf(" ) ");
				}
				printAST(localroot->left->next);
			}
			break;
	};
	
	putchar(32);
};


#endif
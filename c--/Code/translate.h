#ifndef __TRANSLATE_H__
#define __TRANSLATE_H__
#include "intercode.h"
void trans_Program(struct Node *root);
void trans_ExtDefList(struct Node *root);
void trans_ExtDef(struct Node *root);
Operand trans_VarDec(struct Node *root);
void trans_FuncDec(struct Node *root);
void trans_Compst(struct Node *root);
void trans_StmtList(struct Node *root);
void trans_Stmt(struct Node *root);
void trans_DefList(struct Node *root);
void trans_Def(struct Node *root);
void trans_DecList(struct Node *root);
void trans_Dec(struct Node *root);
void trans_Exp(struct Node *root, Operand place);
void trans_Args(struct Node *root, bool write_func);
void trans_Cond(struct Node *root, Operand true_label, Operand false_label);

int get_size(Type type);
// Operand deep_copy(Operand left,Operand right);
// void debug_print(char* s,int line);
Operand load_value(Operand a);
void deep_copy(Operand left, Operand right);
Operand get_address(Operand op, bool is_arg);
#endif
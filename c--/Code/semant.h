#ifndef __SEMANT_H__
#define __SEMANT_H__
#include "my_node.h"
#include "hash_table.h"
#include "type.h"
#define NUM_INT 0;
#define NUM_FLOAT 1;
struct Node *get_child(struct Node *parent, int num);
void Program(struct Node *root);
void ExtDefList(struct Node *root);
void ExtDef(struct Node *root);
void ExtDecList(struct Node *root, Type type);
Type Specifier(struct Node *root);
Type StructSpecifier(struct Node *root);
char *OptTag(struct Node *root);
char *Tag(struct Node *root);
FieldList VarDec(struct Node *root, Type type, FieldList field);
void FunDec(struct Node *root, Type type);
void VarList(struct Node *root, FieldList field);
void ParamDec(struct Node *root, FieldList field);
void CompSt(struct Node *root, Type type);
void StmtList(struct Node *root, Type type);
void Stmt(struct Node *root, Type type);
void DefList(struct Node *root, FieldList field);
void Def(struct Node *root, FieldList field);
void DecList(struct Node *root, Type type, FieldList field);
void Dec(struct Node *root, Type type, FieldList field);
Type Exp(struct Node *root);
FieldList Args(struct Node *root);

FieldList InStructer(char *name, FieldList struct_field);
void add_to_structer(struct Node *node, Type type, FieldList field);
bool func_check_matched(FieldList a, FieldList b);
void add_to_args(struct Node *node, Type type, FieldList field);
#endif

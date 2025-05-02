#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__
// 要在这里实现“十字链表”
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"

#define HASHTABLE_SIZE 0x3fff
#define STACK_SIZE 100
typedef struct HashNode_ *HashNode;
HashNode hashtable[HASHTABLE_SIZE + 1];
HashNode stacktable[STACK_SIZE + 1];
extern int stack_index;
struct HashNode_
{
    Signal data;         // 当前节点存储的内容
    HashNode hash_next;  // hash链表中指向下一个节点
    HashNode stack_next; // stack链表中指向下一个节点
};

void init_hashtable();
unsigned int hash_pjw(char *name);
void insert_signal(Signal s);
void node_del(char *name);
Signal look_up(char *name);
void init_stacktable();
void stack_add();
void stack_del();
void semant_error(int err_type, int line, char *s);
#endif
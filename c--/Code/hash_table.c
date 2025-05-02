#include "hash_table.h"
// 初始化hash表
void init_hashtable()
{
    for (int i = 0; i <= HASHTABLE_SIZE; i++)
    {
        hashtable[i] = NULL;
    }
    // 一开始就插入read和write函数好了
    Signal r = (Signal)malloc(sizeof(struct Signal_));
    Signal w = (Signal)malloc(sizeof(struct Signal_));
    r->name = "read";
    w->name = "write";
    r->type = (Type)malloc(sizeof(struct Type_));
    w->type = (Type)malloc(sizeof(struct Type_));
    r->type->kind = FUNCTION;
    w->type->kind = FUNCTION;
    r->type->u.func.argc = 0;
    w->type->u.func.argc = 1;
    Type r_t = (Type)malloc(sizeof(struct Type_));
    r_t->kind == BASIC;
    r_t->u.basic = 0;
    r->type->u.func.return_type = r_t;
    w->type->u.func.return_type = r_t;
    r->type->u.func.args = (FieldList)malloc(sizeof(struct FieldList_));
    r->type->u.func.args->name = "read";
    r->type->u.func.args->type = (Type)malloc(sizeof(struct Type_));
    r->type->u.func.args->type->kind = FUNCTION;
    r->type->u.func.args->tail = NULL;
    w->type->u.func.args = (FieldList)malloc(sizeof(struct FieldList_));
    w->type->u.func.args->name = "write";
    w->type->u.func.args->type = (Type)malloc(sizeof(struct Type_));
    w->type->u.func.args->type->kind = FUNCTION;
    FieldList wargs = (FieldList)malloc(sizeof(struct FieldList_));
    wargs->name = "argv";
    wargs->type = r_t;
    wargs->tail = NULL;
    w->type->u.func.args->tail = wargs;
    insert_signal(r);
    insert_signal(w);
    // printf("******\n");
    // printf("finish insert r and w\n");
}
// 初始化stack链表
void init_stacktable()
{
    for (int i = 0; i <= STACK_SIZE; i++)
    {
        stacktable[i] = NULL;
    }
    stack_index = 0;
}
// hash值计算函数
unsigned int hash_pjw(char *name)
{
    if (name == NULL)
        return HASHTABLE_SIZE; // 如果是匿名函数的话就插到最后一格。
    unsigned int val = 0, i;
    for (; *name; name++)
    {
        val = (val << 2) + *name;
        if (i = val & ~HASHTABLE_SIZE)
            val = (val ^ (i >> 12)) & HASHTABLE_SIZE;
    }
    return val;
}
// 插入节点
void insert_signal(Signal s)
{
    unsigned int pos = hash_pjw(s->name);
    HashNode node = (HashNode)malloc(sizeof(struct HashNode_));
    assert(node != NULL);
    node->data = s;
    if (hashtable[pos] == NULL)
    {
        hashtable[pos] = (HashNode)malloc(sizeof(struct HashNode_));
        assert(hashtable[pos] != NULL);
        hashtable[pos]->hash_next = NULL;
    }
    // 头插(hash部分)
    node->hash_next = hashtable[pos]->hash_next;
    hashtable[pos]->hash_next = node;
    if (stacktable[stack_index] == NULL)
    {
        stacktable[stack_index] = (HashNode)malloc(sizeof(struct HashNode_));
        assert(stacktable[stack_index] != NULL);
        stacktable[stack_index]->stack_next = NULL;
    }
    // 头插(stack部分)
    node->stack_next = stacktable[stack_index]->stack_next;
    stacktable[stack_index]->stack_next = node;
}
// 查找节点
Signal look_up(char *name)
{
    if (name == NULL)
    {
        return NULL;
    }
    unsigned int pos = hash_pjw(name);
    if (hashtable[pos] == NULL)
    {
        return NULL;
    }
    HashNode node = hashtable[pos]->hash_next;
    while (node != NULL)
    {
        if (strcmp(node->data->name, name) == 0)
        {
            return node->data;
        }
        node = node->hash_next;
    }
    return NULL;
}
// 辅助删除节点：只是改变指针指向，不释放内存
void node_del(char *name)
{
    return;
    unsigned int pos = hash_pjw(name);
    HashNode node = hashtable[pos];
    while (node->hash_next != NULL)
    {
        if (strcmp(node->hash_next->data->name, name) == 0)
        {
            node->hash_next = node->hash_next->hash_next;
            return;
        }
        node = node->hash_next;
    }
}
// 当进入一个新的语句嵌套的时候使用，标志进入了新的一层。
void stack_add()
{
    stack_index++;
    stacktable[stack_index] = NULL;
}
// 删除这个语句块中的符号
void stack_del()
{
    return;
    if (stacktable[stack_index] == NULL)
    {
        return;
    }
    HashNode node = stacktable[stack_index]->stack_next;
    HashNode temp = NULL;
    while (node != NULL)
    {
        node_del(node->data->name);
        temp = node->stack_next;
        free(node);
        node = temp;
    }
    stack_index--;
}

void semant_error(int err_type, int line, char *s)
{
    printf("Error type %d at Line %d : %s\n", err_type, line, s);
    fflush(stdout);
}
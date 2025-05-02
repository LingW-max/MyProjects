// 这里是语义分析的主战场
#include "semant.h"
// 得到特定节点的第几个孩子。
struct Node *get_child(struct Node *parent, int num)
{
    // 检查 num 是否在有效范围内
    if (num < 1 || num > parent->child_num)
    {
        return NULL; // 或者可以返回一个适当的错误代码
    }
    return parent->children[num - 1];
}
bool if_debug = false;
void print_line(int line)
{
    if (if_debug == false)
        return;
    printf("line:%d\n", line);
    fflush(stdout);
}
void debug_print(char *s)
{
    if (if_debug == false)
        return;
    printf("%s\n", s);
    fflush(stdout);
}

void debug_tree(struct Node *root, int count)
{
    if (if_debug == false)
        return;
    if (root == NULL)
    {
        return;
    }
    for (int i = 0; i < count; ++i)
    {
        printf(" ");
    }
    if (root->type == stnc)
    {
        fprintf(stdout, "%s (%d)\n", root->name, root->line_num);
    }
    else
    {
        if (strcmp(root->name, "ID") == 0)
        {
            fprintf(stdout, "%s: %s\n", root->name, root->node_str);
        }
        else if (strcmp(root->name, "TYPE") == 0)
        {
            fprintf(stdout, "%s: %s\n", root->name, root->node_str);
        }
        else if (strcmp(root->name, "INT") == 0)
        {
            fprintf(stdout, "%s: %d\n", root->name, root->node_int);
        }
        else if (strcmp(root->name, "FLOAT") == 0)
        {
            fprintf(stdout, "%s: %f\n", root->name, root->node_float);
        }
        else
        {
            fprintf(stdout, "%s\n", root->name);
        }
    }
    for (int k = 0; k < root->child_num; k++)
    {
        debug_tree(root->children[k], count + 2);
    }
}

void Program(struct Node *root) // 程序入口
{
    debug_print("Program");
    print_line(root->line_num);
    // debug_tree(root, 0);
    // debug_tree(root, 7);
    // Program -> ExtDefList
    // 程序一开始先初始化符号表
    init_hashtable();
    init_stacktable();
    if (root == NULL)
    {
        return;
    }
    ExtDefList(get_child(root, 1));
}
void ExtDefList(struct Node *root) // 全局变量，结构体和函数的定义。
{
    debug_print("ExtDefList");
    if (root == NULL)
        return;
    print_line(root->line_num);
    // ExtDefList -> ExtDef ExtDefList
    //      | e
    ExtDef(get_child(root, 1));
    ExtDefList(get_child(root, 2));
}
void ExtDef(struct Node *root)
{
    debug_print("ExtDef");
    print_line(root->line_num);
    /*ExtDef -> Specifier ExtDecList SEMI 全局变量的定义
| Specifier SEMI 结构体的定义
| Specifier FunDec CompSt 函数的定义*/
    if (root == NULL)
        return;
    // assert(root->child_num == 2 || root->child_num == 3);
    Type type = Specifier(get_child(root, 1));
    if (strcmp(get_child(root, 2)->name, "ExtDecList") == 0)
    {
        ExtDecList(get_child(root, 2), type);
    }
    else if (strcmp(get_child(root, 2)->name, "FunDec") == 0)
    {
        FunDec(get_child(root, 2), type);
        CompSt(get_child(root, 3), type);
    }
    // else if (strcmp(get_child(root, 2)->name, "SEMI") == 0)
    //     ;
}

void ExtDecList(struct Node *root, Type type) // 全局变量的定义
{
    debug_print("ExtDecList");
    print_line(root->line_num);
    /*ExtDecList ->VarDec
| VarDec COMMA ExtDecList*/
    if (root == NULL)
        return;
    if (root->child_num == 1)
    {
        VarDec(get_child(root, 1), type, NULL);
    }
    else if (root->child_num == 3)
    {
        VarDec(get_child(root, 1), type, NULL);
        ExtDecList(get_child(root, 3), type);
    }
}

Type Specifier(struct Node *root) // 解析类型（如int，float这样的）和结构体
{
    debug_print("Specifier");
    print_line(root->line_num);
    /*Specifier -> TYPE
| StructSpecifier*/
    if (root == NULL)
        return NULL;
    Type type = NULL;
    if (strcmp(get_child(root, 1)->name, "TYPE") == 0)
    {
        type = (Type)malloc(sizeof(struct Type_));
        type->kind = BASIC;
        if (strcmp(get_child(root, 1)->node_str, "int") == 0)
        {
            type->u.basic = NUM_INT;
        }
        else if (strcmp(get_child(root, 1)->node_str, "float") == 0)
        {
            type->u.basic = NUM_FLOAT;
        }
    }
    else if (strcmp(get_child(root, 1)->name, "StructSpecifier") == 0)
    { // 结构体的情况
        type = StructSpecifier(get_child(root, 1));
    }
    return type;
}

Type StructSpecifier(struct Node *root)
{
    debug_print("StructSpecifier");
    print_line(root->line_num);
    /*StructSpecifier -> STRUCT OptTag LC DefList RC
| STRUCT Tag*/
    // debug_tree(root, 2);
    if (root == NULL)
        return NULL;
    Type type = NULL;
    if (root->child_num == 5) // 结构体定义的情况
    {
        // 把结构体存到符号表里面
        char *opt_tag = NULL;
        if (get_child(root, 2) == NULL)
        {
            opt_tag = NULL;
        }
        else
        {
            opt_tag = OptTag(get_child(root, 2));
        }
        int line = root->line_num;
        type = (Type)malloc(sizeof(struct Type_));
        type->kind = STRUCT_TYPE;
        type->u.structer = (FieldList)malloc(sizeof(struct FieldList_));
        type->u.structer->name = opt_tag;
        type->u.structer->type = (Type)malloc(sizeof(struct Type_));
        type->u.structer->type->kind = STRUCT_TYPE;
        type->u.structer->tail = NULL;
        DefList(get_child(root, 4), type->u.structer);
        Signal sig = (Signal)malloc(sizeof(struct Signal_));
        sig->name = opt_tag;
        sig->type = type;
        Signal ss = look_up(sig->name);
        if (ss != NULL)
        {
            semant_error(16, root->line_num, "structer redefined");
        }
        else
        {
            insert_signal(sig);
        }
    }
    else if (root->child_num == 2)
    {
        char *tag = Tag(get_child(root, 2));
        Signal rel_s = look_up(tag);
        int line = get_child(root, 2)->line_num;
        if (rel_s == NULL) // 即未在符号表内,报错
        {
            semant_error(17, line, "structer undefined");
            return NULL;
        }
        type = rel_s->type;
    }
    return type;
}

char *OptTag(struct Node *root)
{
    debug_print("OptTag");
    print_line(root->line_num);
    /*OptTag ->ID
| e*/
    if (root == NULL)
        return NULL;
    return get_child(root, 1)->node_str;
}
char *Tag(struct Node *root)
{
    debug_print("Tag");
    print_line(root->line_num);
    // Tag -> ID
    if (root == NULL)
        return NULL;
    return get_child(root, 1)->node_str;
}
FieldList VarDec(struct Node *root, Type type, FieldList field) // 对一个变量的定义
{
    debug_print("VarDec");
    print_line(root->line_num);
    /*VarDec ->ID
    | VarDec LB INT RB*/
    if (root == NULL)
        return NULL;
    Signal sig = NULL;
    FieldList ftemp = (FieldList)malloc(sizeof(struct FieldList_));
    if (root->child_num == 1)
    {
        sig = (Signal)malloc(sizeof(struct Signal_));
        sig->name = get_child(root, 1)->node_str;
        sig->type = type;
        ftemp->name = sig->name;
        ftemp->type = sig->type;
        ftemp->tail = NULL;
        int line = get_child(root, 1)->line_num;
        if (field == NULL || field->type->kind == FUNCTION)
        {
            Signal s = look_up(sig->name);
            if (s != NULL) // 即符号表中已存在
            {
                semant_error(3, root->line_num, "variable redefined");
            }
            else
            {
                insert_signal(sig);
            }
        }
        return ftemp;
    }
    else // 数组类型
    {
        Type arrary_type = (Type)malloc(sizeof(struct Type_));
        arrary_type->kind = ARRAY;
        // arrary_type->u.array.elem = type;
        if (type->kind == ARRAY)
        {
            arrary_type->u.array.elem = type;
            arrary_type->u.array.dismention = type->u.array.dismention + 1; // 那第一个维度怎么来的呢？
            arrary_type->u.array.size = type->u.array.size * get_child(root, 3)->node_int;
        }
        else
        {
            arrary_type->u.array.elem = type;
            arrary_type->u.array.dismention = 1;
            arrary_type->u.array.size = get_child(root, 3)->node_int;
        }
        return VarDec(get_child(root, 1), arrary_type, field);
    }
}
void FunDec(struct Node *root, Type type)
{
    debug_print("FunDec");
    print_line(root->line_num);
    if (root == NULL)
    {
        return;
    }
    /*FunDec -> ID LP VarList RP
| ID LP RP*/
    char *str = get_child(root, 1)->node_str;
    int line = root->line_num;
    Signal sig = (Signal)malloc(sizeof(struct Signal_));
    sig->type = (Type)malloc(sizeof(struct Type_));
    sig->name = str;
    // sig->type = (Type)malloc(sizeof(struct Type_));
    sig->type->kind = FUNCTION;
    sig->type->u.func.return_type = type;
    sig->type->u.func.args = (FieldList)malloc(sizeof(struct FieldList_));
    sig->type->u.func.args->name = str;
    sig->type->u.func.args->type = (Type)malloc(sizeof(struct Type_));
    sig->type->u.func.args->type->kind = FUNCTION;
    sig->type->u.func.args->tail = NULL;
    if (root->child_num == 4)
    {
        VarList(get_child(root, 3), sig->type->u.func.args);
    }
    Signal temp = look_up(sig->name);
    if (temp != NULL)
    {
        semant_error(4, line, "function redefined");
        return;
    }
    else if (temp == NULL)
    {
        insert_signal(sig);
    }
}
void VarList(struct Node *root, FieldList field) // 定义函数参数列表
{
    debug_print("VarList");
    print_line(root->line_num);
    /*VarList -> ParamDec COMMA VarList
| ParamDec*/
    if (root == NULL)
        return;
    if (root->child_num == 1)
    {
        ParamDec(get_child(root, 1), field);
    }
    else if (root->child_num == 3)
    {
        ParamDec(get_child(root, 1), field);
        VarList(get_child(root, 3), field);
    }
}
void ParamDec(struct Node *root, FieldList field) // 对每一个形参的定义
{
    debug_print("ParamDec");
    print_line(root->line_num);
    /*ParamDec -> Specifier VarDec*/
    if (root == NULL)
        return;
    FieldList field1 = (FieldList)malloc(sizeof(struct FieldList_));
    field1->type = Specifier(get_child(root, 1));
    field1->tail = NULL;
    // 关于函数形参的问题，是否应该存入符号表
    stack_index++;
    add_to_args(get_child(root, 2), field1->type, field);
    // VarDec(get_child(root, 2), field1->type, field);
    stack_index--;
}
void CompSt(struct Node *root, Type type) // 语句块
{
    debug_print("CompSt");
    print_line(root->line_num);
    if (root == NULL)
        return;
    /*CompSt -> LC DefList StmtList RC*/
    stack_add(); // 因为这是一个局部的东西
    // 插入东西
    // debug_tree(get_child(root, 2), 7);
    DefList(get_child(root, 2), NULL);
    StmtList(get_child(root, 3), type);
    stack_del();
    debug_print("finish comspt");
    return;
}
void StmtList(struct Node *root, Type type) // 语句部分，不是定义部分，也不需要考虑函数返回类型是否匹配问题
{
    debug_print("StmtList");
    if (root == NULL)
    {
        debug_print("stmtlist is null");
        return;
    }
    print_line(root->line_num);
    // debug_tree(root,3);
    /*StmtList -> Stmt StmtList
| e*/
    // if (root->line_num == 9)
    // {
    //     debug_tree(root, 7);
    // }
    Stmt(get_child(root, 1), type);
    StmtList(get_child(root, 2), type);
    debug_print("finish_stmtlist");
}
void Stmt(struct Node *root, Type type)
{
    debug_print("Stmt");
    print_line(root->line_num);
    // debug_tree(root,3);
    if (root == NULL)
    {
        return;
    }
    /*Stmt -> Exp SEMI
| CompSt
| RETURN Exp SEMI
| IF LP Exp RP Stmt
| IF LP Exp RP Stmt ELSE Stmt
| WHILE LP Exp RP Stmt*/
    if (strcmp(get_child(root, 1)->name, "CompSt") == 0)
    {
        CompSt(get_child(root, 1), type);
    }
    else if (strcmp(get_child(root, 1)->name, "Exp") == 0)
    {
        Exp(get_child(root, 1));
    }
    else if (strcmp(get_child(root, 1)->name, "RETURN") == 0)
    {
        Type temp = Exp(get_child(root, 2));
        if (!base_type_check(temp, type))
        {
            semant_error(8, root->line_num, "return type is error");
        }
    }
    else if (strcmp(get_child(root, 1)->name, "WHILE") == 0)
    {
        Exp(get_child(root, 3));
        Stmt(get_child(root, 5), type);
    }
    else if (strcmp(get_child(root, 1)->name, "IF") == 0)
    {
        if (root->child_num == 7)
        {
            Exp(get_child(root, 3));
            Stmt(get_child(root, 5), type);
            Stmt(get_child(root, 7), type);
        }
        else if (root->child_num == 5)
        {
            Exp(get_child(root, 3));
            Stmt(get_child(root, 5), type);
        }
    }
    return;
}
void DefList(struct Node *root, FieldList field) // 主要与局部变量有关
{
    debug_print("DefList");
    if (root == NULL)
    {
        debug_print("deflist is null");
        return;
    }
    print_line(root->line_num);
    /*DefList -> Def DefList 定义变量的语句
| e*/
    debug_print("here?");
    // debug_tree(get_child(root, 1), 7);
    Def(get_child(root, 1), field);
    DefList(get_child(root, 2), field);
}
void Def(struct Node *root, FieldList field)
{
    debug_print("Def");
    print_line(root->line_num);
    // Def -> Specifier DecList SEMI
    if (root == NULL)
    {
        return;
    }
    Type type = Specifier(get_child(root, 1));
    if (type != NULL)
        DecList(get_child(root, 2), type, field);
}
void DecList(struct Node *root, Type type, FieldList field)
{
    debug_print("DecList");
    print_line(root->line_num);
    /*DecList -> Dec
| Dec COMMA DecList*/
    if (root == NULL)
        return;
    if (root->child_num == 1)
    {
        Dec(get_child(root, 1), type, field);
    }
    else if (root->child_num == 3)
    {
        Dec(get_child(root, 1), type, field);
        DecList(get_child(root, 3), type, field);
    }
}
void Dec(struct Node *root, Type type, FieldList field)
{
    debug_print("Dec");
    print_line(root->line_num);
    /*Dec -> VarDec
| VarDec ASSIGNOP Exp*/
    if (root == NULL)
        return;
    if (root->child_num == 1)
    { // 如果这个是在结构体域中的，那就直接检查然后存入即可。
        if (field != NULL && field->type->kind == STRUCT_TYPE)
        {
            char *temp_name = NULL;
            struct Node *temp_node = get_child(root, 1);
            while (strcmp(temp_node->name, "ID") != 0)
            {
                temp_node = get_child(temp_node, 1);
            }
            temp_name = temp_node->node_str;

            if (InStructer(temp_name, field) != NULL)
            {
                semant_error(15, root->line_num, "variable in structer redefined");
            }
            else
            {
                add_to_structer(get_child(root, 1), type, field);
            }
        }
        // 我感觉还要单独考虑一下函数的参数那个部分，但现在先等等。
        else
        {
            VarDec(get_child(root, 1), type, field);
        }
    }
    else if (root->child_num == 3)
    {
        if (field != NULL && field->type->kind == STRUCT_TYPE)
        {
            add_to_structer(get_child(root, 1), type, field);
            semant_error(15, get_child(root, 1)->line_num, "Initialiazed struct variable in defined");
            return;
        }
        FieldList var_dec = VarDec(get_child(root, 1), type, field);
        // 先进行类型检查然后再存入符号表
        if (var_dec != NULL && base_type_check(type, Exp(get_child(root, 3))) == false)
        {
            // 类型不符合，报错
            semant_error(5, root->line_num, "type not equal on assignop");
        }
    }
}

Type Exp(struct Node *root)
{
    debug_print("Exp");
    print_line(root->line_num);
    /*Exp -> Exp ASSIGNOP Exp
| Exp AND Exp
| Exp OR Exp
| Exp RELOP Exp
| Exp PLUS Exp
| Exp MINUS Exp
| Exp STAR Exp
| Exp DIV Exp
| LP Exp RP
| MINUS Exp
| NOT Exp
| ID LP Args RP
| ID LP RP
| Exp LB Exp RB
| Exp DOT ID
| ID
| INT
| FLOAT*/
    if (root == NULL)
        return NULL;
    Type type = NULL;
    Signal rel = NULL;
    if (root->child_num == 1)
    {
        if (strcmp(get_child(root, 1)->name, "ID") == 0)
        {
            rel = look_up(get_child(root, 1)->node_str);
            if (rel == NULL || rel->type->kind == FUNCTION)
            {
                semant_error(1, root->line_num, "Undifined variables");
            }
            else
            {
                type = rel->type;
            }
        }
        // EXP->INT
        else if (strcmp(get_child(root, 1)->name, "INT") == 0)
        {
            type = (Type)malloc(sizeof(struct Type_));
            type->kind = BASIC;
            type->u.basic = NUM_INT;
        }
        // EXP->FLOAT
        else if (strcmp(get_child(root, 1)->name, "FLOAT") == 0)
        {
            type = (Type)malloc(sizeof(struct Type_));
            type->kind = BASIC;
            type->u.basic = NUM_FLOAT;
        }
    }
    else if (root->child_num == 2)
    {
        // EXP->NOT EXP
        if (strcmp(get_child(root, 1)->name, "NOT") == 0)
        {
            type = Exp(get_child(root, 2));
            if (type != NULL && (type->kind != BASIC || type->u.basic != 0))
            {
                semant_error(7, root->line_num, "only int type can perform logical operations");
            }
            type->kind = BASIC;
            type->u.basic = NUM_INT;
        }
        // EXP->MINUS EXP
        else if (strcmp(get_child(root, 1)->name, "MINUS") == 0)
        {
            type = Exp(get_child(root, 2));
        }
    }
    else if (root->child_num == 3)
    {
        // LP EXP RP
        if (strcmp(get_child(root, 1)->name, "LP") == 0)
        {
            type = Exp(get_child(root, 2));
        }
        // ID LP RP这就是函数的使用了
        else if (strcmp(get_child(root, 1)->name, "ID") == 0)
        {
            rel = look_up(get_child(root, 1)->node_str);
            if (rel == NULL)
            {
                semant_error(2, root->line_num, "use function but not defined");
            }
            else if (rel->type->kind != FUNCTION)
            {
                semant_error(11, root->line_num, "not a function,cannot use lp rp");
            }
            else if (func_check_matched(rel->type->u.func.args, NULL) == false)
            {
                semant_error(9, root->line_num, "args of function is not macthable");
            }
            if (rel != NULL && rel->type->kind == FUNCTION)
            {
                type = rel->type->u.func.return_type;
            }
        }
        // EXP-> Exp DOT ID
        else if (strcmp(get_child(root, 2)->name, "DOT") == 0)
        {
            Type temp = Exp(get_child(root, 1));
            if (temp->kind != STRUCT_TYPE)
            {
                semant_error(13, root->line_num, "use dot on not structer");
            }
            else
            {
                FieldList temp2 = InStructer(get_child(root, 3)->node_str, temp->u.structer);
                if (temp2 == NULL)
                {
                    semant_error(14, root->line_num, "field not exists");
                }
                else
                {
                    type = temp2->type;
                }
            }
        }
        // Exp ASSIGNOP Exp在这里要处理左值问题
        else if (strcmp(get_child(root, 2)->name, "ASSIGNOP") == 0)
        {
            struct Node *left = get_child(root, 1);
            Type left_type = Exp(left);
            if (left_type != NULL)
            {
                if (!((left->child_num == 1 && strcmp(get_child(left, 1)->name, "ID") == 0) ||
                      (left->child_num == 3 && strcmp(get_child(left, 2)->name, "DOT") == 0) ||
                      (left->child_num == 4 && strcmp(get_child(left, 1)->name, "Exp") == 0)))
                {
                    semant_error(6, root->line_num, "left-side must be a variable");
                }
            }
            Type right_type = Exp(get_child(root, 3));
            if (base_type_check(left_type, right_type) == false)
            {
                semant_error(5, root->line_num, "type mismatched");
            }
        }
        else
        { // 处理各种操作符运算
            type = Exp(get_child(root, 1));
            Type temp = Exp(get_child(root, 3));
            if (base_type_check(type, temp) == false)
            {
                semant_error(7, root->line_num, "type mismatched for ops");
            }
            else if (strcmp(get_child(root, 2)->name, "AND") == 0 || strcmp(get_child(root, 2)->name, "OR") == 0)
            {
                if (type != NULL && (type->kind != BASIC || type->u.basic != 0))
                {
                    semant_error(7, root->line_num, "only int type can use logical ops");
                }
                type = (Type)malloc(sizeof(struct Type_));
                type->kind = BASIC;
                type->u.basic = NUM_INT;
            }
            else if (strcmp(get_child(root, 2)->name, "RELOP") == 0)
            {
                type = (Type)malloc(sizeof(struct Type_));
                type->kind = BASIC;
                type->u.basic = NUM_INT;
            }
        }
    }

    else if (root->child_num == 4)
    {
        // ID LP Args RP
        if (strcmp(get_child(root, 1)->name, "ID") == 0)
        {
            rel = look_up(get_child(root, 1)->node_str);
            if (rel == NULL)
            {
                semant_error(2, root->line_num, "undifined function");
            }
            else if (rel->type->kind != FUNCTION)
            {
                semant_error(11, root->line_num, "Not a function");
            }
            else
            {
                if (func_check_matched(rel->type->u.func.args, Args(get_child(root, 3))) == false)
                {
                    semant_error(9, root->line_num, "function is not applicable for arguments");
                }
                else
                {
                    type = rel->type->u.func.return_type;
                }
            }
        }
        // Exp LB Exp RB
        else if (strcmp(get_child(root, 1)->name, "Exp") == 0)
        {
            Type temp = Exp(get_child(root, 1));
            if (temp->kind != ARRAY)
            {
                semant_error(10, root->line_num, "not a array");
                return NULL;
            }
            else
            {
                type = temp->u.array.elem;
            }
            Type temp2 = Exp(get_child(root, 3));
            if (temp2 != NULL && (temp2->kind != BASIC || temp2->u.basic != 0))
            {
                semant_error(12, root->line_num, "not a int ");
            }
        }
    }
    return type;
}
FieldList Args(struct Node *root)
{
    /*Args -> Exp COMMA Args
| Exp*/
    debug_print(root->name);
    print_line(root->line_num);
    if (root == NULL)
        return NULL;
    Type temp = Exp(get_child(root, 1));
    if (temp == NULL)
        return NULL;
    FieldList rel = (FieldList)malloc(sizeof(struct FieldList_));
    rel->name = "args";
    rel->type = temp;
    rel->tail = NULL;
    if (root->child_num == 3)
    {
        rel->tail = Args(get_child(root, 3));
    }
    return rel;
}

// 检查当前局部变量是否在结构体中
FieldList InStructer(char *name, FieldList struct_field)
{
    debug_print("Instructer");
    print_line(root->line_num);
    debug_print(name);
    FieldList temp = struct_field->tail;
    while (temp != NULL)
    {
        if (strcmp(temp->name, name) == 0)
        {
            return temp;
        }
        temp = temp->tail;
    }
    return NULL;
}
// 添加节点到结构体中
void add_to_structer(struct Node *node, Type type, FieldList field)
{
    debug_print("add_to_structer");
    print_line(root->line_num);
    FieldList node_field = VarDec(node, type, field);
    node_field->tail = NULL;
    FieldList temp = field;
    while (temp->tail != NULL)
    {
        temp = temp->tail;
    }
    temp->tail = node_field;
    debug_print("finish add to struct");
}
// a 是形参
bool func_check_matched(FieldList a, FieldList b)
{ // 判断函数的形参和实参是否匹配
    a = a->tail;
    if (a == NULL && b == NULL)
    {
        return true;
    }
    if (a == NULL || b == NULL)
    {
        return false;
    }
    while (true)
    {
        if (a == NULL && b == NULL)
        {
            break;
        }
        if (a == NULL || b == NULL)
        {
            return false;
        }
        if (base_type_check(a->type, b->type) == false)
        {
            return false;
        }
        a = a->tail;
        b = b->tail;
    }
    return true;
}

void add_to_args(struct Node *node, Type type, FieldList field)
{
    FieldList node_field = VarDec(node, type, field);
    node_field->tail = NULL;
    FieldList temp = field;
    while (temp->tail != NULL)
    {
        temp = temp->tail;
    }
    temp->tail = node_field;
}
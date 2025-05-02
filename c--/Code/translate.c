#include "translate.h"
extern InterCodeList intercode_head;
bool debug_or_not = false;
void if_true_debug(char *s, int line)
{
    if (debug_or_not == false)
    {
        return;
    }
    printf("now is %s : %d \n", s, line);
}
void trans_Program(struct Node *root)
{

    // Program-> ExtDefList
    intercode_head = init_ir_list(); // 初始化
    if (root == NULL)
        return;
    if_true_debug(root->name, root->line_num);
    trans_ExtDefList(get_child(root, 1));
}
void trans_ExtDefList(struct Node *root)
{
    // ExtDefList->ExtDef ExtDEfList|e
    if (root == NULL)
        return;
    if_true_debug(root->name, root->line_num);
    trans_ExtDef(get_child(root, 1));
    trans_ExtDefList(get_child(root, 2));
}
void trans_ExtDef(struct Node *root)
{
    /*ExtDef->Specifier ExtDecList SEMI
    | Specifier SEMI
    | Specifier FunDec CompSt
    */
    // 因为假设4，没有全局变量，所以只管最后一个。
    if (root == NULL)
        return;
    if_true_debug(root->name, root->line_num);
    if (root->child_num == 3)
    {
        if (strcmp(get_child(root, 2)->name, "FunDec") == 0)
        {
            trans_FuncDec(get_child(root, 2));
            trans_Compst(get_child(root, 3));
        }
    }
}

Operand trans_VarDec(struct Node *root)
{ // 这个部分是变量的声明，extdeclist（全局变量定义语句不包含结构体）、形参定义（对应的是函数定义里使用到的形参）、dec（局部变量的定义）
    /*
    VarDec->ID
    | VarDec LB INT RB
    */
    if (root == NULL)
        return NULL;
    if_true_debug(root->name, root->line_num);
    Operand res_op = NULL;
    if (root->child_num == 1)
    { // VarDec->ID
        char *var_name = get_child(root, 1)->node_str;
        Signal res = look_up(var_name);
        assert(res != NULL);
        if (res->type->kind == BASIC)
        {
            res_op = gen_operand(1, -1, -1, var_name);
        }
        // 对数组变量的定义，要为其分配内存大小
        else if (res->type->kind == ARRAY)
        { // 高维数组变量的情况
            // if (res->type->u.array.elem->kind == ARRAY)
            // {
            //     inter_error = true;
            //     return NULL;
            // }
            if (res->type->u.array.elem->kind == BASIC || res->type->u.array.elem->kind == STRUCT_TYPE)
            {
                res_op = gen_operand(2, -1, -1, var_name);
                res_op->type = res->type->u.array.elem; // 记录数组的类型
                res_op->size = res->type->u.array.size; // 记录数组的元素个数
                // 还需要为数组变量分配空间 Dec的指令
                int dec_size = get_size(res_op->type) * res_op->size;
                gen_ir(intercode_head, 9, res_op, NULL, NULL, NULL, dec_size);
            }
            else
            {
                inter_error = true;
                return NULL;
            }
        }
        else if (res->type->kind == STRUCT_TYPE)
        {
            // 处理结构体变量
            res_op = gen_operand(7, -1, -1, var_name);
            res_op->type = res->type;
            int dec_size = get_size(res->type);
            gen_ir(intercode_head, 9, res_op, NULL, NULL, NULL, dec_size);
        }
    }
    // 数组类型的,先往后看看他的作用
    else if (root->child_num == 4)
    {
        trans_VarDec(get_child(root, 1));
    }
    return res_op;
}
void trans_FuncDec(struct Node *root)
{
    /*
    Func->ID LP VarList RP
    | ID LP RP
    */
    if (root == NULL)
        return;
    if_true_debug(root->name, root->line_num);
    char *func_name = get_child(root, 1)->node_str;
    Signal func_list = look_up(func_name);
    assert(func_list != NULL);
    Operand func_op = gen_operand(3, -1, -1, func_name);
    gen_ir(intercode_head, 2, func_op, NULL, NULL, NULL, 0);
    // 处理参数部分
    if (root->child_num == 4)
    {
        FieldList temp = func_list->type->u.func.args->tail;
        while (temp != NULL)
        {
            Operand temp_op;
            switch (temp->type->kind)
            {
            case BASIC:
                temp_op = gen_operand(1, -1, -1, temp->name);
                break;
            case STRUCT_TYPE:
                // 处理结构体类型的形参，我认为这里只是形参，不需要进行取地址等操作
                temp_op = gen_operand(7, -1, -1, temp->name);
                insert_param(temp->name);
                break;
            case ARRAY:
                inter_error = true;
                break;
            default:
                break;
            }
            gen_ir(intercode_head, 6, temp_op, NULL, NULL, NULL, 0);
            temp = temp->tail;
        }
    }
}
void trans_Compst(struct Node *root)
{
    // CompSt -> LC DefList StmtList RC
    if (root == NULL)
        return;
    if_true_debug(root->name, root->line_num);
    trans_DefList(get_child(root, 2));
    trans_StmtList(get_child(root, 3));
}
void trans_StmtList(struct Node *root)
{
    /*
    StmtList -> Stmt StmtList
| e
    */
    if (root == NULL)
        return;
    if_true_debug(root->name, root->line_num);
    trans_Stmt(get_child(root, 1));
    trans_StmtList(get_child(root, 2));
}
void trans_Stmt(struct Node *root)
{ // 语句块里面的语句部分，也就是运算部分
    /*
    Stmt -> Exp SEMI
| CompSt
| RETURN Exp SEMI
| IF LP Exp RP Stmt
| IF LP Exp RP Stmt ELSE Stmt
| WHILE LP Exp RP Stmt
    */
    if (root == NULL)
        return;
    if_true_debug(root->name, root->line_num);
    if (root->child_num == 1)
    {
        trans_Compst(get_child(root, 1));
    }
    else if (root->child_num == 2)
    {
        trans_Exp(get_child(root, 1), new_temp()); // Exp的结果应该存在一个临时变量中,但可以看到这里不用打印出来这个临时变量。
    }
    else if (root->child_num == 3)
    { // RETURN Exp SEMI
        Operand temp = new_temp();
        trans_Exp(get_child(root, 2), temp);
        temp = load_value(temp);
        gen_ir(intercode_head, 4, temp, NULL, NULL, NULL, 0);
    }
    else if (root->child_num == 5)
    {
        if (strcmp(get_child(root, 1)->name, "IF") == 0)
        { // IF LP Exp RP Stmt
            Operand true_label = new_label();
            Operand false_label = new_label();
            trans_Cond(get_child(root, 3), true_label, false_label);
            // true_label
            gen_ir(intercode_head, 1, true_label, NULL, NULL, NULL, 0);
            trans_Stmt(get_child(root, 5));
            // false_label
            // if (strcmp(get_child(get_child(root, 5), 1)->name, "RETURN") != 0)
            gen_ir(intercode_head, 1, false_label, NULL, NULL, NULL, 0);
        }
        else //  WHILE LP Exp RP Stmt
        {
            Operand temp1 = new_label();
            Operand temp2 = new_label();
            Operand temp3 = new_label();
            gen_ir(intercode_head, 1, temp1, NULL, NULL, NULL, 0);
            trans_Cond(get_child(root, 3), temp2, temp3);
            gen_ir(intercode_head, 1, temp2, NULL, NULL, NULL, 0);
            trans_Stmt(get_child(root, 5));
            gen_ir(intercode_head, 3, temp1, NULL, NULL, NULL, 0);
            gen_ir(intercode_head, 1, temp3, NULL, NULL, NULL, 0);
        }
    }
    else if (root->child_num == 7)
    { // IF LP EXP RP STMT ELSE STMT
        Operand temp1 = new_label();
        Operand temp2 = new_label();
        Operand temp3 = new_label();
        trans_Cond(get_child(root, 3), temp1, temp2);
        gen_ir(intercode_head, 1, temp1, NULL, NULL, NULL, 0);
        trans_Stmt(get_child(root, 5));
        if (strcmp(get_child(get_child(root, 5), 1)->name, "RETURN") != 0)
        {
            gen_ir(intercode_head, 3, temp3, NULL, NULL, NULL, 0);
        }
        gen_ir(intercode_head, 1, temp2, NULL, NULL, NULL, 0);
        trans_Stmt(get_child(root, 7));
        if (strcmp(get_child(get_child(root, 7), 1)->name, "RETURN") != 0)
        {
            gen_ir(intercode_head, 1, temp3, NULL, NULL, NULL, 0);
        }
    }
}
void trans_DefList(struct Node *root)
{
    // 局部变量的定义部分
    /*
    DefList -> Def DefList
| e
    */
    if (root == NULL)
        return;
    if_true_debug(root->name, root->line_num);
    trans_Def(get_child(root, 1));
    trans_DefList(get_child(root, 2));
}
void trans_Def(struct Node *root)
{
    /*
    Def -> Specifier DecList SEMI
    */
    if (root == NULL)
        return;
    if_true_debug(root->name, root->line_num);
    trans_DecList(get_child(root, 2));
}
void trans_DecList(struct Node *root)
{
    /*
    DecList -> Dec
| Dec COMMA DecList
    */
    if (root == NULL)
        return;
    if_true_debug(root->name, root->line_num);
    if (root->child_num == 1)
        trans_Dec(get_child(root, 1));
    else if (root->child_num == 3)
    {
        trans_Dec(get_child(root, 1));
        trans_DecList(get_child(root, 3));
    }
}
void trans_Dec(struct Node *root)
{
    // 局部变量的定义部分
    /*
    Dec -> VarDec
| VarDec ASSIGNOP Exp
    */
    if (root == NULL)
        return;
    if_true_debug(root->name, root->line_num);
    if (root->child_num == 1)
    {
        trans_VarDec(get_child(root, 1));
    }
    else if (root->child_num == 3)
    {
        // 变量的初始化，不涉及结构体的，主要就是数组的深拷贝。
        Operand res_op = trans_VarDec(get_child(root, 1));
        Operand temp = new_temp();
        trans_Exp(get_child(root, 3), temp);
        if (res_op->kind == OP_VARIABLE)
        {
            temp = load_value(temp); // 防止temp是地址的情况
            gen_ir(intercode_head, 10, res_op, temp, NULL, NULL, 0);
        }
        else if (res_op->kind == OP_ARRAY)
        {
            deep_copy(res_op, temp);
        }
    }
}
void trans_Exp(struct Node *root, Operand place)
{
    /*
    Exp -> Exp ASSIGNOP Exp
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
| FLOAT
    */
    if (root == NULL)
        return;
    if_true_debug(root->name, root->line_num);
    if (root->child_num == 1)
    {
        if (strcmp(get_child(root, 1)->name, "ID") == 0)
        {
            char *var_name = get_child(root, 1)->node_str;
            Signal res = look_up(var_name);
            assert(res != NULL);
            if (res->type->kind == BASIC)
            {
                place->kind = OP_VARIABLE;
                place->u.var_name = var_name;
            }
            else if (res->type->kind == ARRAY)
            {
                // 数组类型相关的
                place->kind = OP_ARRAY;
                place->type = res->type->u.array.elem;
                place->size = res->type->u.array.size;
                place->u.array_name = var_name;
            }
            else if (res->type->kind == STRUCT_TYPE)
            {
                // 结构体相关的
                place->kind = OP_STRUCTURE;
                place->type = res->type;
                place->u.struct_name = var_name;
            }
        }
        else if (strcmp(get_child(root, 1)->name, "INT") == 0)
        {
            place->kind = OP_CONSTANT;
            place->u.cons_val = get_child(root, 1)->node_int;
        }
        // 假设1是不存在浮点数的，因此省略。
    }
    // 和条件判断相关的几个式子
    else if ((root->child_num == 2 && strcmp(get_child(root, 1)->name, "NOT") == 0) ||
             (root->child_num == 3 && (strcmp(get_child(root, 2)->name, "AND") == 0 || strcmp(get_child(root, 1)->name, "OR") == 0 ||
                                       strcmp(get_child(root, 2)->name, "RELOP") == 0)))
    {
        printf("################3 now is %s ##################3", get_child(root, 2)->name);
        Operand temp1 = new_label();
        Operand temp2 = new_label();
        // place=0
        gen_ir(intercode_head, 10, place, gen_operand(6, 0, -1, NULL), NULL, NULL, -1);
        trans_Cond(root, temp1, temp2);
        gen_ir(intercode_head, 1, temp1, NULL, NULL, NULL, -1);
        // place=1
        gen_ir(intercode_head, 10, place, gen_operand(6, 1, -1, NULL), NULL, NULL, -1);
        gen_ir(intercode_head, 1, temp2, NULL, NULL, NULL, -1);
    }
    else if (root->child_num == 2)
    {
        if (strcmp(get_child(root, 1)->name, "MINUS") == 0)
        {
            Operand temp = new_temp();
            trans_Exp(get_child(root, 2), temp);
            temp = load_value(temp);
            if (temp->kind == OP_CONSTANT)
            {
                place->kind = OP_CONSTANT;
                place->u.cons_val = -1 * temp->u.cons_val;
            }
            else
            {
                gen_ir(intercode_head, 16, place, gen_operand(6, 0, -1, NULL), temp, NULL, -1);
            }
        }
    }
    else if (root->child_num == 3)
    {
        if (strcmp(get_child(root, 1)->name, "LP") == 0)
        { // LP Exp RP
            trans_Exp(get_child(root, 2), place);
        }
        else if (strcmp(get_child(root, 1)->name, "ID") == 0)
        { // ID LP RP
            char *func_name = get_child(root, 1)->node_str;
            Signal res = look_up(func_name);
            assert(res != NULL);
            if (strcmp(func_name, "read") == 0)
            {
                gen_ir(intercode_head, 7, place, NULL, NULL, NULL, -1);
            }
            else
            {
                Operand func_op = gen_operand(3, -1, -1, func_name);
                gen_ir(intercode_head, 14, place, func_op, NULL, NULL, -1);
            }
        }
        else if (strcmp(get_child(root, 2)->name, "DOT") == 0)
        { // Exp DOT ID
          // 这是对应的什么？结构体的调用 其实应该返回的是一个地址
            printf("############ here dot ##########\n");
            Operand a_temp = new_temp();
            trans_Exp(get_child(root, 1), a_temp);
            printf("exp is %s\n", a_temp->type->u.structer->name);
            assert(a_temp->type->kind == STRUCT_TYPE);
            char *id_name = get_child(root, 3)->node_str;
            FieldList temp = InStructer(id_name, a_temp->type->u.structer);
            printf("id_name is %s temp type is %d\n", temp->name, temp->type->kind);
            assert(temp != NULL);
            Operand base = new_temp();
            if (a_temp->kind == OP_ADDRESS)
            {
                gen_ir(intercode_head, 10, base, a_temp, NULL, NULL, -1);
            }
            else
                gen_ir(intercode_head, 11, base, a_temp, NULL, NULL, -1);
            int offset = 0;
            // Signal s = look_up(get_child(get_child(root, 1), 1)->node_str);
            // assert(s != NULL && s->type->kind == STRUCT_TYPE);
            // FieldList s_temp = s->type->u.structer->tail;
            FieldList s_temp = a_temp->type->u.structer->tail;
            while (s_temp != NULL)
            {
                if (strcmp(s_temp->name, id_name) == 0)
                {
                    break;
                }
                offset += get_size(s_temp->type);
                s_temp = s_temp->tail;
            }
            place->kind = OP_ADDRESS;
            address_number++;
            gen_ir(intercode_head, 15, place, base, gen_operand(6, offset, -1, NULL), NULL, -1);
            // if (temp->type->kind == STRUCT_TYPE)
            // {
            //     if (is_param(s->name))
            //     {
            //         insert_param(temp->name);
            //     }
            //     place->kind = OP_STRUCTURE;
            //     place->type = temp;
            //     place->u.struct_name = temp->name;
            // }
            place->kind = OP_VARIABLE;
            place->u.var_name = temp->name;

            if (temp->type->kind == ARRAY)
            {
                place->kind = OP_ARRAY;
                insert_param(temp->name);
                place->type = temp->type->u.array.elem;
                place->size = temp->type->u.array.size;
                place->u.array_name = temp->name;
                int num = 0;
                FieldList new_s = a_temp->type->u.structer->tail;
                while (new_s != NULL)
                {
                    if (strcmp(new_s->name, place->u.array_name) == 0)
                    {
                        place->add_pre = num;
                        break;
                    }
                    if (new_s->type->kind == ARRAY)
                    {
                        num += get_size(new_s->type->u.array.elem) * new_s->type->u.array.size;
                    }
                    else
                    {
                        num += get_size(new_s->type);
                    }
                    new_s = new_s->tail;
                }
            }
            // else if (temp->type->kind == STRUCT_TYPE)
            // {
            //     // place->kind = OP_STRUCTURE;
            //     place->type = temp->type;
            //     place->u.struct_name = temp->name;
            // }
        }
        else if (strcmp(get_child(root, 2)->name, "ASSIGNOP") == 0)
        { // Exp ASSIGNOP Exp
            Operand left = new_temp();
            Operand right = new_temp();
            trans_Exp(get_child(root, 1), left);
            trans_Exp(get_child(root, 3), right);
            // 左值需要求地址的情况
            if (left->kind == OP_ADDRESS || left->kind == OP_ARRAY)
            {
                // 右值也要从地址中取值
                if (right->kind == OP_ADDRESS || right->kind == OP_ARRAY)
                {
                    // 这个时候应该要深拷贝
                    deep_copy(left, right);
                }
                else // 右值就是简单的赋值
                {
                    gen_ir(intercode_head, 13, left, right, NULL, NULL, -1);
                }
            }
            // 左值就是普通的赋值
            else
            {
                right = load_value(right);
                gen_ir(intercode_head, 10, left, right, NULL, NULL, -1);
            }
            place->kind = right->kind;
            place->u = right->u;
        }
        else // 一些算术的式子
        {

            Operand temp1 = new_temp();
            trans_Exp(get_child(root, 1), temp1);
            temp1 = load_value(temp1);
            Operand temp2 = new_temp();
            trans_Exp(get_child(root, 3), temp2);
            temp2 = load_value(temp2);
            int kind = -1;
            long long val;
            if (strcmp(get_child(root, 2)->name, "PLUS") == 0)
            {
                kind = 15;
                val = temp1->u.cons_val + temp2->u.cons_val;
            }
            else if (strcmp(get_child(root, 2)->name, "MINUS") == 0)
            {
                kind = 16;
                val = temp1->u.cons_val - temp2->u.cons_val;
            }
            else if (strcmp(get_child(root, 2)->name, "STAR") == 0)
            {
                printf("###########3 here #############\n");
                kind = 17;
                val = temp1->u.cons_val * temp2->u.cons_val;
            }
            else if (strcmp(get_child(root, 2)->name, "DIV") == 0)
            {
                kind = 18;
                val = temp1->u.cons_val / temp2->u.cons_val;
            }
            if (temp1->kind == OP_CONSTANT && temp2->kind == OP_CONSTANT)
            {
                place->kind == OP_CONSTANT;
                place->u.cons_val = val;
            }
            else
            {
                gen_ir(intercode_head, kind, place, temp1, temp2, NULL, -1);
            }
        }
    }
    else if (root->child_num == 4)
    { // 函数调用部分
        if (strcmp(get_child(root, 1)->name, "ID") == 0)
        {
            char *func_name = get_child(root, 1)->node_str;
            Signal res = look_up(func_name);
            assert(res != NULL);
            if (strcmp(func_name, "write") == 0)
            {
                trans_Args(get_child(root, 3), true);
                place->kind = OP_CONSTANT;
                place->u.cons_val = 0;
            }
            else
            {
                trans_Args(get_child(root, 3), false);
                Operand temp = gen_operand(3, -1, -1, func_name);
                gen_ir(intercode_head, 14, place, temp, NULL, NULL, -1);
            }
        }
        else if (strcmp(get_child(root, 1)->name, "Exp") == 0)
        { // Exp LB Exp RB 对应a[i]这样的情况
            Operand temp1 = new_temp();
            trans_Exp(get_child(root, 1), temp1);
            Operand temp2 = new_temp();
            trans_Exp(get_child(root, 3), temp2);
            Operand offset = new_temp();
            int width = get_size(temp1->type);
            // if (temp2->kind == OP_ARRAY)
            // {
            //     inter_error = true;
            //     return;
            // }
            if (temp2->kind == OP_CONSTANT)
            {
                offset->kind = OP_CONSTANT;
                offset->u.cons_val = width * (temp2->u.cons_val) + temp1->add_pre;
            }
            else
            {
                gen_ir(intercode_head, 17, offset, temp2, gen_operand(6, width, -1, NULL), NULL, -1);
            }
            // 将place设置为ADDRESS类型，名字为临时变量编号
            free(place);
            place = new_address();
            // place->kind = OP_ADDRESS;
            // place->u.addr_no = address_number++;

            if (temp1->kind == OP_ARRAY)
            { // Exp1-> ID
                printf("offset type %d\n", offset->kind);
                if (offset->kind == OP_CONSTANT && offset->u.cons_val == 0)
                {
                    gen_ir(intercode_head, 11, place, temp1, NULL, NULL, -1);
                }
                else
                {
                    Operand base = new_address();
                    // base := &addr
                    gen_ir(intercode_head, 11, base, temp1, NULL, NULL, -1);
                    // place := base + offset
                    gen_ir(intercode_head, 15, place, base, offset, NULL, -1);
                }
            }
            // 处理结构体数组的情况
            //  if (temp1->type->kind == BASIC)
            //  { // 数组解析完毕
            //      place->type = NULL;
            //      place->size = 1;
            //  }
            //  else if (temp1->type->kind == ARRAY)
            //  {
            //      place->type = temp1->type->u.array.elem;
            //      place->size = temp1->type->u.array.size;
            //  }
            if (temp1->type->kind == STRUCT_TYPE)
            {
                printf("here? \n");
                // place->kind = OP_STRUCTURE;
                place->type = temp1->type;
                place->size = 1;
            }
        }
    }
}
void trans_Args(struct Node *root, bool write_func)
{
    /*
    Args -> Exp COMMA Args
| Exp
    */
    if (root == NULL)
        return;
    if_true_debug(root->name, root->line_num);
    Operand temp = new_temp();
    trans_Exp(get_child(root, 1), temp);
    if (root->child_num == 3)
    {
        trans_Args(get_child(root, 3), write_func);
    }
    if (write_func == true)
    {
        temp = load_value(temp);
        gen_ir(intercode_head, 8, temp, NULL, NULL, NULL, -1);
    }
    if (temp->kind == OP_STRUCTURE)
    {
        // 处理结构体作为参数的情况
        Operand addr = new_address();
        gen_ir(intercode_head, 11, addr, temp, NULL, NULL, -1);
        gen_ir(intercode_head, 5, addr, NULL, NULL, NULL, -1);
    }
    else
    {
        temp = load_value(temp);
        // 似乎需要再这里处理一下结构体作为参数的情况。
        gen_ir(intercode_head, 5, temp, NULL, NULL, NULL, -1);
    }
}
void trans_Cond(struct Node *root, Operand true_label, Operand false_label)
{
    if (root == NULL)
        return;
    if (root->child_num == 2 && strcmp(get_child(root, 1)->name, "NOT") == 0)
    {
        trans_Cond(get_child(root, 1), false_label, true_label);
    }
    else if (root->child_num == 3 && strcmp(get_child(root, 2)->name, "AND") == 0)
    { // A AND B
        Operand temp_label = new_label();
        trans_Cond(get_child(root, 1), temp_label, false_label);
        gen_ir(intercode_head, 1, temp_label, NULL, NULL, NULL, 0);
        trans_Cond(get_child(root, 3), true_label, false_label);
    }
    else if (root->child_num == 3 && strcmp(get_child(root, 2)->name, "OR") == 0)
    {
        Operand temp_label = new_label();
        trans_Cond(get_child(root, 1), true_label, temp_label);
        gen_ir(intercode_head, 1, temp_label, NULL, NULL, NULL, 0);
        trans_Cond(get_child(root, 3), true_label, false_label);
    }
    else if (root->child_num == 3 && strcmp(get_child(root, 2)->name, "RELOP") == 0)
    { // Exp RELOP Exp
        Operand temp1 = new_temp();
        trans_Exp(get_child(root, 1), temp1);
        temp1 = load_value(temp1);
        Operand temp2 = new_temp();
        trans_Exp(get_child(root, 3), temp2);
        temp2 = load_value(temp2);
        char *relop = get_child(root, 2)->node_str;
        // if temp1 op t2 goto true_label
        gen_ir(intercode_head, 19, temp1, temp2, true_label, relop, -1);
        // goto false_label
        gen_ir(intercode_head, 3, false_label, NULL, NULL, NULL, -1);
    }
    else
    {
        Operand temp = new_temp();
        trans_Exp(root, temp);
        gen_ir(intercode_head, 19, temp, gen_operand(6, 0, -1, NULL), true_label, "!=", -1);
        gen_ir(intercode_head, 3, false_label, NULL, NULL, NULL, 0);
    }
}
// 算数据类型的内存吧
int get_size(Type type)
{
    if (type == NULL)
        return 0;
    if (type->kind == BASIC)
    {
        return 4;
    }
    else if (type->kind == ARRAY)
    {
        return type->u.array.size * get_size(type->u.array.elem);
    }
    else if (type->kind == STRUCT_TYPE)
    {
        int size = 0;
        FieldList temp = type->u.structer->tail;
        while (temp != NULL)
        {
            size += get_size(temp->type);
            temp = temp->tail;
        }
        return size;
    }
    return 0;
}

void deep_copy(Operand left, Operand right)
{
    Operand left_base = get_address(left, false);
    Operand right_base = get_address(right, false);
    int size_left = get_size(left->type) * left->size;
    int size_right = get_size(right->type) * right->size;
    int size = size_left < size_right ? size_left : size_right; // 要根据了left的长度进行截断
    assert(size % 4 == 0);
    Operand left1 = new_address();
    Operand right1 = new_address();
    Operand val = new_temp();
    gen_ir(intercode_head, 12, val, right_base, NULL, NULL, -1);
    gen_ir(intercode_head, 13, left_base, val, NULL, NULL, -1);
    for (int i = 4; i < size; i += 4)
    {
        Operand offset = gen_operand(6, i, -1, NULL);
        gen_ir(intercode_head, 15, left1, left_base, offset, NULL, -1);
        gen_ir(intercode_head, 15, right1, right_base, offset, NULL, -1);
        gen_ir(intercode_head, 12, val, right1, NULL, NULL, -1);
        gen_ir(intercode_head, 13, left1, val, NULL, NULL, -1);
    }
    return left_base;
}
Operand get_address(Operand op, bool is_arg)
{
    if (op->kind != OP_ARRAY)
        return op;
    Operand place = new_address();
    gen_ir(intercode_head, 11, place, op, NULL, NULL, -1);
    return place;
}
Operand load_value(Operand a)
{
    if (a->kind != OP_ADDRESS)
        return a;
    Operand place = new_temp();
    gen_ir(intercode_head, 12, place, a, NULL, NULL, -1);
    return place;
}
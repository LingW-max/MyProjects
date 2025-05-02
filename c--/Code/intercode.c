#include "intercode.h"

InterCodeList init_ir_list()
{
    InterCodeList root = (InterCodeList)malloc(sizeof(struct InterCodeList_));
    assert(root != NULL);
    root->code = NULL;
    root->prev = root->next = root;
    int temp_number = 0;
    int label_number = 0;
    int address_number = 0;
    bool inter_error = false;
    Param_s param_head = NULL;
    return root;
}

void add_ir(InterCodeList root, InterCode new_ir)
{
    if (root == NULL)
        return;
    InterCodeList new_term = (InterCodeList)malloc(sizeof(struct InterCodeList_));
    assert(new_term != NULL);
    new_term->code = new_ir;
    InterCodeList temp = root->prev;
    root->prev = new_term;
    temp->next = new_term;
    new_term->prev = temp;
    new_term->next = root;
}

void delete_ir(InterCodeList ir) // 删除某条指令
{
    if (ir == NULL)
        return;
    InterCodeList prev = ir->prev;
    InterCodeList next = ir->next;
    assert(prev != next && ir != next && ir != prev);
    // 不应出现只有一条指令或者只有头部的情况
    prev->next = next;
    next->prev = prev;
    free(ir);
}
// 双向环形链表
void print_ir_list(InterCodeList root, FILE *out_file)
{
    if (inter_error == true)
    {
        printf("Cannot translate: Code contains variables of multi-dimensional array type or\
parameters of array type\n");
        return;
    }
    InterCodeList temp = root->next;
    while (temp != root)
    {
        assert(temp->code != NULL);
        print_ir(temp->code, out_file);
        temp = temp->next;
    }
}

void print_ir(InterCode ir, FILE *out_file)
{
    if (ir == NULL || out_file == NULL)
        return;
    switch (ir->kind)
    {
    case IR_LABEL:
        fprintf(out_file, "LABEL ");
        print_op(ir->u.unary_ir.op, out_file);
        fprintf(out_file, ": ");
        break;
    case IR_FUNCTION:
        fprintf(out_file, "FUNCTION ");
        print_op(ir->u.unary_ir.op, out_file);
        fprintf(out_file, ": ");
        break;
    case IR_GOTO:
        fprintf(out_file, "GOTO ");
        print_op(ir->u.unary_ir.op, out_file);
        break;
    case IR_RETURN:
        fprintf(out_file, "RETURN ");
        print_op(ir->u.unary_ir.op, out_file);
        break;
    case IR_ARG:
        fprintf(out_file, "ARG ");
        print_op(ir->u.unary_ir.op, out_file);
        break;
    case IR_PARAM:
        fprintf(out_file, "PARAM ");
        print_op(ir->u.unary_ir.op, out_file);
        break;
    case IR_READ:
        fprintf(out_file, "READ ");
        print_op(ir->u.unary_ir.op, out_file);
        break;
    case IR_WRITE:
        fprintf(out_file, "WRITE ");
        print_op(ir->u.unary_ir.op, out_file);
        break;
    case IR_ASSIGN:
        print_op(ir->u.binary_ir.left, out_file);
        fprintf(out_file, ":= ");
        print_op(ir->u.binary_ir.right, out_file);
        break;
    case IR_ADDR:
        print_op(ir->u.binary_ir.left, out_file);
        fprintf(out_file, ":= ");
        if (!is_param(ir->u.binary_ir.right->u.struct_name))
        { // 如果是形参的结构体，那就不打印&
            fprintf(out_file, "&");
        }
        print_op(ir->u.binary_ir.right, out_file);
        break;
    case IR_LOAD:
        print_op(ir->u.binary_ir.left, out_file);
        fprintf(out_file, ":= *");
        print_op(ir->u.binary_ir.right, out_file);
        break;
    case IR_STORE:
        fprintf(out_file, "*");
        print_op(ir->u.binary_ir.left, out_file);
        fprintf(out_file, ":= ");
        print_op(ir->u.binary_ir.right, out_file);
        break;
    case IR_CALL:
        print_op(ir->u.binary_ir.left, out_file);
        fprintf(out_file, ":= CALL ");
        print_op(ir->u.binary_ir.right, out_file);
        break;
    case IR_ADD:
        print_op(ir->u.ternary_ir.res, out_file);
        fprintf(out_file, ":= ");
        print_op(ir->u.ternary_ir.op1, out_file);
        fprintf(out_file, "+ ");
        print_op(ir->u.ternary_ir.op2, out_file);
        break;
    case IR_SUB:
        print_op(ir->u.ternary_ir.res, out_file);
        fprintf(out_file, ":= ");
        print_op(ir->u.ternary_ir.op1, out_file);
        fprintf(out_file, "- ");
        print_op(ir->u.ternary_ir.op2, out_file);
        break;
    case IR_MUL:
        print_op(ir->u.ternary_ir.res, out_file);
        fprintf(out_file, ":= ");
        print_op(ir->u.ternary_ir.op1, out_file);
        fprintf(out_file, "* ");
        print_op(ir->u.ternary_ir.op2, out_file);
        break;
    case IR_DIV:
        print_op(ir->u.ternary_ir.res, out_file);
        fprintf(out_file, ":= ");
        print_op(ir->u.ternary_ir.op1, out_file);
        fprintf(out_file, "/ ");
        print_op(ir->u.ternary_ir.op2, out_file);
        break;
    case IR_IF_GOTO:
        fprintf(out_file, "IF ");
        print_op(ir->u.if_goto.x, out_file);
        fprintf(out_file, "%s ", ir->u.if_goto.relop);
        print_op(ir->u.if_goto.y, out_file);
        fprintf(out_file, "GOTO ");
        print_op(ir->u.if_goto.z, out_file);
        break;
    case IR_DEC:
        fprintf(out_file, "DEC ");
        print_op(ir->u.dec.op, out_file);
        fprintf(out_file, "%d ", ir->u.dec.size);
        break;
    default:
        break;
    }
    fprintf(out_file, "\n");
}
void print_op(Operand op, FILE *out_file)
{
    if (op == NULL)
        return;
    switch (op->kind)
    {
    case OP_VARIABLE:
        fprintf(out_file, "%s ", op->u.var_name);
        break;
    case OP_FUNCTION:
        fprintf(out_file, "%s ", op->u.func_name);
        break;
    case OP_LABEL:
        fprintf(out_file, "label%d ", op->u.label_no);
        break;
    case OP_TEMP:
        fprintf(out_file, "t%d ", op->u.temp_no);
        break;
    case OP_CONSTANT:
        fprintf(out_file, "#%lld ", op->u.cons_val);
        break;
    case OP_ADDRESS:
        fprintf(out_file, "addr%d ", op->u.addr_no);
        break;
    case OP_ARRAY:
        fprintf(out_file, "%s ", op->u.array_name);
        break;
    case OP_STRUCTURE:
        fprintf(out_file, "%s ", op->u.struct_name);
        break;
    default:
        break;
    }
}
// 生成新指令
InterCode gen_ir(InterCodeList intercode_head, int ir_kind, Operand op1, Operand op2, Operand op3, char *relop, int dec_size)
{
    InterCode new_ir = (InterCode)malloc(sizeof(struct InterCode_));
    assert(new_ir != NULL);
    new_ir->kind = ir_kind;
    switch (ir_kind)
    {
        // 一元的指令
    case IR_LABEL:
    case IR_FUNCTION:
    case IR_GOTO:
    case IR_RETURN:
    case IR_ARG:
    case IR_PARAM:
    case IR_READ:
    case IR_WRITE:
        if (op1 == NULL)
        {
            free(new_ir);
            new_ir = NULL;
        }
        else
        {
            new_ir->u.unary_ir.op = op1;
        }
        break;
        // DEC指令部分
    case IR_DEC:
        if (op1 == NULL)
        {
            free(new_ir);
            new_ir = NULL;
        }
        else
        {
            new_ir->u.dec.op = op1;
            new_ir->u.dec.size = dec_size;
        }
        break;
        // 二元的指令
    case IR_ASSIGN:
    case IR_ADDR:
    case IR_LOAD:
    case IR_STORE:
    case IR_CALL:
        if (op1 == NULL || op2 == NULL)
        {
            free(new_ir);
            new_ir = NULL;
        }
        else
        {
            new_ir->u.binary_ir.left = op1;
            new_ir->u.binary_ir.right = op2;
        }
        break;
        // 三元的指令
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
        if (op1 == NULL || op2 == NULL || op3 == NULL)
        {
            free(new_ir);
            new_ir = NULL;
        }
        else
        {
            new_ir->u.ternary_ir.res = op1;
            new_ir->u.ternary_ir.op1 = op2;
            new_ir->u.ternary_ir.op2 = op3;
        }
        break;
        // 特殊的指令对于if_goto的
    case IR_IF_GOTO:
        if (op1 == NULL || op2 == NULL || op3 == NULL)
        {
            free(new_ir);
            new_ir = NULL;
        }
        else
        {
            new_ir->u.if_goto.x = op1;
            new_ir->u.if_goto.y = op2;
            new_ir->u.if_goto.z = op3;
            // strcpy(relop, new_ir->u.if_goto.relop);
            new_ir->u.if_goto.relop = relop;
        }
        break;
    default:
        break;
    }
    if (intercode_head)
        add_ir(intercode_head, new_ir);
    return new_ir;
}
// 生成操作数,number是记录临时变量和label的编号的，name是函数、变量、数组的名字的，val是常数值
Operand gen_operand(int op_kind, int cont, int number, char *name)
{
    Operand new_op = (Operand)malloc(sizeof(struct Operand_));
    assert(new_op != NULL);
    new_op->kind = op_kind;
    new_op->add_pre = 0;
    Signal res = NULL; // 记录查表结果的
    switch (op_kind)
    {
    case OP_FUNCTION:
        res = look_up(name);
        assert(res != NULL && res->type->kind == FUNCTION);
        new_op->u.func_name = name;
        break;
    case OP_ARRAY:
        res = look_up(name);
        assert(res != NULL && res->type->kind == ARRAY);
        new_op->u.array_name = name;
        break;
    case OP_VARIABLE:
        res = look_up(name);
        assert(res != NULL && res->type->kind == BASIC);
        new_op->u.var_name = name;
        break;
    case OP_LABEL:
        new_op->u.label_no = number;
        break;
    case OP_TEMP:
        new_op->u.temp_no = number;
        break;
    case OP_CONSTANT:
        new_op->u.cons_val = cont;
        break;
    case OP_ADDRESS:
        new_op->u.addr_no = number;
        break;
    case OP_STRUCTURE:
        res = look_up(name);
        assert(res != NULL && res->type->kind == STRUCT_TYPE);
        new_op->u.struct_name = name;
        break;
    default:
        break;
    }
    new_op->size = 1;
    return new_op;
}

Operand new_temp()
{
    temp_number++;
    Operand res = gen_operand(5, -1, temp_number, NULL);
    return res;
}
Operand new_address()
{
    address_number++;
    Operand res = gen_operand(8, -1, address_number, NULL);
    return res;
}
Operand new_label()
{
    label_number++;
    Operand res = gen_operand(4, -1, label_number, NULL);
    return res;
}

void insert_param(char *name)
{
    if (name == NULL)
        return;
    Param_s new_param = (Param_s)malloc(sizeof(struct Param_s_));
    new_param->name = name;
    new_param->next = param_head;
    param_head = new_param;
    return;
}
bool is_param(char *name)
{
    if (name == NULL)
        return false;
    Param_s temp = param_head;
    while (temp != NULL)
    {
        if (strcmp(temp->name, name) == 0)
        {
            return true;
        }
        temp = temp->next;
    }
    return false;
}
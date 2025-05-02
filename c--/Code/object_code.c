#include "object_code.h"
bool debugf = true;
void debug_printf(char *str)
{
    if (debugf == false)
        return;
    printf("%s\n", str);
}
// 寄存器的别名
char *reg_names[] = {"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
                     "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"};

void init_environment(FILE *code_out)
{
    if (code_out == NULL)
        debug_printf("code_out is null");
    debug_printf("init_environment");
    // head
    fprintf(code_out, ".data\n");
    fprintf(code_out, "_prompt: .asciiz \"Enter an integer:\"\n");
    fprintf(code_out, "_ret: .asciiz \"\\n\"\n");
    fprintf(code_out, ".globl main\n");

    // read
    fprintf(code_out, ".text\n");
    fprintf(code_out, "read:\n");
    fprintf(code_out, "  li $v0, 4\n");
    fprintf(code_out, "  la $a0, _prompt\n");
    fprintf(code_out, "  syscall\n");
    fprintf(code_out, "  li $v0, 5\n");
    fprintf(code_out, "  syscall\n");
    fprintf(code_out, "  jr $ra\n");
    fprintf(code_out, "\n");

    // write
    fprintf(code_out, "write:\n");
    fprintf(code_out, "  li $v0, 1\n");
    fprintf(code_out, "  syscall\n");
    fprintf(code_out, "  li $v0, 4\n");
    fprintf(code_out, "  la $a0, _ret\n");
    fprintf(code_out, "  syscall\n");
    fprintf(code_out, "  move $v0, $0\n");
    fprintf(code_out, "  jr $ra\n");
    fprintf(code_out, "\n");
}
// 初始化寄存器
void init_registers()
{
    debug_printf("init_registers");
    for (int i = 0; i < 32; i++)
    {
        regs[i].name = reg_names[i];
        regs[i].state = FREE;
        regs[i].var = NULL;
    }
}
// 初始化函数链表
void init_funclist()
{
    debug_printf("init_funclist");
    local_funclist = NULL;
}
// 插入函数，头插
void insert_func(char *name)
{
    debug_printf("insert_func");
    func_list temp = (func_list)malloc(sizeof(struct FUNC_list_));
    debug_printf("0hhhh");
    temp->func = (fucn_v)malloc(sizeof(struct FUNC_V_));
    temp->func->func_name = name;
    debug_printf("1");
    temp->func->sp_offset = -1;
    debug_printf("2");
    temp->next = local_funclist;
    debug_printf("3");
    local_funclist = temp;
    debug_printf("4");
    debug_printf("here");
}
// 找到对应函数
func_list find_func(char *name)
{
    func_list temp = local_funclist;
    while (temp != NULL)
    {
        if (strcmp(temp->func->func_name, name) == 0)
            return temp;
        temp = temp->next;
    }
}
// 为每个函数确定栈的大小
void insert_funcoffset(int offset)
{
    local_funclist->func->sp_offset = offset;
}
// 初始化变量链表
void init_varlist()
{
    debug_printf("init_varlist");
    local_varlist = NULL;
}
// 插入变量
void insert_var(Variable var)
{ // 头插
    debug_printf("insert_var");
    VariableList temp = (VariableList)malloc(sizeof(struct VariableList_));
    assert(temp);
    temp->var = var;
    temp->next = local_varlist;
    local_varlist = temp;
}
// 查找操作符对应的变量是不是在链表中
Variable find_var(Operand op)
{
    debug_printf("find_var");
    assert(op);
    if (op->kind == OP_CONSTANT)
        return NULL;
    assert(op->kind != OP_FUNCTION && op->kind != OP_LABEL);
    for (VariableList cur = local_varlist; cur; cur = cur->next)
    {
        if (cur->var->op->kind != op->kind)
            continue;
        switch (op->kind)
        {
        case OP_STRUCTURE:
            if (strcmp(cur->var->op->u.struct_name, op->u.struct_name) == 0)
            {
                return cur->var;
            }
        case OP_TEMP:
            if (cur->var->op->u.temp_no == op->u.temp_no)
            {
                return cur->var;
            }
            break;
        case OP_ARRAY:
            if (strcmp(cur->var->op->u.array_name, op->u.array_name) == 0)
                return cur->var;
            break;
        case OP_VARIABLE:
            if (strcmp(cur->var->op->u.var_name, op->u.var_name) == 0)
                return cur->var;
            break;
        case OP_ADDRESS:
            if (cur->var->op->u.addr_no == op->u.addr_no)
                return cur->var;
            break;
        default:
            break;
        }
    }
    return NULL;
}
// 把操作符插入到变量链表里面
void insert_op(Operand op)
{
    debug_printf("insert_op");
    assert(op);
    if (op->kind == OP_CONSTANT)
        return;
    if (!find_var(op))
    {
        // 不在符号表中
        local_offset -= 4;
        local_func_offset -= 4;
        Variable var = (Variable)malloc(sizeof(struct Variable_));
        assert(var);
        var->reg_no = -1;
        var->op = op;
        var->offset = local_offset;
        insert_var(var);
    }
}

// 为普通操作数分配寄存器，从t0-t7里面找
int get_reg(Operand op, bool left, FILE *code_out)
{
    debug_printf("get_reg");
    assert(op);
    int i;
    for (i = 8; i <= 15; i++)
    { // 调用者保存的t0-t7寄存器随意使用
        if (regs[i].state == FREE)
            break;
    }
    // assert(i != 16); // 使用后立即写会内存，不应该找不到空闲寄存器
    regs[i].state = BUSY;
    if (op->kind == OP_CONSTANT)
    {
        fprintf(code_out, "  li $%s, %lld\n", regs[i].name, op->u.cons_val);
    }
    else
    {
        Variable var = find_var(op);
        // if (op->kind == OP_STRUCTURE)
        //     printf("%s\n", op->u.struct_name);
        assert(var);
        var->reg_no = i;
        regs[i].var = var;
        switch (op->kind)
        {
            // 处理结构体变量的操作数
        case OP_STRUCTURE:
            debug_printf(op->u.struct_name);
            break;
        case OP_VARIABLE:
            debug_printf(op->u.var_name);
            if (!left)
                fprintf(code_out, "  lw $%s, %d($fp)\n", regs[i].name, var->offset);
            break;
        case OP_TEMP:
        case OP_ADDRESS:
            if (!left)
                fprintf(code_out, "  lw $%s, %d($fp)\n", regs[i].name, var->offset);
            break;
        case OP_ARRAY: // 因为如果是个数组的话本质上就是一个地址
            fprintf(code_out, "  addi $%s, $fp, %d\n", regs[i].name, var->offset);
            break;
        default:
            break;
        }
    }
    return i;
}
// 将寄存器保存变量存回内存
void store_reg(int reg_no, FILE *code_out)
{
    debug_printf("store_reg");
    assert(reg_no != -1);
    if (regs[reg_no].var)
    { // 寄存器存有变量，否则寄存器可能存放的为立即数
        assert(regs[reg_no].var->offset != -1);
        fprintf(code_out, "  sw $%s, %d($fp)\n", regs[reg_no].name, regs[reg_no].var->offset);
    }
    clear_reg(reg_no);
}
// 清除某个寄存器
void clear_reg(int reg_no)
{
    debug_printf("clear_reg");
    regs[reg_no].state = FREE;
    regs[reg_no].var = NULL;
}
// 处理实参和形参的寄存器部分，也就是a0-a3
void store_args(Operand op, int arg_num, FILE *code_out)
{
    assert(regs[4 + arg_num].state == FREE);
    regs[4 + arg_num].state = BUSY;
    int i = 4 + arg_num;
    if (op->kind == OP_CONSTANT)
    {
        fprintf(code_out, "  li $%s, %lld\n", regs[i].name, op->u.cons_val);
    }
    else
    {
        Variable var = find_var(op);
        // if (op->kind == OP_STRUCTURE)
        //     printf("%s\n", op->u.struct_name);
        assert(var);
        var->reg_no = i;
        regs[i].var = var;
        switch (op->kind)
        {
            // 处理结构体变量的操作数
        case OP_STRUCTURE:
            break;
        case OP_TEMP:
        case OP_VARIABLE:
        case OP_ADDRESS:
            fprintf(code_out, "  lw $%s, %d($fp)\n", regs[i].name, var->offset);
            break;
        case OP_ARRAY: // 因为如果是个数组的话本质上就是一个地址
            fprintf(code_out, "  addi $%s, $fp, %d\n", regs[i].name, var->offset);
            break;
        default:
            break;
        }
    }
}

void pre_scan_ir(InterCode ir, FILE *code_out)
{
    debug_printf("pre_scan_ir");
    if (ir == NULL || code_out == NULL)
        return;
    switch (ir->kind)
    {
    case IR_LABEL:
    case IR_GOTO:
        break;
    case IR_FUNCTION:
        insert_func(ir->u.unary_ir.op->u.func_name);
        local_func_offset = 0; // 开始计算它的栈的大小
        local_offset = 0;
        break;
    case IR_RETURN:
        insert_op(ir->u.unary_ir.op);
        insert_funcoffset(local_func_offset);
        break;
    case IR_PARAM:
        insert_op(ir->u.unary_ir.op); // 在第一遍扫描确定位置的时候只是一个普通的变量，没什么问题。
        break;
    case IR_READ:
    case IR_WRITE:
        insert_op(ir->u.unary_ir.op);
        break;
    case IR_ARG:
        // 实参应该是在call前
        insert_op(ir->u.unary_ir.op);
        break;
    case IR_DEC:
        local_offset -= (ir->u.dec.size - 4);
        local_func_offset -= (ir->u.dec.size - 4);
        insert_op(ir->u.dec.op);
        break;
    case IR_ASSIGN:
    case IR_ADDR:
    case IR_LOAD:
    case IR_STORE:
        insert_op(ir->u.binary_ir.left);
        insert_op(ir->u.binary_ir.right);
        break;
    case IR_CALL:
        insert_op(ir->u.binary_ir.left); // 把左边的变量存到链表里面
        break;
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
        insert_op(ir->u.ternary_ir.res);
        insert_op(ir->u.ternary_ir.op1);
        insert_op(ir->u.ternary_ir.op2);
        break;
    case IR_IF_GOTO:
        insert_op(ir->u.if_goto.x);
        insert_op(ir->u.if_goto.y);
        break;
    default:
        break;
    }
}
void pre_scan_all(InterCodeList head, FILE *code_out)
{
    debug_printf("pre_scan_all");
    if (head == NULL || code_out == NULL)
        return;
    InterCodeList temp = head->next;
    while (temp != head)
    {
        pre_scan_ir(temp->code, code_out);
        temp = temp->next;
    }
}

void gen_code_ir(InterCode ir, FILE *code_out)
{
    if (ir == NULL || code_out == NULL)
        return;
    switch (ir->kind)
    {
    case IR_LABEL:
        gen_ir_LABEL(ir, code_out);
        break;
    case IR_FUNCTION:
        gen_ir_FUNC(ir, code_out);
        break;
    case IR_GOTO:
        gen_ir_GOTO(ir, code_out);
        break;
    case IR_RETURN:
        gen_ir_RETURN(ir, code_out);
        break;
    case IR_ARG:
        gen_ir_ARG(ir, code_out);
        break;
    case IR_PARAM:
        gen_ir_PARAM(ir, code_out);
        break;
    case IR_READ:
        gen_ir_READ(ir, code_out);
        break;
    case IR_WRITE:
        gen_ir_WRITE(ir, code_out);
        break;
    case IR_DEC:
        gen_ir_DEC(ir, code_out);
        break;
    case IR_ASSIGN:
    case IR_ADDR:
        gen_ir_ASSIGN_ADDR(ir, code_out);
        break;
    case IR_LOAD:
    case IR_STORE:
        gen_ir_LOAD_STORE(ir, code_out);
        break;
    case IR_CALL:
        gen_ir_CALL(ir, code_out);
        break;
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
        gen_ir_ARITH(ir, code_out);
        break;
    case IR_IF_GOTO:
        gen_ir_IF_GOTO(ir, code_out);
        break;
    default:
        break;
    }
}
// 下面开始正式的生成目标代码：
void gen_code_all(InterCodeList head, FILE *code_out)
{
    arg_num = 0;
    param_num = 0;
    if (head == NULL || code_out == NULL)
        return;
    // 进行一些初始化
    init_funclist();
    init_registers();
    init_varlist();
    init_environment(code_out);
    pre_scan_all(head, code_out); // 先扫一遍，确定变量的位置
    InterCodeList temp = head->next;
    while (temp != head)
    {
        if (temp->code->kind == IR_WRITE && temp->next->code->kind == IR_ARG)
        {
            gen_code_ir(temp->code, code_out);
            temp = temp->next->next;
            continue;
        }

        gen_code_ir(temp->code, code_out);
        temp = temp->next;
    }
}

void gen_ir_LABEL(InterCode ir, FILE *code_out)
{
    assert(ir && code_out);
    assert(ir->kind == IR_LABEL);
    fprintf(code_out, "  label%d:\n", ir->u.unary_ir.op->u.label_no);
}
void gen_ir_FUNC(InterCode ir, FILE *code_out)
{
    debug_printf("gen_ir_function");
    assert(ir && code_out);
    assert(ir->kind == IR_FUNCTION);
    fprintf(code_out, "%s:\n", ir->u.unary_ir.op->u.func_name);
    fprintf(code_out, "  move $fp, $sp\n"); // 初始化帧指针
    func_list temp = find_func(ir->u.unary_ir.op->u.func_name);
    fprintf(code_out, "  addi $sp, $sp, %d\n", temp->func->sp_offset); // 获得栈空间
}
void gen_ir_RETURN(InterCode ir, FILE *code_out)
{
    debug_printf("gen_ir_return");
    assert(ir && code_out);
    assert(ir->kind == IR_RETURN);
    int reg = get_reg(ir->u.unary_ir.op, false, code_out);
    fprintf(code_out, "  move $v0, $%s\n", regs[reg].name);
    store_reg(reg, code_out);
    fprintf(code_out, "  jr $ra\n");
    clear_reg(reg);
}
void gen_ir_ARG(InterCode ir, FILE *code_out)
{ // 遇到实参，把他存到a0-a3的寄存器里面去。
    debug_printf("gen_ir_arg");
    assert(ir && code_out);
    assert(ir->kind == IR_ARG);
    store_args(ir->u.unary_ir.op, arg_num, code_out);
    arg_num++;
}
void gen_ir_CALL(InterCode ir, FILE *code_out)
{
    debug_printf("gen_ir_call");
    assert(ir && code_out);
    assert(ir->kind == IR_CALL);
    debug_printf("here1");
    // 保存帧指针和返回地址
    fprintf(code_out, "  addi $sp, $sp, -8\n");
    fprintf(code_out, "  sw $fp, 0($sp)\n");
    fprintf(code_out, "  sw $ra, 4($sp)\n");
    debug_printf("here2");
    // 把实参写入到形参地址中
    for (int i = 1; i <= arg_num; i++)
    {
        fprintf(code_out, " sw $%s,%d($sp)\n", regs[4 + arg_num - i].name, i * (-4));
        clear_reg(4 + arg_num - i); // 写完就即使清理
    }
    arg_num = 0;
    // 调用函数
    fprintf(code_out, "  jal %s\n", ir->u.binary_ir.right->u.func_name);
    debug_printf("here3");
    // 恢复栈帧指针和返回地址
    fprintf(code_out, "  move $sp, $fp\n");
    fprintf(code_out, "  lw $ra, 4($sp)\n");
    fprintf(code_out, "  lw $fp, 0($sp)\n");
    debug_printf("here4");

    fprintf(code_out, "  addi $sp, $sp, %d\n", 8);
    int reg = get_reg(ir->u.binary_ir.left, true, code_out);
    fprintf(code_out, "  move $%s, $v0\n", regs[reg].name); // 函数调用返回值赋值
    debug_printf("here5");
    store_reg(reg, code_out);
    clear_reg(reg);
}
void gen_ir_PARAM(InterCode ir, FILE *code_out)
{
    debug_printf("gen_ir_param");
}
void gen_ir_GOTO(InterCode ir, FILE *code_out)
{
    debug_printf("gen_ir_goto");
    assert(ir && code_out);
    assert(ir->kind == IR_GOTO);
    fprintf(code_out, "  j label%d\n", ir->u.unary_ir.op->u.label_no);
}

void gen_ir_READ(InterCode ir, FILE *code_out)
{
    debug_printf("gen_ir_read");
    assert(ir && code_out);
    assert(ir->kind == IR_READ);
    // 保存返回地址
    fprintf(code_out, "  addi $sp, $sp, -4\n");
    fprintf(code_out, "  sw $ra, 0($sp)\n");
    // 调用read函数
    fprintf(code_out, "  jal read\n");
    // 恢复栈指针和返回地址
    fprintf(code_out, "  lw $ra, 0($sp)\n");
    fprintf(code_out, "  addi $sp, $sp, 4\n");
    int reg = get_reg(ir->u.unary_ir.op, true, code_out);
    fprintf(code_out, "  move $%s, $v0\n", regs[reg].name);
    store_reg(reg, code_out);
}
void gen_ir_WRITE(InterCode ir, FILE *code_out)
{

    debug_printf("gen_ir_write");
    assert(ir && code_out);
    assert(ir->kind == IR_WRITE);
    int reg = get_reg(ir->u.unary_ir.op, false, code_out);
    fprintf(code_out, "  move $a0, $%s\n", regs[reg].name);
    // 保存返回地址
    fprintf(code_out, "  addi $sp, $sp, -4\n");
    fprintf(code_out, "  sw $ra, 0($sp)\n");
    // 调用write函数
    fprintf(code_out, "  jal write\n");
    // 恢复栈指针和返回地址
    fprintf(code_out, "  lw $ra, 0($sp)\n");
    fprintf(code_out, "  addi $sp, $sp, 4\n");
    clear_reg(reg);
}
void gen_ir_DEC(InterCode ir, FILE *code_out)
{
    debug_printf("gen_ir_dec");
}
void gen_ir_ASSIGN_ADDR(InterCode ir, FILE *code_out)
{
    debug_printf("gen_ir_assign_addr");
    assert(ir && code_out);
    int left = get_reg(ir->u.binary_ir.left, true, code_out);
    int right = get_reg(ir->u.binary_ir.right, false, code_out);
    assert(ir->kind == IR_ASSIGN || ir->kind == IR_ADDR);
    fprintf(code_out, "  move $%s, $%s\n", regs[left].name, regs[right].name);
    store_reg(left, code_out);
    clear_reg(right);
}
void gen_ir_LOAD_STORE(InterCode ir, FILE *code_out)
{
    debug_printf("gen_ir_load_store");
    assert(ir && code_out);
    assert(ir->kind == IR_LOAD || ir->kind == IR_STORE);
    int left, right;
    if (ir->kind == IR_LOAD)
    { // x = *y
        left = get_reg(ir->u.binary_ir.left, true, code_out);
        right = get_reg(ir->u.binary_ir.right, false, code_out);
        fprintf(code_out, "  lw $%s, 0($%s)\n", regs[left].name, regs[right].name);
        store_reg(left, code_out);
        clear_reg(right);
    }
    else if (ir->kind == IR_STORE)
    { //*x=y
        left = get_reg(ir->u.binary_ir.left, true, code_out);
        right = get_reg(ir->u.binary_ir.right, false, code_out);
        fprintf(code_out, "  lw $%s, %d($fp)\n", regs[left].name, regs[left].var->offset);
        fprintf(code_out, "  sw $%s, 0($%s)\n", regs[right].name, regs[left].name);
        // store_reg(left, code_out);
        clear_reg(left);
        clear_reg(right);
    }
}
void gen_ir_ARITH(InterCode ir, FILE *code_out)
{
    debug_printf("gen_ir_arith");
    assert(ir && code_out);
    int res = get_reg(ir->u.ternary_ir.res, true, code_out);
    int op1 = get_reg(ir->u.ternary_ir.op1, false, code_out);
    int op2 = get_reg(ir->u.ternary_ir.op2, false, code_out);
    char *arith_op = NULL;
    switch (ir->kind)
    {
    case IR_ADD:
        arith_op = "add";
        break;
    case IR_SUB:
        arith_op = "sub";
        break;
    case IR_MUL:
        arith_op = "mul";
        break;
    case IR_DIV:
        arith_op = "div";
        break;
    default:
        assert(0);
        break;
    }
    if (ir->kind == IR_DIV)
    {
        fprintf(code_out, "  %s $%s,$%s\n", arith_op, regs[op1].name, regs[op2].name);
        fprintf(code_out, "  mflo $%s\n", regs[res].name);
    }
    else
    {
        fprintf(code_out, "  %s $%s, $%s, $%s\n", arith_op, regs[res].name, regs[op1].name, regs[op2].name);
    }
    store_reg(res, code_out);
    clear_reg(op1);
    clear_reg(op2);
}
void gen_ir_IF_GOTO(InterCode ir, FILE *code_out)
{
    debug_printf("gen_ir_if_goto");
    assert(ir && code_out);
    assert(ir->kind == IR_IF_GOTO);
    int x = get_reg(ir->u.if_goto.x, false, code_out);
    int y = get_reg(ir->u.if_goto.y, false, code_out);
    char *rel_op = NULL;
    if (strcmp(ir->u.if_goto.relop, "==") == 0)
    {
        rel_op = "beq";
    }
    else if (strcmp(ir->u.if_goto.relop, "!=") == 0)
    {
        rel_op = "bne";
    }
    else if (strcmp(ir->u.if_goto.relop, ">") == 0)
    {
        rel_op = "bgt";
    }
    else if (strcmp(ir->u.if_goto.relop, "<") == 0)
    {
        rel_op = "blt";
    }
    else if (strcmp(ir->u.if_goto.relop, ">=") == 0)
    {
        rel_op = "bge";
    }
    else if (strcmp(ir->u.if_goto.relop, "<=") == 0)
    {
        rel_op = "ble";
    }
    fprintf(code_out, "  %s $%s, $%s, label%d\n", rel_op, regs[x].name, regs[y].name, ir->u.if_goto.z->u.label_no);
    clear_reg(x);
    clear_reg(y);
}
// 将采用朴素的寄存器选择方法实现（无函数实现部分）
#ifndef __OBJECT_CODE_H__
#define __OBJECT_CODE_H__
#include "intercode.h"
#include "translate.h"

typedef struct Register_ Register;
typedef struct Variable_ *Variable;
typedef struct VariableList_ *VariableList;
typedef struct FUNC_V_ *fucn_v;
typedef struct FUNC_list_ *func_list;
// 数据结构部分
typedef enum
{
    FREE,
    BUSY
} state_type;
// 定义寄存器和变量数据类型
struct Register_
{
    char *name;       // 别名
    state_type state; // 寄存器的状态
    Variable var;     // 当前存放的变量
} regs[32];
struct Variable_
{
    Operand op;
    int offset;
    int reg_no;
};
struct FUNC_V_
{
    char *func_name;
    int sp_offset;
};
struct FUNC_list_
{
    fucn_v func;
    struct FUNC_list_ *next;
};
struct VariableList_
{
    Variable var;
    struct VariableList_ *next;
};
VariableList local_varlist;
func_list local_funclist;
int local_offset, param_num, local_func_offset, arg_num, param_num;
// 函数部分
void init_environment(FILE *code_out);
void init_registers();
void init_funclist();
void insert_func(char *name);
void insert_funcoffset(int offset);
void init_varlist();
void insert_var(Variable var);

void pre_scan_all(InterCodeList head, FILE *code_out);
void pre_scan_ir(InterCode ir, FILE *code_out);

void gen_code_ir(InterCode ir, FILE *code_out);
void gen_code_all(InterCodeList head, FILE *code_out);

int get_reg(Operand op, bool left, FILE *code_out);
void store_reg(int reg_no, FILE *code_out);
void clear_reg(int reg_no);
void store_args(Operand op, int arg_num, FILE *code_out);

void gen_ir_LABEL(InterCode ir, FILE *code_out);
void gen_ir_FUNC(InterCode ir, FILE *code_out);
void gen_ir_GOTO(InterCode ir, FILE *code_out);
void gen_ir_RETURN(InterCode ir, FILE *code_out);
void gen_ir_ARG(InterCode ir, FILE *code_out);
void gen_ir_PARAM(InterCode ir, FILE *code_out);
void gen_ir_READ(InterCode ir, FILE *code_out);
void gen_ir_WRITE(InterCode ir, FILE *code_out);
void gen_ir_DEC(InterCode ir, FILE *code_out);
void gen_ir_ASSIGN_ADDR(InterCode ir, FILE *code_out);
void gen_ir_LOAD_STORE(InterCode ir, FILE *code_out);
void gen_ir_CALL(InterCode ir, FILE *code_out);
void gen_ir_ARITH(InterCode ir, FILE *code_out); // 算术运算
void gen_ir_IF_GOTO(InterCode ir, FILE *code_out);

#endif
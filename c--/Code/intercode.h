// 在这里定义中间代码的数据结构
#ifndef __INTERCODE_H__
#define __INTERCODE_H__
#include "semant.h"
typedef struct Operand_ *Operand;
typedef struct InterCode_ *InterCode;
typedef struct InterCodeList_ *InterCodeList;
typedef struct Param_s_  *Param_s;
// 定义几个记录个数的变量
int temp_number, label_number, address_number;
bool inter_error;
//记录形参的部分
struct Param_s_
{
  char* name;
  Param_s next;
};
Param_s param_head;
void insert_param(char* name);
bool is_param(char* name);
// 定义操作数数据结构
struct Operand_
{
  // 操作数类型
  enum
  {
    OP_VARIABLE = 1,
    OP_ARRAY,
    OP_FUNCTION,
    OP_LABEL,
    OP_TEMP,
    OP_CONSTANT,
    OP_STRUCTURE,
    OP_ADDRESS, // 数组或结构体的地址,d对应需要赋值类型的
  } kind;
  // 操作数对应的值
  union
  {
    char *func_name;    // 函数名称
    int label_no;       // label的编号
    int temp_no;        // temp的编号
    long long cons_val; // 常量的值
    int addr_no;        // 地址编号
    char *array_name;   // 数组名
    char *var_name;     // 变量名
    char *struct_name;  // 结构体名
  } u;
  // 记录该操作数类型的内存大小，除了数组之外（用于深拷贝部分，也就是数组变量赋值），其他的size=1
  Type type;
  int size;
  int add_pre;
};
// 定义中间代码的数据结构
struct InterCode_
{
  // 中间代码的语法类型
  enum
  {
    IR_LABEL = 1,
    IR_FUNCTION,
    IR_GOTO,
    IR_RETURN,
    IR_ARG,
    IR_PARAM,
    IR_READ,
    IR_WRITE,
    IR_DEC, // DEC x[size]
    IR_ASSIGN,
    IR_ADDR,  // 对应取地址的赋值：x:=&y
    IR_LOAD,  // 取值x:=*y
    IR_STORE, // 存值 *x:=y
    IR_CALL,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_IF_GOTO,
  } kind;
  // 每行中间代码涉及的操作数
  union
  {
    // 一元
    struct
    {
      Operand op;
    } unary_ir;
    // DEC对应的情况
    struct
    {
      Operand op;
      int size;
    } dec;

    // 二元
    struct
    {
      Operand left, right;
    } binary_ir;
    // 三元
    struct
    {
      Operand res, op1, op2;
    } ternary_ir;
    // 特殊的一个语句
    struct
    {
      Operand x, y, z;
      char *relop;
    } if_goto;
  } u;
};
// 中间代码链表部分数据结构——用环形双链表
struct InterCodeList_
{
  InterCode code;
  InterCodeList prev, next;
};
// 相关的函数
InterCodeList init_ir_list();
void add_ir(InterCodeList root, InterCode new_ir);
void delete_ir(InterCodeList ir);
void print_ir_list(InterCodeList root, FILE *out_file);
void print_ir(InterCode ir, FILE *out_file);
void print_op(Operand op, FILE *out_file);
// 一些辅助函数
InterCode gen_ir(InterCodeList intercode_head, int ir_kind, Operand op1, Operand op2, Operand op3, char *relop, int dec_size);
Operand gen_operand(int op_kind, int val, int number, char *name);

Operand new_temp();
Operand new_address();
Operand new_label();
#endif
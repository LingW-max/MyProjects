#ifndef __TYPE_H__
#define __TYPE_H__
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
// c--中的类型
typedef struct Type_ *Type;
typedef struct FieldList_ *FieldList;
typedef struct Signal_ *Signal;
struct Type_
{
    enum
    {
        BASIC,
        ARRAY,
        STRUCT_TYPE,
        FUNCTION
    } kind;
    union
    {
        // 基本类型
        int basic;
        // 数组类型信息包括元素类型和数组大小构成
        struct
        {
            Type elem;
            int dismention; // 去记录维度信息
            int size;//数组的元素个数
        } array;
        // 结构体类型信息是一个链表
        FieldList structer;
        // 函数类型的我还没想好
        struct
        {
            Type return_type;
            int argc;
            FieldList args;
        } func;
    } u;
};
struct FieldList_
{
    char *name;     // 域的名字
    Type type;      // 域的类型
    FieldList tail; // 下一个域
};
struct Signal_
{
    char *name;
    Type type;
};

Signal create_Signal(char *name, Type type); // 创建符号
bool base_type_check(Type a, Type b);        // 类型检查是否一致
bool fun_matched(Type a, Type b);            // 检查函数
#endif
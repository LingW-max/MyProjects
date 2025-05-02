// 在这里处理结构问题，也就是类型检查
#include "type.h"
// 创建符号
Signal create_Signal(char *name, Type type)
{
    Signal new_node = (Signal)malloc(sizeof(Signal));
    new_node->name = name;
    new_node->type = type;
    return new_node;
}

// 检查类型，只检查basic和structure的
bool base_type_check(Type a, Type b)
{
    if(a==NULL||b==NULL)
    return true;
    if (a->kind != b->kind)
        return false;
    switch (a->kind)
    {
    case BASIC:
        return a->u.basic == b->u.basic ? true : false;
        break;
    case ARRAY:
        printf("now we use this\n");
        return a->u.array.elem == b->u.array.elem && a->u.array.dismention == b->u.array.dismention ? true : false;
        break;
    case STRUCT_TYPE: // 因为在我的选做里对于结构体只需要名等价就可以了，对了，最后要考虑一下匿名结构体。
        return a->u.structer->name == b->u.structer->name ? true : false;
        break;
    default:
        break;
    }
}
bool fun_matched(Type a, Type b)
{
    assert(a->kind == FUNCTION && b->kind == FUNCTION);
    if (a->u.func.return_type != b->u.func.return_type || a->u.func.argc != b->u.func.argc)
        return false;
    FieldList temp_a = a->u.func.args;
    FieldList temp_b = b->u.func.args;
    for (; temp_a != NULL && temp_b != NULL; temp_a = temp_a->tail, temp_b = temp_b->tail)
    {
        if (!base_type_check(temp_a->type, temp_b->type))
        {
            return false;
        }
    }
}
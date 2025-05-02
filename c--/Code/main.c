#include <stdio.h>
#include <stdbool.h>
#include "syntax.tab.h"
#include "semant.h"
#include "translate.h"
#include "object_code.h"
#include "intercode.h"
InterCodeList intercode_head = NULL;
int stack_index = 0;
void yyrestart(FILE *input_file); // 显式声明 yyrestart 函数
extern FILE *yyin;
extern int yylineno;
extern int errorno;
extern struct Node *root;
int main(int argc, char **argv)
{
    if (argc <= 1)
        return 1;
    for (int i = 1; i < argc; i++)
    {
        FILE *f = fopen(argv[i], "r");
        if (!f)
        {
            perror(argv[i]);
            return 1;
        }
        yyrestart(f); // 使用正确的函数名
        // yylex();      // 开始词法分析
        yyparse();
        // fclose(f);
        // printTree(root, 0);
        if (errorno == 0)
            Program(root);
        trans_Program(root);
        FILE *f2 = fopen(argv[++i], "w");
        // FILE *f3 = fopen(argv[++i], "w");
        gen_code_all(intercode_head, f2);
        // print_ir_list(intercode_head, f3);
        i++;
    }
}
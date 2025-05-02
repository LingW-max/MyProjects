
%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdbool.h>
    #include "syntax.tab.h"
    #include "my_node.h"
    int yylineno; // 添加行号
    extern FILE* yyin;
    int errorno=0;

    node_type w=wrds;

    bool is_oct=0,is_hex=0;

    /*about yycolumn Definition*/
    int yycolumn=1;
    #define YY_USER_ACTION \
    yylloc.first_line=yylloc.last_line=yylineno;\
    yylloc.first_column=yycolumn;\
    yylloc.last_column=yycolumn+yyleng-1;\
    yycolumn +=yyleng;

    int _to_int(char* s);
    float _to_float(char* s);
%}
%option yylineno


digit       [0-9]
nondigit    [_a-zA-Z]
oct         0[0-7]+
oct_error   0[0-7]*[89]+[0-9]*
hex         0[xX][0-9a-fA-F]+
hex_error   0[xX][0-9a-fA-F]*[g-zG-Z]+[0-9a-zA-Z]*
float       {digit}+\.{digit}+
float_error  {digit}*\.{digit}*[eE][+-]?{digit}+
number      {oct}|{hex}|[1-9][0-9]*|0
id          {nondigit}({nondigit}|{digit})*
relop       >|<|>=|<=|==|!=
type        int|float
%%

"//"      {errorno++;printf("Error Type B at Line %d : syntax error\n",yylineno);
while(1)
{
    char c=input();
    if (c=='\n')
    break;
}
}
"/*"       {
    errorno++;printf("Error Type B at Line %d : syntax error\n",yylineno);
    while(1)
    {
        char c=input();
        if(c=='\n')
        {
        
            errorno++;printf("Error Type B at Line %d : syntax error\n",yylineno);
        }
        if(c=='*')
        {
            c=input();
            if(c=='/')
            break;
        }
        if(c==EOF)
        {
             errorno++;printf("Error Type B at Line %d : syntax error\n",yylineno);
             break;
        }
    }
}

."*/"   {errorno++;printf("Error Type B at Line %d : syntax error\n",yylineno);}
 
"struct"  {yylval.node=createNode("STRUCT",yylineno,w);return STRUCT;}
"return"  {yylval.node=createNode("RETURN",yylineno,w);return RETURN;}
"if"      {yylval.node=createNode("IF",yylineno,w);return IF;}
"else"    {yylval.node=createNode("ELSE",yylineno,w);return ELSE;}
"while"   {yylval.node=createNode("WHILE",yylineno,w);return WHILE;}
{type}    {yylval.node=createNode("TYPE",yylineno,w);yylval.node->node_str=strdup(yytext); return TYPE;}
";"       {yylval.node=createNode("SEMI",yylineno,w);return SEMI;}
","       {yylval.node=createNode("COMMA",yylineno,w);return COMMA;}
"="       {yylval.node=createNode("ASSIGNOP",yylineno,w);return ASSIGNOP;}
{relop}   {yylval.node=createNode("RELOP",yylineno,w);yylval.node->node_str=strdup(yytext); return RELOP;}
"+"       {yylval.node=createNode("PLUS",yylineno,w);return PLUS;}
"-"       {yylval.node=createNode("MINUS",yylineno,w);return MINUS;}
"*"       {yylval.node=createNode("STAR",yylineno,w);return STAR;}
"/"       {yylval.node=createNode("DIV",yylineno,w);return DIV;}
"&&"      {yylval.node=createNode("AND",yylineno,w);return AND;}
"||"      {yylval.node=createNode("OR",yylineno,w);return OR;}
"."       {yylval.node=createNode("DOT",yylineno,w);return DOT;}
"!"       {yylval.node=createNode("NOT",yylineno,w);return NOT;}
"("       {yylval.node=createNode("LP",yylineno,w);return LP;}
")"       {yylval.node=createNode("RP",yylineno,w);return RP;}
"["       {yylval.node=createNode("LB",yylineno,w);return LB;}
"]"       {yylval.node=createNode("RB",yylineno,w);return RB;}
"{"       {yylval.node=createNode("LC",yylineno,w);return LC;}
"}"       {yylval.node=createNode("RC",yylineno,w);return RC;}
{number}  {yylval.node=createNode("INT",yylineno,w);yylval.node->node_int=_to_int(yytext);return INT;}
{id}      {yylval.node=createNode("ID",yylineno,w);yylval.node->node_str=strdup(yytext); return ID;}
{float}   {yylval.node=createNode("FLOAT",yylineno,w);yylval.node->node_float=atof(yytext);return FLOAT;}
{float_error}   {yylval.node=createNode("FLOAT",yylineno,w);errorno++; printf("Error Type A at Line %d : Mysterious Characters %s \n",yylineno,yytext);return FLOAT;}
{oct_error} {yylval.node=createNode("INT",yylineno,w);errorno++; printf("Error Type A at Line %d : Mysterious Characters %s \n",yylineno,yytext);return INT;}
{hex_error} {yylval.node=createNode("INT",yylineno,w);errorno++; printf("Error Type A at Line %d : Mysterious Characters %s \n",yylineno,yytext);return INT;}
"\r\n" {yycolumn=1;}
"\n"      {yycolumn=1; }
[ \t\r]+ {}
.         {errorno++; printf("Error Type A at Line %d : Mysterious Characters %s \n",yylineno,yytext);}
%%

struct Node* createNode(char *name, int lineno,node_type type)
{
    struct Node* new_node=(struct Node*)malloc(sizeof(struct Node));
    new_node->name=name;
    new_node->line_num=lineno;
    new_node->type=type;
    new_node->child_num=0;
    for(int i=0;i<10;i++)
    {
        new_node->children[i]=NULL;
    }
    return new_node;
}
int _to_int(char* s)
{
    if(*s=='0')
    {
        if(*(s+1)=='x'||*(s+1)=='X')
        {
            is_hex=1;
        }
        else
        is_oct=1;
    }
    char* endptr;
    int rel;
    if(is_hex==0 && is_oct==0)
    return atoi(s);
    else if(is_hex)
    {
        rel=strtol(s,&endptr,16);
        is_hex=0;
        return rel;
    }
    else if(is_oct)
    {
        rel=strtol(s,&endptr,8);
        is_oct=0;
        return rel;
    }   
}


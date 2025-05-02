%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "lex.yy.c"
    #include "my_node.h"
    extern int yylineno;
    extern int yylex();
    extern FILE* yyin;
    void yyerror(char*s);
    void addChild(struct Node* parent,struct Node* child);
    void printTree(struct Node* root,int count);

%}
%union
{
    struct Node* node;
}
/*Definition tokens*/
%token <node> INT FLOAT ID SEMI COMMA ASSIGNOP RELOP
%token <node> PLUS MINUS STAR DIV AND OR DOT NOT TYPE 
%token <node> LP RP LB RB LC RC
%token <node> STRUCT RETURN IF ELSE WHILE
/*Definition types*/
%type <node>  Program ExtDefList ExtDef Specifier ExtDecList FunDec CompSt VarDec 
%type <node>  StructSpecifier OptTag DefList Tag 
%type <node>  VarList ParamDec 
%type <node>  StmtList Stmt Exp 
%type <node>  Dec DecList Def Args
/*Definition prioroty*/
// 运算优先级
%right ASSIGNOP
%left OR 
%left AND 
%left  RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT 
%left  DOT 
%left LB RB 
%left LP RP
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
/*Definition others*/
%locations
%start Program

%%
/*High-Level Definition*/
Program : ExtDefList  {$$ = createNode("Program", @$.first_line,stnc); addChild($$, $1);root=$$;}
;
ExtDefList : ExtDef ExtDefList {$$=createNode("ExtDefList",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);}
        | {$$=NULL;}
;
ExtDef : Specifier ExtDecList SEMI {$$=createNode("ExtDef",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | Specifier SEMI  {$$=createNode("ExtDef",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);}
        | Specifier FunDec CompSt {$$=createNode("ExtDef",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | Specifier FunDec SEMI {yyerror("..");}
        | error SEMI {}
     
        


;
ExtDecList : VarDec  {$$=createNode("ExtDecList",@$.first_line,stnc);addChild($$,$1);}
        | VarDec COMMA ExtDecList {$$=createNode("ExtDecList",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
;
/*Specifiers*/
Specifier : TYPE {$$=createNode("Specifier",@$.first_line,stnc);addChild($$,$1);}
        | StructSpecifier {$$=createNode("Specifier",@$.first_line,stnc);addChild($$,$1);}
;
StructSpecifier : STRUCT OptTag LC DefList RC {$$=createNode("StructSpecifier",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);addChild($$,$4);addChild($$,$5);}
        | STRUCT Tag {$$=createNode("StructSpecifier",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);}
;       
OptTag : ID {$$=createNode("OptTag",@$.first_line,stnc);addChild($$,$1);}
        | {$$=NULL;}
;
Tag : ID {$$=createNode("Tag",@$.first_line,stnc);addChild($$,$1);}
;
/*Declarators*/
VarDec : ID {$$=createNode("VarDec",@$.first_line,stnc);addChild($$,$1);}
        | VarDec LB INT RB {$$=createNode("VarDec",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);addChild($$,$4);}
        | VarDec LB error RB {}
;
FunDec : ID LP VarList RP  {$$=createNode("FunDec",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);addChild($$,$4);}
        | ID LP RP {$$=createNode("FunDec",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | ID LP error RP {}
;
VarList : ParamDec COMMA VarList {$$=createNode("VarList",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | ParamDec {$$=createNode("VarList",@$.first_line,stnc);addChild($$,$1);} 
;
ParamDec : Specifier VarDec  {$$=createNode("ParamDec",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);}
;
/*Statements*/
CompSt : LC DefList StmtList RC {$$=createNode("CompSt",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);addChild($$,$4);}
        | error RC {}
;
StmtList : Stmt StmtList {$$=createNode("StmtList",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);}
        | {$$=NULL;}
;
Stmt : Exp SEMI {$$=createNode("Stmt",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);}
        | CompSt {$$=createNode("Stmt",@$.first_line,stnc);addChild($$,$1);}
        | RETURN Exp SEMI {$$=createNode("Stmt",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$=createNode("Stmt",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);addChild($$,$4);addChild($$,$5);}
        | IF LP Exp RP Stmt ELSE Stmt  {$$=createNode("Stmt",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);addChild($$,$4);addChild($$,$5);addChild($$,$6);addChild($$,$7);}
        | WHILE LP Exp RP Stmt {$$=createNode("Stmt",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);addChild($$,$4);addChild($$,$5);}
        | IF LP error RP Stmt  {}
        | IF LP error RP Stmt ELSE Stmt {}
        | WHILE LP error RP Stmt {}
          | error SEMI {}
;
/*Local Definition*/
DefList : Def DefList {$$=createNode("DefList",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);}
        | {$$=NULL;}
;
Def : Specifier DecList SEMI {$$=createNode("Def",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | Specifier error SEMI {}
        | Specifier DecList error {}
;     
DecList : Dec  {$$=createNode("DecList",@$.first_line,stnc);addChild($$,$1);}
        | Dec COMMA DecList {$$=createNode("DecList",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
;
Dec : VarDec {$$=createNode("Dec",@$.first_line,stnc);addChild($$,$1);}
        | VarDec ASSIGNOP Exp {$$=createNode("Dec",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
;
/*Expressions*/
Exp : Exp ASSIGNOP Exp {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | Exp AND Exp {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | Exp OR Exp  {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | Exp RELOP Exp {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | Exp PLUS Exp  {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | Exp MINUS Exp {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | Exp STAR Exp  {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | Exp DIV Exp   {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | LP Exp RP {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | MINUS Exp {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);}
        | NOT Exp {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);}
        | ID LP Args RP {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);addChild($$,$4);}
        | ID LP RP {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | Exp LB Exp RB {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);addChild($$,$4);}
        | Exp DOT ID {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | ID {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);}
        | INT {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);}
        | FLOAT {$$=createNode("Exp",@$.first_line,stnc);addChild($$,$1);}
  

;
Args : Exp COMMA Args {$$=createNode("Args",@$.first_line,stnc);addChild($$,$1);addChild($$,$2);addChild($$,$3);}
        | Exp {$$=createNode("Args",@$.first_line,stnc);addChild($$,$1);}
;
%%
void yyerror(char* s){
    errorno++;
    fprintf(stdout,"Error Type B at Line %d : %s\n",yylineno,s);
}

void addChild(struct Node* parent,struct Node* child){
        parent->children[parent->child_num]=child;
        parent->child_num+=1;
}

void printTree(struct Node* root,int count){
        if (errorno>0) return;
        if(root==NULL)
        {
            return ;
        }
        for(int i=0;i<count;i++)
        {
                printf(" ");
        }
        if(root->type==stnc)
        {
        fprintf(stdout,"%s (%d)\n",root->name,root->line_num);
        }
        else
        {
                if(strcmp(root->name,"ID")==0)
                {
                        fprintf(stdout,"%s: %s\n",root->name,root->node_str);
                }
                else if(strcmp(root->name,"TYPE")==0)
                {
                        fprintf(stdout,"%s: %s\n",root->name,root->node_str);
                }
                else if(strcmp(root->name,"INT")==0)
                {
                        fprintf(stdout,"%s: %d\n",root->name,root->node_int);
                }
                else if(strcmp(root->name,"FLOAT")==0)
                {
                        fprintf(stdout,"%s: %f\n",root->name,root->node_float);
                }
                else
                {
                        fprintf(stdout,"%s\n",root->name);
                }
        }
        for(int k=0;k<root->child_num;k++){
                printTree(root->children[k],count+2);
}
}

int yywrap(){
    return 1;
    }   


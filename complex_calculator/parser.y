%{

#include <iostream>
#include <math.h>

extern int yylex();
extern void yyerror(const char* s, ...);

%}

%define parse.trace

/* A complex number (real + imaginary*i)
 */
%code requires {
    struct Complex {
        float real;
        float imaginary;
    };
}

/* yylval == %union
 * union informs the different ways we can store data
 */
%union {
    float value;
    struct Complex complex;
}

/* token defines our terminal symbols (tokens).
 */
%token <value> T_REAL T_IMAGINARY
%token T_PLUS T_MINUS T_NL

/* type defines the type of our nonterminal symbols.
 * Types should match the names used in the union.
 * Example: %type<node> expr
 */
%type <value> program lines line
%type <complex> expr

/* Operator precedence for mathematical operators
 * The latest it is listed, the highest the precedence
 * left, right, nonassoc
 */
%left T_PLUS T_MINUS

/* Starting rule 
 */
%start program

%%

program: /*use ctrl+d to stop*/
    lines /*$$ = $1 when nothing is said*/
    ;

lines: 
    line /*$$ = $1 when nothing is said*/
    | lines line
    ;

line: 
    T_NL { $$ = 0 /*NULL*/; } /*nothing here to be used */
    | expr T_NL { std::cout << "Res: " << $1.real << " + " << $1.imaginary << "i" << std::endl; }
    ;

expr:
    T_REAL T_PLUS T_IMAGINARY { $$.real = $1; $$.imaginary = $3; } /*Could write nothing here since we are just copying*/
/*    | expr T_PLUS expr { $$ = $1 + $3; std::cout << $1 << " + " << $3 << std::endl; }
    | expr T_MINUS expr { $$ = $1 - $3; std::cout << $1 << " - " << $3 << std::endl; }*/
    ;

%%



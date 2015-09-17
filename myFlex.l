%{
/*
myFlex.l
	This file defines the lexical anaylyzer
*/
#include "com.h"
#include "Print.h"
#include "ANTNode.h"
#include "y.tab.h"//Yacc's output head file name
#include <string>
#include <cstdio>
#include <cstdlib>
using namespace std;

int getValue(char *s);
extern int yylineno;
%}
digit [0-9]
digits {digit}+
hexletter [a-fA-F]
letter [a-zA-Z]
letters {letter}+
whiteSpace [\r\t ]+

%%

"struct" {  /*cout << yytext;*/ yylval.value = 0; return STRUCT;}
"int" {  /*cout << yytext;*/ yylval.value = 0; return TYPE;}
"return" {  /*cout << yytext;*/ yylval.value = 0; return RETURN;}
"if" {  /*cout << yytext;*/ yylval.value = 0; return IF;}
"else" {  /*cout << yytext;*/ yylval.value = 0; return ELSE;}
"break" {  /*cout << yytext;*/ yylval.value = 0; return BREAK;}
"continue" {  /*cout << yytext;*/ yylval.value = 0; return CONT;}
"for" {  /*cout << yytext;*/ yylval.value = 0; return FOR;}
"read" {  /*cout << yytext;*/ yylval.value = 0; return READ;}
"write" {  /*cout << yytext;*/ yylval.value = 0; return WRITE;}

"-"  {  /*cout << yytext;*/ yylval.value = 0; return SUB;}
";"  {  /*cout << yytext;*/ yylval.value = 0; return SEMI;}
","  {  /*cout << yytext;*/ yylval.value = 0; return COMMA;}
"."  {  /*cout << yytext;*/ yylval.value = 0; return DOT;}
"!"  {  /*cout << yytext;*/ yylval.value = 0; return NOT;}
"~"  {  /*cout << yytext;*/ yylval.value = 0; return BNOT;}
"*"  {  /*cout << yytext;*/ yylval.value = 0; return MUL;}
"/"  {  /*cout << yytext;*/ yylval.value = 0; return DIV;}
"%"  {  /*cout << yytext;*/ yylval.value = 0; return MOD;}
"+"  {  /*cout << yytext;*/ yylval.value = 0; return ADD;}
">"  {  /*cout << yytext;*/ yylval.value = 0; return GT;}
"<"  {  /*cout << yytext;*/ yylval.value = 0; return LT;}
"&"  {  /*cout << yytext;*/ yylval.value = 0; return BAND;}
"^"  {  /*cout << yytext;*/ yylval.value = 0; return BXOR;}
"|"  {  /*cout << yytext;*/ yylval.value = 0; return BOR;}
"="  {  /*cout << yytext;*/ yylval.value = 0; return AN;}
"["  {  /*cout << yytext;*/ yylval.value = 0; return LB;}
"]"  {  /*cout << yytext;*/ yylval.value = 0; return RB;}
"("  {  /*cout << yytext;*/ yylval.value = 0; return LP;}
")"  {  /*cout << yytext;*/ yylval.value = 0; return RP;}
"{"  {  /*cout << yytext;*/ yylval.value = 0; return LC;}
"}"  {  /*cout << yytext;*/ yylval.value = 0; return RC;}
"<<" {  /*cout << yytext;*/ yylval.value = 0; return SL;}
">>" {  /*cout << yytext;*/ yylval.value = 0; return SR;}
">=" {  /*cout << yytext;*/ yylval.value = 0; return GE;}
"<=" {  /*cout << yytext;*/ yylval.value = 0; return LE;}
"==" {  /*cout << yytext;*/ yylval.value = 0; return EQ;}
"!=" {  /*cout << yytext;*/ yylval.value = 0; return NE;}
"&&" {  /*cout << yytext;*/ yylval.value = 0; return AND;}
"||" {  /*cout << yytext;*/ yylval.value = 0; return OR;}
"+=" {  /*cout << yytext;*/ yylval.value = 0; return ADDA;}
"-=" {  /*cout << yytext;*/ yylval.value = 0; return SUBA;}
"*=" {  /*cout << yytext;*/ yylval.value = 0; return MULA;}
"/=" {  /*cout << yytext;*/ yylval.value = 0; return DIVA;}
"&=" {  /*cout << yytext;*/ yylval.value = 0; return BANDA;}
"^=" {  /*cout << yytext;*/ yylval.value = 0; return BXORA;}
"|=" {  /*cout << yytext;*/ yylval.value = 0; return BORA;}
"<<=" {  /*cout << yytext;*/ yylval.value = 0; return SLA;}
">>=" {  /*cout << yytext;*/ yylval.value = 0; return SRA;}
"++" {  /*cout << yytext;*/ yylval.value = 0; return UADD;}
"--" {  /*cout << yytext;*/ yylval.value = 0; return USUB;}

{digits} { /*cout << yytext;*/ yylval.value = getValue(yytext);return INTEGER;}
((0x)|(0X))({digit}|{hexletter})* {/*cout << yytext;*/ yylval.value = getValue(yytext);return INTEGER;}
({letter}|"_")({letter}|"_"|{digit})* { /*cout << yytext;*/ yylval.text = new std::string(yytext);return ID;}
{whiteSpace} {;}
\n {yylineno ++;}
. {printErrorMessage(yylineno,"Lexical Error","Undefined character");
}

%%
int yywrap(void)
{
	return 1;
}
/*initInput: Redirecting the input file of the lex.*/
int initInput()
{
	FILE *temp;
	/*Open the input file*/

	if((temp = fopen(inputFileName.c_str(),"r")) == NULL )
	{
		cerr << "Error: Can not open input file: " << inputFileName << " \n"; 
		return ERROR;
	}
	yyin = temp;
	return OK;
}

int getValue(char *s)
{
    int value;
	char *stopPoint;
	value = strtol(s,&stopPoint,0);
	if( errno == ERANGE ) 
	{
		cerr << "Warning[Line" << yylineno << "]:The constant number \"" << s 
			<< "\" overflows. It has been set to INTMAX\n";
		errno = 0;
	}
    return value;
}

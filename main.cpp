/*
main.cpp
    This file defines the entry of the program and some global variables.
*/
#include "Print.h"
#include "ANTNode.h"
#include <iostream>
#include <vector>
#include <string>
using namespace std;
//config
string datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S128";
string triple = "i386-pc-linux-gnu";
//parse
extern int yylineno;
extern int yyparse();
extern int initInput();
//main.cpp
ofstream llvmout;
//ostream& llvmout = cout;
//ifstream llvmin;
string inputFileName;
string outputFileName;

int main(int argc, char *argv[])
{
    yylineno = 1;	
    if(argc != 3)
    {
    	cerr << "Incorrect arguments count\n";
    	return 1;
    }
    inputFileName = argv[1];
    outputFileName = argv[2];
    llvmout.open(argv[2]);
    if(!llvmout)
    {
    	cerr << "Failed in opening iuput file\n";
    	return 1;
    }
	if (initInput() == ERROR ) 
	{
		cerr << "Failed in opening input file\n";
		return 1;
	}
	yyparse();

	if(parseOK == OK && errorCount == 0)
	{
		cout << "\nStart generating all code...\n\n";
		genAllCode();
	}
	else
		cout << "Found " << errorCount << " error.\nFailed\n";
}

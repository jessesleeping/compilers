/*
version_2
com.h
	This file defines some global macro and variables
*/
#ifndef COM_H
#define COM_H

#include <iostream>
#include <fstream>
using namespace std;
#define OK 1
#define ERROR -1
//Error type
#define NULLEXP ERROR
#define UNINIT ERROR-1
#define NOTINT ERROR-2
#define NOTFOR ERROR-3
#define NOTLEFT ERROR-4
#define NOTSTRUCT ERROR-5
#define OUTOFBORDER ERROR-6
#define NOTARRAY ERROR-7
#define NOTMATCH ERROR-8
#define ANDORBUG ERROR-9
#define TYPEERROR ERROR-10
#define NOTMEMBER ERROR-11
#define NORETURN ERROR-12

//config
extern string datalayout;
extern string triple;
//parse
extern int parseOK;
extern int yylineno;
extern int errorCount;
extern string yyString;
//main.cpp
extern ofstream llvmout;
//extern ostream &llvmout;
//extern ifstream llvmin;
extern string inputFileName;
extern string outputFileName;

#endif

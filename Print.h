/*
Print.h
	This file declares the functions used in printing llvm code
*/
#ifndef PRINT_H
#define PRINT_H

#include "com.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <list>
using namespace std;

#define LOCALVAR false
#define GLOBALVAR true
extern string retLabel;
extern int regNum;
extern int label;
extern int errorCount;
extern list<string>LabelList;
extern string ReadFunc;
extern string WriteFunc;
extern string InitFunc;
//label and temp
string getTempReg();
void resetRegAndLabel();
string *addLabel( string s );/*Return value has label "Ret"*/
void printLabel( string *labelP );
//Definition
void printGloIntDef(string addr );
void printStrVarDef(string addr, string type, bool glo = false );
void printIntArrayDef( string addr, vector<int> *lP, bool glo = false );
void printStrArrayDef( string addr, vector<int> *lP, string type,  bool glo = false );
void printLocalIntDef(string addr);
//IO function
void printRead(string dest);
void printWrite(string source);
void printWrite(int i);
//Load&Store
void printLoad( string reg, string addr, string t = "i32");
void printStore( string addr, string reg, string t = "i32" );
void printStore( string addr, int i, string t = "i32" );
//getelementptr
void printGetElePtr_iA( string left, string addr, vector<int> *lP, int i );
void printGetElePtr_iA( string left, string addr, vector<int> *lP, string i );
void printGetElePtr_sA(string left, string addr, string type, vector<int> *lP, int i );
void printGetElePtr_sA(string left, string addr, string type, vector<int> *lP, string i );
void printGetElePtr_st( string left, string addr, string type, int i );
//bitcast&memcpy
void printBitCast(string dest, string source, string type);
void printMemCpy(string dest, string source, int l);
//operation
void printOp(string dest, string sL, string sR, int op);
void printOp(string dest, string sL, int i, int op);
void printOp(string dest, int i, string sR, int op);
void printXOr( string dest, string sL, string sR );/*i1*/
//icmp
void printICmp( string dest, string sL, string sR, int cond );
void printICmp( string dest, int i, string sR, int cond );
void printICmp( string dest, string sL, int i, int cond );
//zext
void printZext( string dest, string s );
//branch
void printCondBr( string cond, string *trueP, string *falseP);
void printBr( string *labelP );
//Others
void printHeadOfMain();
//void printTailOfProgame();
void printReadDef();
void printWriteDef();
void printMemCpyDef();
//Error message
void printErrorMessage(int lineno,string position, string messages);
#endif
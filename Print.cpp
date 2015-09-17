/*
Print.cpp
	This file defines the function used to print code in llvm format
*/
#include "Print.h"
#include "ANTNode.h"
#include "y.tab.h"
//#include <fstream>
//#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <list>
using namespace std;

string ReadFunc = "readFunc";
string WriteFunc = "writeFunc";
string InitFunc = "initFunc";
string retLabel = "Ret";
int regNum = 1;
int label = 0;
int errorCount = 0;
list<string>LabelList;
/************************************************************************/
/*                             label&temp                               */
/************************************************************************/
string getTempReg()
{
	char buffer[15];
	sprintf(buffer,".%d",++regNum);//For convenience, we use %.num as temp instead of %num
	return "%"+string(buffer);
}
void resetRegAndLabel()
{
	regNum = 0;
	label = 0;
}
string *addLabel( string s )
{
	char buffer[15];
	sprintf(buffer,".%d",++label);
	LabelList.push_back(s+buffer);
	return &LabelList.back();
}
void printLabel( string *labelP )
{
	llvmout << '\n' << *labelP << ":\n";
}
/************************************************************************/
/*                            Print Definition                          */
/************************************************************************/
void printGloIntDef(string addr )
{
	llvmout << addr << " = global i32 0, align 4\n";
}

void printLocalIntDef(string addr)
{
	llvmout << "  " << addr << " = alloca i32, align 4\n"; 
}

void printStrVarDef(string addr, string type, bool glo )
{
	if( glo == true )
		llvmout << addr << " = global " << type << " zeroinitializer, align 4\n";
	else
		llvmout << "  " << addr << " = alloca" << type << ", align 4\n";
}

void printIntArrayDef( string addr, vector<int> *lP, bool glo )
{
	//cout << "## In printIntArrayDef()\n";
/*	if( glo == true )
		llvmout << addr << " = global [" << l 
		<< " x i32 ] zeroinitializer, align 4\n";
	else
		llvmout << "  " << addr << " = alloca [" << l
		<< " x i32 ], align 4\n";*/
	if( glo == true ) llvmout << addr << " = global ";
	else llvmout << addr << " = alloca ";
	for( int j = 0; j < lP->size(); ++j )
		llvmout << "[ " << lP->at(j) << " x ";
	llvmout << "i32";
	for( int j = 0; j < lP->size(); ++j )
		llvmout << " ]";
	if( glo == true ) llvmout << " zeroinitializer, align 4\n";
	else llvmout << ", align 4\n";
}

void printStrArrayDef( string addr, vector<int> *lP, string type,  bool glo )
{
	/*if( glo == true )
		llvmout << addr << " = global [" << l
		<< " x " << type << " ] zeroinitializer, align 4\n";
	else
		llvmout << "  " << addr << " = alloca [" << l
		<< " x " << type << " ], align 4\n";*/
	if( glo == true ) llvmout << addr << " = global ";
	else llvmout << addr << " = alloca ";
	for( int j = 0; j < lP->size(); ++j )
		llvmout << "[ " << lP->at(j) << " x ";
	llvmout << type;
	for( int j = 0; j < lP->size(); ++j )
		llvmout << " ]";
	if( glo == true ) llvmout << " zeroinitializer, align 4\n";
	else llvmout << ", align 4\n";
}

/************************************************************************/
/*                        load & store                                  */
/************************************************************************/
void printLoad( string reg, string addr, string t)
{
	llvmout << "  " << reg << " = load " << t << "* " << addr
		<< ", align 4\n";
}
void printStore( string addr, string reg, string t )
{
	llvmout << "  " << "store " << t << ' ' << reg
		<< ", " << t << "* " << addr
		<< ", align 4\n\n";
}
void printStore( string addr, int i, string t )
{
	llvmout << "  " << "store " << t << ' ' << i
		<< ", " << t << "* " << addr
		<< ", align 4\n";
}
/************************************************************************/
/*                        getelementptr                                 */
/************************************************************************/
//$$  Need modify
void printGetElePtr_iA( string left, string addr, vector<int> *lP, int i )
{
	llvmout << "  " << left << " = getelementptr inbounds ";
	for( int j = 0; j < lP->size(); ++j )
	{
		if( lP->at(j) == -1 ) continue;
		llvmout << "[ " << lP->at(j) << " x ";
	}
	llvmout << "i32";
	for( int j = 0; j < lP->size(); ++j )
	{
		if( lP->at(j) == -1 ) continue;
		llvmout << " ]";
	}
	llvmout << "* " << addr << ",";
	if( !lP->empty() && lP->front() != -1 )
		llvmout <<  " i32 0,";
	llvmout << " i32 " << i << '\n';
}
void printGetElePtr_iA( string left, string addr, vector<int> *lP, string i )
{
	llvmout << "  " << left << " = getelementptr inbounds ";
	for( int j = 0; j < lP->size(); ++j )
	{
		if( lP->at(j) == -1 ) continue;
		llvmout << "[ " << lP->at(j) << " x ";
	}
	llvmout << "i32";
	for( int j = 0; j < lP->size(); ++j )
	{
		if( lP->at(j) == -1 ) continue;
		llvmout << " ]";
	}
	llvmout << "* " << addr << ",";
	if( !lP->empty() && lP->front() != -1 )
		llvmout <<  " i32 0,";
	llvmout << " i32 " << i << '\n';
}
void printGetElePtr_sA(string left, string addr, string type, vector<int> *lP, int i )
{
	llvmout << "  " << left << " = getelementptr inbounds ";
	for( int j = 0; j < lP->size(); ++j )
	{
		if( lP->at(j) == -1 ) continue;
		llvmout << "[ " << lP->at(j) << " x ";
	}
	llvmout << type;
	for( int j = 0; j < lP->size(); ++j )
	{
		if( lP->at(j) == -1 ) continue;
		llvmout << " ]";
	}
	llvmout << "* " << addr << ", i32 0, i32 " << i << '\n';
}
void printGetElePtr_sA(string left, string addr, string type, vector<int> *lP, string i )
{
	llvmout << "  " << left << " = getelementptr inbounds ";
	for( int j = 0; j < lP->size(); ++j )
	{
		if( lP->at(j) == -1 ) continue;
		llvmout << "[ " << lP->at(j) << " x ";
	}
	llvmout << type;
	for( int j = 0; j < lP->size(); ++j )
	{
		if( lP->at(j) == -1 ) continue;
		llvmout << " ]";
	}
	llvmout << "* " << addr << ", i32 0, i32 " << i << '\n';
}
void printGetElePtr_st( string left, string addr, string type, int i )
{
	llvmout << "  " << left << " = getelementptr inbounds "
		<< type << "* " 
		<< addr << ", i32 0, i32 " << i << '\n';
}
/************************************************************************/
/*                        bitcast&memcpy                                */
/************************************************************************/
void printBitCast(string dest, string source, string type)
{
	llvmout << "  " << dest << " = bitcast " << type << "* " << source << " to i8*\n";
}
void printMemCpy(string dest, string source, int l)
{
	llvmout << "call void @llvm.memcpy.p0i8.p0i8.i32( i8* " << dest
		<< ", i8* " << source << ", i32 " << l*4 << ", i32 4, i1 false)\n";
}
/************************************************************************/
/*                        operation                                     */
/************************************************************************/
const char* getOp(int op)
{
	switch(op)
	{
	case ADD: return "add nsw";
	case SUB: return "sub nsw";
	case MUL: return "mul nsw";
	case DIV: return "sdiv";
	case MOD: return "srem";
	case SL: return "shl";
	case SR: return "ashr";
	case BAND: return "and";
	case BXOR: return "xor";
	case BOR: return "or";
	default: 
		printErrorMessage(-1,"Operator","Undefined operator");
		return "ERROR";
	}
}
const char* getReOp( int re )
{
	switch(re)
	{
	case EQ:return "eq";
	case NE:return "ne";
	case GT:return "sgt";
	case GE:return "sge";
	case LT:return "slt";
	case LE:return "sle";
	default:
	printErrorMessage(-1,"Operator","Undefined comparasion condition");
	return "ERROR";
	}
}
void printOp(string dest, string sL, string sR, int op )
{
	llvmout << "  " << dest << " = " << getOp(op) << " i32 " 
		<< sL << ", " << sR << '\n';
}
void printOp(string dest, int i, string sR, int op )
{
	llvmout << "  " << dest << " = " << getOp(op) << " i32 " 
		<< i << ", " << sR << '\n';
}
void printOp(string dest, string sL, int i, int op )
{
	llvmout << "  " << dest << " = " << getOp(op) << " i32 " 
		<< sL << ", " << i << '\n';
}
/************************************************************************/
/*                        icmp&xor&zext                                 */
/************************************************************************/
void printICmp( string dest, string sL, string sR, int cond )
{
	llvmout << "  " << dest << " = icmp " << getReOp(cond) 
		<< " i32 " << sL << ", " << sR << "\n";
}
void printICmp( string dest, string sL, int i, int cond )
{
	llvmout << "  " << dest << " = icmp " << getReOp(cond)
		<< " i32 " << sL << ", " << i << "\n";
}
void printICmp( string dest, int i, string sR, int cond )
{
	llvmout << "  " << dest << " = icmp " << getReOp(cond)
		<< " i32 " << i << ", " << sR << "\n";
}
void printXOr( string dest, string sL, string sR )/*i1*/
{
	llvmout << "  " << dest << " = xor i1 " << sL 
		<< ", " << sR << "\n";
}
void printZext( string dest, string s )
{
	llvmout << "  " << dest << " = zext i1 " << s << " to i32\n";
}
/************************************************************************/
/*                         branch                                       */
/************************************************************************/
void printCondBr( string cond, string *trueP, string *falseP)
{
	llvmout << "  " << "br i1 " << cond << ", label %" 
		<< *trueP << ", label %" << *falseP << '\n';
	//Handle the basic blocks number
	//getTempReg();
}
void printBr( string *labelP )
{
	llvmout << "  " << "br label %" << *labelP << '\n';
	//Handle the basic blocks number
	//getTempReg();
}
/************************************************************************/
/*                          Read&Write                                  */
/************************************************************************/
void printReadDef()
{
	llvmout << "@.strRead = private unnamed_addr constant [3 x i8] "
		<< "c\"%d\\00\", align 1\n";
	llvmout << "declare i32 @scanf(i8*, ...)\n";
	llvmout << "define i32 @" << ReadFunc << "() {\n";
	llvmout << "  %a = alloca i32, align 4\n";
	llvmout << "  %1 = call i32 (i8*, ...)* @scanf(i8* getelementptr inbounds "
		    << "([3 x i8]* @.strRead, i32 0, i32 0), i32* %a)\n";
	llvmout << "  %2 = load i32* %a, align 4\n";
	llvmout << "  ret i32 %2\n";
	llvmout << "}\n\n";
}
void printWriteDef()
{
	llvmout << "@.strWrite = private unnamed_addr constant [4 x i8] "
		    << "c\"%d\\0A\\00\", align 1\n";
	llvmout << "declare i32 @printf(i8*, ...)\n";
	llvmout << "define void @" << WriteFunc << "(i32 %a) {\n";
	llvmout << "  %1 = alloca i32, align 4\n";
    llvmout	<< "  store i32 %a, i32* %1, align 4\n";
	llvmout << "  %2 = load i32* %1, align 4\n";
	llvmout << "  %3 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds " 
	        << "([4 x i8]* @.strWrite, i32 0, i32 0), i32 %2)\n";
	llvmout << "  ret void\n";
	llvmout << "}\n\n";
}
void printRead(string dest)
{
	llvmout << "  " << dest << " = call i32 @" << ReadFunc << "()\n";
}
void printWrite(string source)
{
	llvmout << "  " << "call void @"<< WriteFunc << "(i32 " << source << ")\n";
}
void printWrite(int i)
{
	llvmout << "  " << "call void @"<< WriteFunc << "(i32 " << i << ")\n";
}

void printHeadOfMain()
{
	llvmout << "  call void @" << InitFunc << "()\n\n";
}

void printErrorMessage(int lineno,string postion, string messages)
{
	cerr << "Error[" << (++errorCount) << "][line " << lineno << "] in " << postion << " : " << messages << '\n';
}

void printMemCpyDef()
{
	llvmout << "\ndeclare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture, i8* nocapture, i32, i32, i1) nounwind\n";
}

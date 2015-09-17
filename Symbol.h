/*
version_2
Symbol.h:
	This file defines the symbol table class and the symbols used in symboltable.
*/
#ifndef SYMBOL_H
#define SYMBOL_H

#include <iostream>
#include <string>
#include <vector>
#include <list>
using namespace std;

#define NOTFOUND -1
#define GLOBALENABLE true
#define LOCALONLY false

class Symbol;
class IntSym;
class StSym;
class ArraySym;
class StArray;
class IntArray;
class StTypeSym;
class FuncSym;
class SymbolTable;
typedef vector<Symbol*> Environment;
enum SymbolType { INSTANT, INTSYM, STRUCTSYM, STTYPE, INTARRAY, STARRAY, FUNCSYM, NONE };

//SPEC & STSPEC
struct specNode 
{
	SymbolType type;
	StTypeSym *stType;
};
//VAR
struct varNode
{
	string idName;
	vector<int> indexList;
};

class Symbol
{
protected:
	string name;
	string memAdd;
	SymbolType type;

	static int symCount;
	int idNum;

	string regTag;
	bool glo;
	bool init;
public:
	Symbol( string nm = "", SymbolType ty = NONE);

	static int getSymCount() { return symCount;}
	string getName(){ return name;}
	SymbolType getType(){ return type;}
	string getReg(){ return regTag;}
	bool noReg(){ return regTag.empty();}
	void setReg(string r){ regTag = r;}
	string getAddress(){ return memAdd;}
	void setAddress(string addr){ memAdd = addr;}
	bool noAddr(){ return memAdd.empty();}
	bool isGlobal(){return glo;}
	void setGlobal();
	bool hasInited(){ return init;}
	void setInit(){ init = true;}
};
class IntSym : public Symbol
{
private:
public:
	IntSym(string nm = "");
};

class StTypeSym : public Symbol
{
private:
	vector<Symbol *> members;
public:
	StTypeSym( string nm, vector<Symbol*> &m );
	int getMemCount() { return members.size();}
	int findMemIndex(string s);
	int decCodeGen();
};

class FuncSym : public Symbol
{
private:
	vector<Symbol*> argList;
public:
	FuncSym( string nm, vector<Symbol*> &arg );
	int getArgCount(){ return argList.size();}
	Symbol *getArgSym( string s );
	Symbol *getArgSym( int i ){ return argList.at(i);}
};

class StSym : public Symbol
{
private:
	StTypeSym *stType;
public:
	StSym( string nm = "", StTypeSym*stt = NULL );
	StTypeSym *getStType(){ return stType;}
	int getMemCount(){ return stType->getMemCount();}
	string getStTypeAddr(){ return stType->getAddress();}
};

class ArraySym : public Symbol
{
protected:
	vector<int> lengthList;
public:
	ArraySym( string nm = "", SymbolType ty = NONE, vector<int> *l = NULL );
	int getDim(){ return lengthList.size();}
	int getLen()
	{ 
		if(!lengthList.empty()) return lengthList.back();
		return 0;
	}
	void setLen(int i){lengthList.back() = i;}
	vector<int>* getLenListP(){ return &lengthList;}
	void makePointer(){lengthList.front() = -1;}
	bool isPointer(){ return (lengthList.front() == -1);}
};

class IntArray :public ArraySym
{
public:
	IntArray( string nm = "", vector<int> *l = NULL)
		:ArraySym(nm,INTARRAY,l){}
};

class StArray : public ArraySym
{
private:
	StTypeSym *stType;
public:
	StArray( string nm = "", vector<int> *l = NULL, StTypeSym *stt = NULL)
		:ArraySym(nm, STARRAY,l),stType(stt){}

	StTypeSym *getStType(){ return stType;}
	string getStTypeAddr(){ return stType->getAddress();}
};

class SymbolTable
{
private:
	Symbol instantNum;
	list<IntSym> intList;
	list<StSym> stList;
	list<IntArray> intArrayList;
	list<StArray> stArrayList;
	list<FuncSym> funcList;
	list<StTypeSym> stTypeList;
	vector<Environment> envStack;

	IntSym *addInt(string nm);
	StSym *addSt( string nm, StTypeSym *stt );
	IntArray *addIntArray(string nm, vector<int> *i);
	StArray *addStArray(string nm, vector<int> *i, StTypeSym *stt);
public:
	SymbolTable();
	Symbol *addSym(specNode *spec, varNode *var);
	StTypeSym *addStType(string nm, vector<Symbol*> &m);
	FuncSym *addFunc(string nm, vector<Symbol*> &arg);
	IntSym *addTempInt(string nm);
	StSym *addTempSt( string nm, StTypeSym *stt );
	IntArray *addTempIA( string nm, vector<int> *i );
	StArray *addTempSA( string nm, vector<int> *i, StTypeSym *stt);
	Symbol *addInstant(){ return &instantNum;}

	void newEnv();
	bool leaveEnv();

	Symbol *findSym( string nm, bool backCheck);
	FuncSym *findFuncSym( string nm );
	//Environment *getGlobalEnv(){ return &envStack.front();}
	int allStDecGen();
};
bool sameType( Symbol *s, Symbol *t );
extern SymbolTable SBT;
#endif
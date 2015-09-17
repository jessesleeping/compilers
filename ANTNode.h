/*
version_2
ANTNode.h
	This file defines the classes used in the annotating parsing tree.
*/
#ifndef NODE_H
#define NODE_H

#include "Symbol.h"
#include "com.h"
using namespace std;

//PROGRAME  vector<Definition*>

//EXTDEFS vector<Definition*>
//EXTDEF
class Definition;
class VarDef; //DEF
class ExtVarDef; //EXTDEF for global variable
class ExtFuncDef; //EXRDEF for function
//SPEC & STSPEC
struct specNode;
//VAR
struct varNode;
//STMTBLOCK
class StmtBlock;
//STMT
class Stmt;
class RetStmt;
class IfStmt;
//EStmt Stmt
class ForStmt;
class BreakStmt;
class ContStmt;
class ReadStmt;
class WriteStmt;
class ExprStmt;
//PARA  Symbol *
//PARAS  vector<Symbol*>
//INIT  vector<Expr*>
//ARGS  vector<Expr*>
//EXP & ARRS
class Expr;
class BinaryOp;
class UnaryOp;
class FuncCall;
class ArrayIndex;
class StMember;
class VariableExp;
class InstantNum;
//DEC
struct Declaration
{
	Symbol *s;
	vector<Expr*>  inits;
};

class Definition
{
protected:
	int lineno;//Error report
public:
	Definition(){lineno = yylineno;}
	virtual ~Definition(){}
	virtual int codeGen() = 0;
	int errorInfo( int errorT );
};

class VarDef : public Definition
{
protected:
	vector<vector<Expr*> > inits;
	vector<Symbol*> syms;
public:
	VarDef(vector<Declaration*>*decs);
	virtual ~VarDef(){}
	virtual int codeGen();
	const vector<Symbol*> & getSyms(){return syms;}
	int errorInfo( int errorT );
};

class ExtVarDef : public VarDef
{
public:
	ExtVarDef( vector<Declaration*>* decs)
		: VarDef(decs){}
	virtual ~ExtVarDef(){}
	virtual int codeGen();
	int errorInfo( int errorT );
};

class ExtFuncDef : public Definition
{
private:
	FuncSym* head;
	StmtBlock *body;
public:
	ExtFuncDef(FuncSym *h, StmtBlock* b)
		:head(h),body(b), Definition() {}
	virtual ~ExtFuncDef(){}
	virtual int codeGen();
	int errorInfo( int errorT );
};

class Stmt
{
protected:
	int lineno;//#用于codeGen中报告错误
	string *nextLabelP;
public:
	Stmt():nextLabelP(NULL){lineno = yylineno;}//#用于codeGen中报告错误
	virtual ~Stmt(){}
	virtual int codeGen() = 0;
	string *getNextP(){ return nextLabelP;}
	void setNextP(string *p){ nextLabelP = p;}
};

class StmtBlock : public Stmt
{
private:
	vector<Definition*> defs;
	vector<Stmt*> stmts;
public:
	StmtBlock(vector<Definition*>*d, vector<Stmt*>*s)
		:Stmt()
	{defs = *d;stmts = *s;delete d; delete s;}
	virtual ~StmtBlock(){}
	virtual int codeGen();
	int errorInfo( int errorT ){return errorT;}
};

class RetStmt : public Stmt
{
private:
	Expr* reval;
public:
	RetStmt(Expr *r )
		: reval(r), Stmt(){}
	virtual ~RetStmt(){}
	virtual int codeGen();
	int errorInfo( int errorT );
};

class IfStmt : public Stmt
{
private:
	Expr* condition;
	Stmt* thenStmt;
	Stmt * elseStmt;
public:
	IfStmt(Expr *c, Stmt *t, Stmt *e)
		: condition(c), thenStmt(t), elseStmt(e),Stmt(){}
	virtual ~IfStmt(){}
	virtual int codeGen();
	int errorInfo( int errorT );
};

class ForStmt : public Stmt
{
private:
	Expr* initExp;
	Expr* condExp;
	Expr* updateExp;
	Stmt* bodyStmt;
public:
	ForStmt( Expr* i, Expr* c, Expr* u, Stmt *b )
		:initExp(i), condExp(c), updateExp(u),bodyStmt(b),Stmt(){}
	virtual ~ForStmt(){}
	virtual int codeGen();
	int errorInfo( int errorT ){return errorT;};
};

class ContStmt : public Stmt
{
public:
	ContStmt():Stmt(){}
	virtual ~ContStmt(){}
	virtual int codeGen();
	int errorInfo( int errorT );
};

class BreakStmt : public Stmt
{
public:
	BreakStmt():Stmt(){}
	virtual ~BreakStmt(){}
	virtual int codeGen();
	int errorInfo( int errorT );
};

class ReadStmt : public Stmt
{
private:
	Expr *leftV;
public:
	ReadStmt(Expr *l)
		:leftV(l) ,Stmt(){}
	virtual ~ReadStmt(){}
	virtual int codeGen();
	int errorInfo( int errorT );
};

class WriteStmt : public Stmt
{
private:
	Expr *rightV;
public:
	WriteStmt(Expr*r)
		:rightV(r),Stmt(){}
	virtual ~WriteStmt(){}
	virtual int codeGen();
	int errorInfo( int errorT );
};

class ExprStmt : public Stmt
{
private:
	Expr* expr;
public:
	ExprStmt( Expr *e ): Stmt(), expr(e){}
	virtual ~ExprStmt(){};
	virtual int codeGen();
	int errorInfo( int errorT ){return errorT;}
};

class Expr
{
protected:
	Symbol *resultSym;
	string *trueP;
	string *falseP;
	int value;
	int lineno;
public:
	Expr() :resultSym(NULL),value(0),trueP(NULL),falseP(NULL){ lineno = yylineno;}
	virtual ~Expr(){}
	virtual int codeGen() = 0;

	Symbol * getResultSym() { return resultSym;}
	void setResultSym( Symbol *s ){ resultSym = s;}
	int getVal() {return value;}
	void setVal( int i ) { value = i;}
	string *getTrueP(){ return trueP;}
	string *getFalseP(){ return falseP;}
	void setTrueP( string *p ){ trueP = p;}
	void setFalseP( string *p ){ falseP = p;}
};

class BinaryOp : public Expr
{
private:
	Expr *leftCh;
	Expr *rightCh;
	int op;
public:
	BinaryOp( Expr *l, Expr* r, int o )
		: leftCh(l), rightCh(r),op(o), Expr(){}
	virtual int codeGen();
	int errorInfo( int errorT );
};

class UnaryOp : public Expr
{
private:
	Expr *ch;
	int op;
public:
	UnaryOp( Expr *c, int o )
		: ch(c), op(o), Expr(){}
	virtual ~UnaryOp(){}
	virtual int codeGen();
	int errorInfo( int errorT );
};

class FuncCall : public Expr
{
private:
	FuncSym *object;
	vector<Expr*> argList;
public:
	FuncCall( string *nm, vector<Expr*>* a );
	virtual int codeGen();
	int errorInfo( int errorT );
	virtual ~FuncCall(){}
	Symbol *getFuncSym(){ return object; }
};

class ArrayIndex : public Expr
{
private:
	Expr *arrayId;
	Expr *index;
	int dim;
public:
	ArrayIndex( Expr *s, Expr *i )
		: arrayId(s), index(i), dim(1),Expr(){}
	//ArrayIndex( string *nm, Expr *i );
	virtual ~ArrayIndex(){}
	virtual int codeGen();//检查下标是否越界
	int errorInfo( int errorT );
	int getDim(){ return dim;}
	//Symbol *getSym() { return arrayId;}
};

class StMember : public Expr
{
private:
	Expr *stExp;
	int index;
	string memberName;
public:
	StMember( Expr *s, int i )
		:stExp(s), index(i),Expr(){}
	StMember( Expr *s, string *nm )
		:stExp(s),index(-1),Expr(){memberName = *nm; delete nm;}
	virtual ~StMember(){};
	virtual int codeGen();
	int errorInfo( int errorT );
	int getIndex() { return index;}
};

class VariableExp : public Expr
{
private:
	Symbol *id;
public:
	VariableExp(Symbol *s )
		: id(s),Expr(){}
	VariableExp( string *nm );
	virtual ~VariableExp(){}
	virtual int codeGen();//建立符号表的工作已经在Parser中做了
	int errorInfo( int errorT ){return errorT;}
	Symbol *getSym(){ return id;}
};

class InstantNum : public Expr
{
private:
public:
	InstantNum( int v ) :Expr(){resultSym = NULL;value = v;}
	virtual ~InstantNum(){}
	virtual int codeGen();
	int errorInfo( int errorT );
};

extern vector<Expr*> globalInitCode;
extern vector<Definition*> extdefs;
void globalInitCodeGen();
void genAllCode();
Definition *handleVarDef(specNode *spec, vector<Declaration*>* decs = new vector<Declaration*>());
StTypeSym *handleStSpec( string *nm, vector<Definition*>* defs);
StTypeSym *handleStSpec(string *nm);
FuncSym *handleFunc(string *nm,vector<Symbol*>* paras = new vector<Symbol*>());
Declaration *handleDec(specNode *spec, varNode *v, vector<Expr*>* init = new vector<Expr*>() );
Symbol *handleAddSym(specNode *s, varNode *v);
#endif

/*
ANTNode.cpp
	This file is the most important part of the project.
	It handle the logic when generating code.
*/
#include "ANTNode.h"
#include "Print.h"
#include "y.tab.h"
#include <iostream>
using namespace std;

vector<string*> break_context;
vector<string*> cont_context;
vector<Symbol*> empty_symbolList;
vector<Expr*> empty_initList;
vector<Expr*> globalInitCode;
bool hasReturn = false;
bool hasMain = false;
/************************************************************************/
/*                  Tools                                               */
/************************************************************************/
//Provide a proper result for an error expression
Symbol *getErrorSym() 
{
	Symbol *s;
	s = SBT.addTempInt("#ERROR#");
	s->setReg("#ERROR#");
	s->setAddress("#ERROR#");
	s->setInit();
	return s;
}
//Provide a proper expression for an error statement
Expr *getErrorExpr() 
{
	Expr *exprP;
	Symbol *s = getErrorSym();
	exprP = new VariableExp(s);
	exprP->setResultSym(s);
	return exprP;
}
//Handle the store instruction
int handleIntAssign( Symbol *left, string right, int lineno ) 
{
	if( left->noAddr() )
	{
		printErrorMessage(lineno,left->getAddress(),"Cannot be assigned");
		left->setInit();
		left->setAddress("#ERROR#");
		left->setReg("#ERROR#");
		return ERROR;//Unknown error
	}
	printStore(left->getAddress(),right,"i32");
	left->setReg(right);
	left->setInit();
	return OK;
}
//Handle the store instruction with instant number
int handleIntAssign( Symbol *left, int right, int lineno )
{
	if( left->noAddr() )
	{
		printErrorMessage(lineno,left->getAddress(),"Cannot be assigned");
		left->setInit();
		left->setAddress("#ERROR#");
		return ERROR;//Unknowned error
	}
	printStore(left->getAddress(),right,"i32");
	left->setReg("");
	left->setInit();
	return OK;
}
 //Handle the load instruction if neccessary
int arrangeIntReg(Symbol *r, int lineno )
{
	if(r->getType() != INTSYM || r->noAddr()) //a instant number, struct, array or temp int
		return OK;
	if( !(r->hasInited())) 
	{
		printErrorMessage(lineno,r->getName(),"Used without initialized");
		r->setInit();
		r->setReg("#ERROR#");
		return ERROR; //Remember global variable is initialized as 0 in default case
	}
	r->setReg(getTempReg());
	printLoad(r->getReg(),r->getAddress(),"i32");
	return OK;
}
 //Handle branch instruction if neccessary
int handleControlFlow(string *tp, string *fp, Symbol *s, int lineno)
{
	string reg;
	if(tp != NULL && fp != NULL )
	{
		if( s->getType() != INTSYM )
		{
			printErrorMessage(lineno,s->getName(),"Non integer variable can not be used as condition");
			return ERROR;
		}
		arrangeIntReg(s,lineno);
		reg = getTempReg();
		printICmp(reg,s->getReg(),0,NE);
		printCondBr(reg,tp,fp);
	}
	return OK;
}
int handleControlFlow(string *tp, string *fp, int v  )
{
	if( tp != NULL && v != 0 )
		printBr(tp);
	else if( fp != NULL && v == 0 )
		printBr(fp);
	return OK;
}
/************************************************************************/
/*                             StmtBlock                                */
/************************************************************************/
int StmtBlock::codeGen()
{
	//cout << "##  In StmtBlock::codeGen\n";
	for( int i = 0; i < defs.size(); ++i )
	{
		//cout << "##  Handle the " << i << " definition\n";
		defs[i]->codeGen();
		delete defs[i];
	}
	llvmout << '\n';
	for( int i = 0; i < stmts.size(); ++i )
	{
		//cout << "##  Handle the " << i << " stmt\n";
		if( i == stmts.size()-1)
			stmts[i]->setNextP(nextLabelP);
		stmts[i]->codeGen();
		delete stmts[i];
	}
	// An empty block should handle the branch instruction itself
	if( nextLabelP != NULL && stmts.empty())
		printBr(nextLabelP);

	return OK;
}
/************************************************************************/
/*                              RetStmt                                 */
/************************************************************************/
int RetStmt::errorInfo( int errorT )
{
	switch(errorT)
	{
	case NULLEXP:
		printErrorMessage(lineno,"return stmt","Unexpected null expression");break;
	case UNINIT:
		printErrorMessage(lineno,"return stmt","Returned an uninitialized variable");break;
	case NOTINT:
		printErrorMessage(lineno,"return stmt","Returned an non-integer variable");break;
	default:
		printErrorMessage(lineno,"return stmt","Undefined error");break;
	}
	return errorT;
}
int RetStmt::codeGen()
{
	string reg;
	hasReturn = true;

	if(reval == NULL ) return errorInfo(NULLEXP);//Empty expression
	reval->codeGen();
	arrangeIntReg(reval->getResultSym(),lineno);

	if( reval->getResultSym()->getType() == INSTANT )
	{
		printStore("%.1",reval->getVal(),"i32");
	}
	else if( reval->getResultSym()->getType() == INTSYM )
	{
		printStore("%.1",reval->getResultSym()->getReg(),"i32");
	}
	else 
		return errorInfo(NOTINT); //Non-integer can not be used in computation

	printBr(&retLabel);

	delete reval;
	return OK;
}
/************************************************************************/
/*                              IfStmt                                  */
/************************************************************************/
int IfStmt::errorInfo( int errorT )
{
	if( errorT == NULLEXP )
		printErrorMessage(lineno,"if stmt","Unexpected null expression");
	else
		printErrorMessage(lineno,"if stmt","Undefined error");
	return errorT;
}
int IfStmt::codeGen()
{
	string *endLabelP;
	if( condition == NULL )
		return errorInfo(NULLEXP);//Can not use empty condition

	if( nextLabelP == NULL ) 
		endLabelP = addLabel("if_end");
	else endLabelP = nextLabelP;

	condition->setTrueP(addLabel("if_then"));
	//If-Then
	if( elseStmt == NULL )
	{
		condition->setFalseP(endLabelP);
		thenStmt->setNextP(endLabelP);

		condition->codeGen();
		printLabel(condition->getTrueP());
		thenStmt->codeGen();
	}
	//If-Then-Else
	else
	{
		condition->setFalseP(addLabel("if_else"));
		thenStmt->setNextP(endLabelP);
		elseStmt->setNextP(endLabelP);

		condition->codeGen();
		printLabel(condition->getTrueP());
		thenStmt->codeGen();
		printLabel(condition->getFalseP());
		elseStmt->codeGen();

		delete elseStmt;
	}

	if( nextLabelP == NULL )
	{
		nextLabelP = endLabelP;
		printLabel(endLabelP);
	}

	delete condition;
	delete thenStmt;
	return OK;
}
/************************************************************************/
/*                              ForStmt                                 */
/************************************************************************/
int ForStmt::codeGen()
{
	string *endLabelP;
	string *updateP = addLabel("update");
	string *beginP = addLabel("begin");

	//cout << "##  In ForStmt::codeGen()\n";
	if( nextLabelP == NULL ) 
		endLabelP = addLabel("end");
	else endLabelP = nextLabelP;

	break_context.push_back(endLabelP);
	cont_context.push_back(updateP);

	//Set label
	condExp->setTrueP(addLabel("loop"));
	condExp->setFalseP(endLabelP);
	bodyStmt->setNextP(updateP);
	//genCode
	if(initExp)	initExp->codeGen();
	printBr(beginP);
	printLabel(beginP);
	if(condExp) condExp->codeGen();
	printLabel(condExp->getTrueP());
	bodyStmt->codeGen();
	printLabel(updateP);
	if(updateExp) updateExp->codeGen();
	printBr(beginP);

	if( nextLabelP == NULL )
	{
		nextLabelP = endLabelP;
		printLabel(endLabelP);
	}

	break_context.pop_back();
	cont_context.pop_back();

	delete initExp;
	delete condExp;
	delete updateExp;
	delete bodyStmt;
	return OK;
}
/************************************************************************/
/*                              Breakont                                */
/************************************************************************/
int BreakStmt::errorInfo( int errorT )
{
	if(errorT == NOTFOR)
		printErrorMessage(lineno,"break stmt","Used outside a for-loop");
	else
		printErrorMessage(lineno,"break stmt","Undefined error");
	return errorT;
}
int ContStmt::errorInfo( int errorT )
{
	if(errorT == NOTFOR)
		printErrorMessage(lineno,"continue stmt","Used outside a for-loop");
	else
		printErrorMessage(lineno,"continue stmt","Undefined error");
	return errorT;
}
int BreakStmt::codeGen()
{
	if(break_context.empty()) 
		return errorInfo(NOTFOR); //Break can only be used in FOR stmt
	printBr(break_context.back());
	return OK;
}
int ContStmt::codeGen()
{
	if(cont_context.empty())
		return errorInfo(NOTFOR);//Continue can only be used in FOR stmt
	printBr(cont_context.back());
	return OK;
}
/************************************************************************/
/*                              Read & Write                            */
/************************************************************************/
int ReadStmt::errorInfo( int errorT )
{
	switch(errorT)
	{
	case NULLEXP:printErrorMessage(lineno,"read stmt","Unexpected null expression");break;
	case NOTLEFT:printErrorMessage(lineno,"read stmt","Expected a left value expression");break;
	case NOTINT:printErrorMessage(lineno,"read stmt","Can not use non-integer variable as argument");break;
	default:printErrorMessage(lineno,"read stmt","Undefined error");break;
	}
	leftV = getErrorExpr();
	return errorT;
}
int ReadStmt::codeGen()
{
	string reg;
	if(leftV == NULL) return errorInfo(NULLEXP);//Empty expression
	leftV->codeGen();

	if(leftV->getResultSym()->getType() != INTSYM )
		return errorInfo(NOTINT);//The destination of Read Function can only be of int type

	if( dynamic_cast<VariableExp*>(leftV) == NULL &&
		dynamic_cast<StMember*>(leftV) == NULL &&
		dynamic_cast<ArrayIndex*>(leftV) == NULL )
		return errorInfo(NOTLEFT); //Not a left value

	/*Assign Logic*/
	reg = getTempReg();
	printRead(reg);

	handleIntAssign(leftV->getResultSym(),reg,lineno);

	if(nextLabelP != NULL )
		printBr(nextLabelP);
	delete leftV;
	return OK;
}
int WriteStmt::errorInfo( int errorT )
{
	switch(errorT)
	{
	case NULLEXP:printErrorMessage(lineno,"write stmt","Unexpected null expression");break;
	case UNINIT:printErrorMessage(lineno,"write stmt","Using uninitialized variable");break;
	case NOTINT:printErrorMessage(lineno,"write stmt","Can not use non-integer variable as argument");break;
	default:printErrorMessage(lineno,"write stmt","Undefined error");break;
	}
	return errorT;
}
int WriteStmt::codeGen()
{
	string reg;
	if( rightV == NULL ) return errorInfo(NULLEXP);//Empty expression
	rightV->codeGen();

	if( rightV->getResultSym()->getType() == INTSYM )
	{
		arrangeIntReg(rightV->getResultSym(),lineno);
		printWrite(rightV->getResultSym()->getReg());
	}
	else if( rightV->getResultSym()->getType() == INSTANT )
		printWrite(rightV->getVal());
	else 
		return errorInfo(NOTINT);//Only integer can be wrote
	if(nextLabelP != NULL )
		printBr(nextLabelP);
	delete rightV;
	return OK;
}
/************************************************************************/
/*                              ExprStmt                                */
/************************************************************************/
int ExprStmt::codeGen()
{
	if( expr != NULL )
		expr->codeGen();

	if(nextLabelP != NULL )
		printBr(nextLabelP);
	delete expr;
	return OK;
}

/************************************************************************/
/*                             InstantNum                               */
/************************************************************************/
int InstantNum::codeGen()
{
	resultSym = SBT.addInstant();
	handleControlFlow(trueP,falseP,value);
	return OK;
}
/************************************************************************/
/*                           VariableExp                                */
/************************************************************************/
VariableExp::VariableExp( string *nm )
	:Expr()
{
	id = SBT.findSym(*nm,true);
	if( id == NULL) printErrorMessage(lineno,*nm,"Has not been defined");
	delete nm;
}
int VariableExp::codeGen()
{
	if( id->getType() == FUNCSYM )
	{
		printErrorMessage(lineno,id->getName(),"Can not be used as a variable");
		resultSym = getErrorSym();
		return ERROR;
	}
	resultSym = id;
	handleControlFlow(trueP,falseP,resultSym,lineno);

	return OK;
}
/************************************************************************/
/*                           StMember                                   */
/************************************************************************/
int StMember::errorInfo( int errorT )
{
	switch(errorT)
	{
	case NULLEXP:printErrorMessage(lineno,"struct member expr","Unexpected null expression");break;
	case NOTSTRUCT:printErrorMessage(lineno,"struct member expr","Applying DOT to non-struct");break;
	case NOTMEMBER: printErrorMessage(lineno,"struct member expr","No such member");break;
	default:
		printErrorMessage(lineno,"struct member expr","Undefined error");break;
	}
	resultSym = getErrorSym();
	return errorT;
}
int StMember::codeGen()
{
	StSym *stP;
	string reg;
	if( stExp == NULL )
		return errorInfo(NULLEXP); //Empty expression
	stExp->codeGen();

	if( stExp->getResultSym()->getType() != STRUCTSYM )
		return errorInfo(NOTSTRUCT); //Only struct symbol can use DOT operation
	stP = static_cast<StSym*>(stExp->getResultSym());

	if( index < 0 )
	{
		index = stP->getStType()->findMemIndex(memberName);
		if( index < 0 )
			return errorInfo(NOTMEMBER);
	}

	reg = getTempReg();
	printGetElePtr_st(reg, stP->getAddress(),stP->getStTypeAddr(),index);

	resultSym = SBT.addTempInt("");
	resultSym->setAddress(reg);

	handleControlFlow(trueP,falseP,resultSym,lineno);

	delete stExp;
	return OK;
}
/************************************************************************/
/*                           ArrayIndex                                 */
/************************************************************************/
/*ArrayIndex::ArrayIndex( string *nm, Expr *i )
	:Expr()
{
	arrayId = SBT.findSym(*nm,true);
	if( arrayId == NULL) printErrorMessage(lineno,*nm,"Has not been defined");
	index = i;
	delete nm;
}*/
int ArrayIndex::errorInfo( int errorT )
{
	switch(errorT)
	{
	case NULLEXP:printErrorMessage(lineno,"array index expr","Unexpected null expression");break;
	case OUTOFBORDER:printErrorMessage(lineno,"array index expr","Index out of border");break;
	case NOTARRAY:printErrorMessage(lineno,"array index expr","Applying '[]' to non-array");break;
	default:printErrorMessage(lineno,"array index expr","Undefined error");break;
	}
	resultSym = getErrorSym();
	return errorT;
}
int ArrayIndex::codeGen()
{
	string reg;
	ArraySym *aP;
	vector<int> lenList;

	if( index == NULL  || arrayId == NULL )
		return errorInfo(NULLEXP);//Empty expression
	arrayId->codeGen();
	index->codeGen();
	arrangeIntReg(index->getResultSym(),lineno);
	reg = getTempReg();

	if( arrayId->getResultSym()->getType() == INTARRAY )
	{
		aP = static_cast<ArraySym*>(arrayId->getResultSym());
		if( index->getResultSym()->getType() == INSTANT )//Instant index
		{
			if( !aP->isPointer() && index->getVal() >= aP->getLen())
				return errorInfo(OUTOFBORDER);//Out of border
			printGetElePtr_iA(reg, aP->getAddress(),
					aP->getLenListP(), index->getVal());
		}
		else
			printGetElePtr_iA(reg, aP->getAddress(),
			aP->getLenListP(), index->getResultSym()->getReg());

		if(aP->getDim() > 1 )
		{
			for( int i = 1; i < aP->getDim(); ++i )
				lenList.push_back(aP->getLenListP()->at(i));
			resultSym = SBT.addTempIA("",&lenList);
		}
		else
			resultSym = SBT.addTempInt("");
		resultSym->setAddress(reg);
	}
	else if( arrayId->getResultSym()->getType() == STARRAY )
	{
		aP = static_cast<ArraySym*>(arrayId->getResultSym());
		if( index->getResultSym()->getType() == INSTANT )//Instant index
		{
			if(!aP->isPointer() && index->getVal() >= aP->getLen())
				return errorInfo(OUTOFBORDER);//Out of border
			printGetElePtr_sA(reg,aP->getAddress(),static_cast<StArray*>(aP)->getStTypeAddr(),
				aP->getLenListP(),index->getVal());
		}
		else
			printGetElePtr_sA(reg,aP->getAddress(),static_cast<StArray*>(aP)->getStTypeAddr(),
				aP->getLenListP(),index->getResultSym()->getReg());

		if( aP->getDim() > 1 )
		{
			for( int i = 1; i < aP->getDim(); ++i )
				lenList.push_back(aP->getLenListP()->at(i));
			resultSym = SBT.addTempSA("",&lenList,static_cast<StArray*>(aP)->getStType());
		}
		else 
			resultSym = SBT.addTempSt("",static_cast<StArray*>(aP)->getStType());
		resultSym->setAddress(reg);
	}
	else return errorInfo(NOTARRAY);//Only array symbol can use [] operator

	handleControlFlow(trueP,falseP,resultSym,lineno);

	delete index;
	return OK;
}
/************************************************************************/
/*                           FuncCall                                   */
/************************************************************************/
FuncCall::FuncCall( string *nm, vector<Expr*>* a )
{
	object = SBT.findFuncSym(*nm);
	if( object == NULL) printErrorMessage(lineno,*nm,"Has not been defined");
	argList = *a;
	delete nm;
	delete a;
}
int FuncCall::errorInfo( int errorT )
{
	switch(errorT)
	{
	case NOTMATCH:
		printErrorMessage(lineno,"function call","Arguments do not match the definition of the function");break;
	case NULLEXP:printErrorMessage(lineno,"function call","Unexpected null expression");break;
	default:printErrorMessage(lineno,"function call","Undefined error");break;
	}
	resultSym = getErrorSym();
	return errorT;
}
int FuncCall::codeGen()
{
	string reg,reg2;
	StSym *stP;
	IntArray *iaP;
	StArray *saP;

	if( argList.size() != object->getArgCount() )
		return errorInfo(NOTMATCH);//Argument count don't match

	for( int i = 0; i < argList.size(); ++i )
	{
		if( argList[i] == NULL )
			return errorInfo(NULLEXP);//Empty expression

		argList[i]->codeGen();
		arrangeIntReg(argList[i]->getResultSym(),lineno);
		//Arrange for Struct Arrgument.
		//Because it's called by value, we need to make a copy of it
		if(argList[i]->getResultSym()->getType() == STRUCTSYM )
		{
			stP = static_cast<StSym*>(argList[i]->getResultSym());//stP is the original Struct Symbol
			reg = getTempReg();
			printStrVarDef(reg,stP->getStTypeAddr(),LOCALVAR);
			argList[i]->setResultSym(SBT.addTempSt(reg,stP->getStType()));

			reg = getTempReg();
			printBitCast(reg,argList[i]->getResultSym()->getAddress(),stP->getStTypeAddr());
			reg2 = getTempReg();
			printBitCast(reg2,stP->getAddress(),stP->getStTypeAddr());
			printMemCpy(reg,reg2,stP->getMemCount());
		}
	}
	//Return value
	reg = getTempReg();
	resultSym = SBT.addTempInt("");
	resultSym->setReg(reg);

	//Start Printing
	llvmout << "  " << reg << " = call i32 " 
		<< object->getAddress() << "(";
	for( int i = 0; i < object->getArgCount(); ++i )
	{
		if(!sameType(object->getArgSym(i),argList[i]->getResultSym()))
			return errorInfo(NOTMATCH); //Argument type don't match

		switch(argList[i]->getResultSym()->getType())
		{
		case INTSYM:
			llvmout << (i==0?" ":", ") << "i32 " 
				<< argList[i]->getResultSym()->getReg();
			break;
		case INSTANT:
			llvmout << (i==0?" ":", ") << "i32 " << argList[i]->getVal();
			break;
		case STRUCTSYM:
			llvmout << (i==0?" ":", ") << (static_cast<StSym*>(object->getArgSym(i)))->getStTypeAddr()
				<< "* byval align 4 " 
				<< argList[i]->getResultSym()->getAddress();
			break;
		case INTARRAY:
			printErrorMessage(lineno,"In Function call","Can not use array as argument");
			llvmout << (i==0?" ":", ") << "i32* " 
				<< argList[i]->getResultSym()->getAddress();
			break;
		case STARRAY:
			printErrorMessage(lineno,"In Function call","Can not use array as argument");
			llvmout << (i==0?" ":", ") << (static_cast<StArray*>(object->getArgSym(i)))->getStTypeAddr()
				<< "* " 
				<< argList[i]->getResultSym()->getAddress();
		default:
			return errorInfo(-65536);
		}
		delete argList[i];
	}
	llvmout << ")\n";

	handleControlFlow(trueP,falseP,resultSym,lineno);

	return OK;
}
/************************************************************************/
/*                           UnaryOp                                    */
/************************************************************************/
int UnaryOp::errorInfo( int errorT )
{
	switch (errorT)
	{
	case NOTINT:printErrorMessage(lineno,"unarayOp expr","Non-integer can not be used in calculation");break;
	case NULLEXP:printErrorMessage(lineno,"unaryOp expr","Unexpected null expression");break;
	default:printErrorMessage(lineno,"unaryOp expr","Undefined error");break;
	}
	resultSym = getErrorSym();
	return errorT;
}
int UnaryOp::codeGen()
{
	string reg,reg2;

	//First handle the NOT expression used as control flow statement
	if( (trueP != NULL || falseP != NULL) && op == NOT )
	{
		ch->setTrueP(falseP);
		ch->setFalseP(trueP);
		ch->codeGen();
		resultSym = NULL;
		delete ch;
		return OK;
	}

	if( ch == NULL )
		return errorInfo(NULLEXP); //Empty expression

	ch->codeGen();
	arrangeIntReg(ch->getResultSym(),lineno);

	if( op == UADD || op == USUB )
	{
		if( ch->getResultSym()->getType() != INTSYM )
			return errorInfo(NOTINT); 

		reg = getTempReg();
		printOp(reg,ch->getResultSym()->getReg(),
			((op == UADD) ? 1 : -1), ADD );
		handleIntAssign(ch->getResultSym(),reg,lineno);

		resultSym = ch->getResultSym();

		handleControlFlow(trueP,falseP,resultSym,lineno);
	}
	else 
	{
		if( ch->getResultSym()->getType() == INSTANT )
		{
			resultSym = SBT.addInstant();
			switch(op)
			{
			case SUB: value = -ch->getVal();break;
			case NOT: value = !ch->getVal();break;
			case BNOT: value = ~ch->getVal();break;
			default:
				return errorInfo(-65536);
			}

			handleControlFlow(trueP,falseP,value);
		}
		else if( ch->getResultSym()->getType() == INTSYM )
		{
			resultSym = SBT.addTempInt("");
			switch(op)
			{
			case NOT:
				reg = getTempReg();
				printICmp(reg,ch->getResultSym()->getReg(),0,NE);
				reg2 = getTempReg();
				printXOr(reg2, reg,"true");
				reg = getTempReg();
				printZext(reg,reg2);
				resultSym->setReg(reg);
				break;
			case BNOT:
				reg = getTempReg();
				printXOr(reg, ch->getResultSym()->getReg(),"-1");
				resultSym->setReg(reg);

				handleControlFlow(trueP,falseP,resultSym,lineno);
				break;
			case SUB:
				reg = getTempReg();
				printOp(reg,0,ch->getResultSym()->getReg(),SUB);
				resultSym->setReg(reg);

				handleControlFlow(trueP,falseP,resultSym,lineno);
				break;
			default:
				return errorInfo(-65536);
			}
		}
		else 
			return errorInfo(NOTINT);
	}

	delete ch;
	return OK;
}
/************************************************************************/
/*                           BinaryOp                                   */
/************************************************************************/
int BinaryOp::errorInfo( int errorT )
{
	switch(errorT)
	{
	case NULLEXP:printErrorMessage(lineno,"binaryOp expr","Unexpected null expression");break;
	case ANDORBUG:printErrorMessage(lineno,"binaryOp expr",
					  "BUG: AND and OR operator can not be used in non-condeition expresion");break;
	case NOTLEFT:printErrorMessage(lineno,"binaryOp expr","Expected a left value");break;
	case TYPEERROR:printErrorMessage(lineno,"binaryOp expr","Type error");break;
	case NOTINT:printErrorMessage(lineno,"binaryOp expr","Non-inteager can not used in calculation");break;
		default:printErrorMessage(lineno,"binaryOp expr","Undefined error");break;
	}
	resultSym = getErrorSym();
	return errorT;
}
int BinaryOp::codeGen()
{
	string reg, reg2;
	StSym *stPL, *stPR;
	Expr *exprP, *exprP2;
	int op2;

	if( leftCh == NULL || rightCh == NULL )
		return errorInfo(NULLEXP);	//Empty expression

	//First handle AND/OR used as control flow statement
	if( op == AND && trueP != NULL && falseP != NULL )
	{
		leftCh->setFalseP(falseP);
		leftCh->setTrueP(addLabel("trueL"));
		rightCh->setFalseP(falseP);
		rightCh->setTrueP(trueP);

		leftCh->codeGen();
		printLabel(leftCh->getTrueP());
		rightCh->codeGen();

		resultSym = NULL;
		delete leftCh;
		delete rightCh;
		return OK;
	}
	else if( op == OR && trueP != NULL && falseP != NULL )
	{
		leftCh->setTrueP(trueP);
		leftCh->setFalseP(addLabel("falseL"));
		rightCh->setTrueP(trueP);
		rightCh->setFalseP(falseP);

		leftCh->codeGen();
		printLabel(leftCh->getFalseP());
		rightCh->codeGen();

		resultSym = NULL;
		delete leftCh;
		delete rightCh;
		return OK;
	}
	//BUG!Although it can pass the parse, we don't allow AND/OR use as ordinary expression
	else if( op == OR || op == AND )
		return errorInfo(ANDORBUG);

	if( op == AN )//Assign
	{
		if( dynamic_cast<VariableExp*>(leftCh) == NULL &&
			dynamic_cast<StMember*>(leftCh) == NULL &&
			dynamic_cast<ArrayIndex*>(leftCh) == NULL )
			return errorInfo(NOTLEFT); //Only left value expreesion can be assigned

		leftCh->codeGen();
		rightCh->codeGen();
		arrangeIntReg(rightCh->getResultSym(),lineno);

		if( !sameType(leftCh->getResultSym(),rightCh->getResultSym()))
			return errorInfo(TYPEERROR); //Can not assigned between different types
		//Handle the left part
		if( leftCh->getResultSym()->getType() == STRUCTSYM )//
		{
			stPR = static_cast<StSym*>(rightCh->getResultSym());//stPR is the source Struct Symbol
			stPL = static_cast<StSym*>(leftCh->getResultSym()); //stPL is the dest Struct Symbol
			reg = getTempReg();
			printBitCast(reg,stPL->getAddress(),stPL->getStTypeAddr());
			reg2 = getTempReg();
			printBitCast(reg2,stPR->getAddress(),stPR->getStTypeAddr());
			printMemCpy(reg,reg2,stPL->getMemCount());
		}
		else if( leftCh->getResultSym()->getType() == INTSYM )
		{
			if( rightCh->getResultSym()->getType() == INSTANT )
				handleIntAssign(leftCh->getResultSym(),rightCh->getVal(),lineno);
			else
				handleIntAssign(leftCh->getResultSym(),rightCh->getResultSym()->getReg(),lineno);
		}
		else 
			return errorInfo(NOTLEFT);

		resultSym = leftCh->getResultSym();
		handleControlFlow(trueP,falseP,resultSym,lineno);
	}
	else if( op < AN && op >= SRA )//Operate and assign
	{
		if( dynamic_cast<VariableExp*>(leftCh) == NULL &&
			dynamic_cast<StMember*>(leftCh) == NULL &&
			dynamic_cast<ArrayIndex*>(leftCh) == NULL )
			return errorInfo(NOTLEFT);

		switch(op)
		{
		case ADDA: op2 = ADD;break;
		case SUBA: op2 = SUB;break;
		case MULA: op2 = MUL;break;
		case DIVA: op2 = DIV;break;
		case BANDA: op2 = BAND;break;
		case BXORA: op2 = BXOR;break;
		case BORA: op2 = BOR;break;
		case SLA: op2 = SL;break;
		case SRA: op2 = SR;break;
		}
		leftCh->codeGen();
		exprP = new VariableExp(leftCh->getResultSym());
		exprP2 = new VariableExp(leftCh->getResultSym());
		exprP = new BinaryOp(exprP,rightCh,op2);
		exprP = new BinaryOp(exprP2,exprP,AN);
		exprP->codeGen();
		resultSym = exprP->getResultSym();

		handleControlFlow(trueP,falseP,resultSym,lineno);

		delete exprP;
		delete leftCh;
		return OK;
	}
	else // Not assign operation
	{
		leftCh->codeGen();
		arrangeIntReg(leftCh->getResultSym(),lineno);
		reg2 = leftCh->getResultSym()->getReg();
		rightCh->codeGen();
		arrangeIntReg(rightCh->getResultSym(),lineno);

		if( !sameType(leftCh->getResultSym(),rightCh->getResultSym()))
			return errorInfo(TYPEERROR); 

		if(leftCh->getResultSym()->getType() == INSTANT && 
			rightCh->getResultSym()->getType() == INSTANT )
		{
			resultSym = SBT.addInstant();
			switch(op)
			{
			case MUL: value = leftCh->getVal() * rightCh->getVal();break;
			case DIV: value = leftCh->getVal() / rightCh->getVal();break;
			case MOD: value = leftCh->getVal() % rightCh->getVal();break;
			case ADD: value = leftCh->getVal() + rightCh->getVal();break;
			case SUB: value = leftCh->getVal() - rightCh->getVal();break;
			case SL: value = leftCh->getVal() << rightCh->getVal();break;
			case SR: value = leftCh->getVal() >> rightCh->getVal();break;
			case GT: value = leftCh->getVal() > rightCh->getVal();break;
			case GE: value = leftCh->getVal() >= rightCh->getVal();break;
			case LT: value = leftCh->getVal() < rightCh->getVal();break;
			case LE: value = leftCh->getVal() <= rightCh->getVal();break;
			case EQ: value = leftCh->getVal() == rightCh->getVal();break;
			case NE: value = leftCh->getVal() != rightCh->getVal();break;
			case BAND: value = leftCh->getVal() & rightCh->getVal();break;
			case BXOR: value = leftCh->getVal() ^ rightCh->getVal();break;
			case BOR: value = leftCh->getVal() | rightCh->getVal();break;
			case AND: value = leftCh->getVal() && rightCh->getVal();break;
			case OR:  value = leftCh->getVal() || rightCh->getVal();break;
			default:
				return errorInfo(-65536);
			}

			handleControlFlow(trueP,falseP,value);
		}
		else if(leftCh->getResultSym()->getType() == INTSYM 
			|| rightCh->getResultSym()->getType() == INTSYM) 
		{
			if( op >= NE && op <= GT ) 
			{
				reg = getTempReg();
				if( leftCh->getResultSym()->getType() == INSTANT )
				printICmp(reg,leftCh->getVal(),rightCh->getResultSym()->getReg(),op);
				else if( rightCh->getResultSym()->getType() == INSTANT )
					printICmp(reg,reg2, rightCh->getVal(),op);
				else
					printICmp(reg,reg2, rightCh->getResultSym()->getReg(), op);

				if( trueP == NULL && falseP == NULL)
				{
					reg2 = getTempReg();
					printZext(reg2, reg);
					resultSym = SBT.addTempInt("");
					resultSym->setReg(reg2);
				}
				else 
				{
					printCondBr(reg,trueP,falseP);
					resultSym = NULL;
				}
			}
			else //Common computation
			{
				reg = getTempReg();
				if( leftCh->getResultSym()->getType() == INSTANT )
					printOp(reg,leftCh->getVal(),rightCh->getResultSym()->getReg(),op);
				else if( rightCh->getResultSym()->getType() == INSTANT )
					printOp(reg,reg2, rightCh->getVal(),op);
				else
					printOp(reg,reg2, rightCh->getResultSym()->getReg(), op);
				resultSym = SBT.addTempInt("");
				resultSym->setReg(reg);
				handleControlFlow(trueP,falseP,resultSym,lineno);
			}
		}
		else
			return errorInfo(NOTINT);
	}
	
	delete leftCh;
	delete rightCh;
	return OK;
}
/************************************************************************/
/*                                 VarDef                               */
/************************************************************************/
int VarDef::errorInfo( int errorT )
{
	if(NOTMATCH)
		printErrorMessage(lineno,"Variable Definition","Initialization doesn't match the variable");
	else
		printErrorMessage(lineno,"Variable Definition","Undefined error");
	return errorT;
}
VarDef::VarDef(vector<Declaration*>*decs)
	:Definition()
{
	if( decs != NULL )
	{
		for( int i = 0; i < decs->size(); ++i )
		{
			syms.push_back(decs->at(i)->s);
			inits.push_back(decs->at(i)->inits);
			delete decs->at(i);
		}
		delete decs;
	}
}
int VarDef::codeGen()
{
	int i, j;
	SymbolType varType;
	Expr *exprP, *exprP2;
	StSym *stP;
	IntArray *iaP;
	StArray *saP;

	//cout << "##  In VarDef::codeGen()\n";
	varType = syms.back()->getType();
	for( i = 0; i < syms.size(); ++i )
	{
		//cout << "##    Handling the " << i << "th declaration\n";
		switch(varType)
		{
		case INTSYM:
			//cout << "##    Find an IntSym\n";
			printLocalIntDef(syms[i]->getAddress());
			if( !inits[i].empty() ) 
			{
				if( inits[i].size() > 1 ) return errorInfo(NOTMATCH);//Too many init

				exprP = new VariableExp(syms[i]);
				exprP = new BinaryOp(exprP,inits[i][0],AN);
				exprP->codeGen();
				delete exprP;//DELETE
			}
			break;
		case STRUCTSYM:
			//cout << "##    Find a StructSym\n";
			stP = static_cast<StSym*>(syms[i]);

			if(inits[i].size() > stP->getMemCount() )
				return errorInfo(NOTMATCH);	

			printStrVarDef(syms[i]->getAddress(),stP->getStTypeAddr(), LOCALVAR);

			if(!inits[i].empty())
				for( j = 0; j < stP->getMemCount(); ++j )
				{
					//cout << "##    Handle the " << j << "th member's init.\n";
					exprP = new VariableExp(syms[i]);
					exprP = new StMember(exprP, j );
					if( j < inits[i].size())
						exprP = new BinaryOp(exprP,inits[i][j], AN );
					else
					{
						//cout << "##      Default inited as 0\n";
						exprP2 = new InstantNum(0);
						exprP = new BinaryOp(exprP,exprP2,AN);
					}
					exprP->codeGen();
					delete exprP;
				}
			break;
		case  INTARRAY:
			//cout << "##    Find an IntArray\n";
			iaP = static_cast<IntArray*>(syms[i]);
			if(inits[i].size() > iaP->getLen() )
				return errorInfo(NOTMATCH);			//初始化列表不能超过数组长度
			printIntArrayDef(iaP->getAddress(), iaP->getLenListP(),LOCALVAR);

			if(!inits[i].empty())
				for( j = 0; j < iaP->getLen(); ++j )
				{
					//cout << "##    Handle the " << j << "th element's init\n";
					exprP = new InstantNum(j);
					exprP2 = new VariableExp(iaP);
					exprP = new ArrayIndex(exprP2, exprP );
					if( j < inits[i].size())
						exprP = new BinaryOp(exprP,inits[i][j],AN);
					else
					{
						//cout << "##      Default inited as 0\n";
						exprP2 = new InstantNum(0);
						exprP = new BinaryOp(exprP,exprP2,AN);
					}
					exprP->codeGen();
					delete exprP;
				}
			break;
		case STARRAY:
			//Ignore the init of StArray because of the grammar
			//cout << "##    Find a StArray\n";
			saP = static_cast<StArray*>(syms[i]);
			printStrArrayDef(saP->getAddress(), saP->getLenListP(),saP->getStTypeAddr(), LOCALVAR);
			break;
		default:
			return errorInfo(-65536);//Unknown Error
		}
	}
	return OK;
}
/************************************************************************/
/*                               ExtVarDef                              */
/************************************************************************/
int ExtVarDef::errorInfo( int errorT )
{
	if(NOTMATCH)
		printErrorMessage(lineno,"Extern Variable Definition","Initialization doesn't match the variable");
	else
		printErrorMessage(lineno,"Extern Variable Definition","Undefined error");
	return errorT;
}
int ExtVarDef::codeGen()
{
	int i,j;
	SymbolType varType;
	Expr *exprP, *exprP2;
	StSym *stP;
	IntArray *iaP;
	StArray *saP;
	
	if( syms.empty() ) return OK;
	
	varType = syms.back()->getType();
	//cout << "##  Find an extern var definition of size " << syms.size() << '\n';
	for( i = 0; i < syms.size(); ++i )
	{
		switch(varType)
		{
		case INTSYM:
			//cout << "##  Find an extern integer: " << syms.at(i)->getName() << "\n";
			if( !inits[i].empty() ) 
			{
				if( inits[i].size() > 1 ) return errorInfo(NOTMATCH); //Too many inits

				syms[i]->setInit();
				exprP = new VariableExp(syms[i]);
				exprP = new BinaryOp(exprP, inits[i][0], AN );
				globalInitCode.push_back(exprP);
			}
			printGloIntDef(syms[i]->getAddress());
			break;
		case STRUCTSYM:
			//cout << "##  Find an extern struct: " << syms.at(i)->getName() << "\n";
			stP = static_cast<StSym*>(syms[i]);
			//The length of inits can be smaller than the length of struct
			if(inits[i].size() > stP->getMemCount() ) return errorInfo(NOTMATCH);
			//cout << "##    Has init.size = " << inits[i].size() << '\n';

			for( j = 0; j < inits[i].size(); ++j )
			{
				exprP = new VariableExp(syms[i]);
				exprP = new StMember(exprP, j );
				exprP = new BinaryOp(exprP,inits[i][j], AN );
				globalInitCode.push_back(exprP);
			}
			printStrVarDef(syms[i]->getAddress() ,stP->getStTypeAddr(), GLOBALVAR );
			break;
		case INTARRAY:
			//cout << "##  Find an extern int array: " << syms.at(i)->getName() << "\n";
			iaP = static_cast<IntArray*>(syms[i]);
			//cout << "##    Has init.size = " << inits[i].size() << '\n';
			//The length of inits can be smaller than the length of array
			if(inits[i].size() > iaP->getLen() ) return errorInfo(NOTMATCH);

			for( j = 0; j < inits[i].size(); ++j )
			{
				//cout << "##    Gennerating code for the " << j << " th element\n";
				exprP = new InstantNum(j);
				exprP2 = new VariableExp(iaP);
				exprP = new ArrayIndex(exprP2, exprP );
				exprP = new BinaryOp(exprP,inits[i][j],AN);
				globalInitCode.push_back(exprP);
				//cout << "##    Finished\n";
			}
			printIntArrayDef(iaP->getAddress(), iaP->getLenListP(),GLOBALVAR );
			//cout << "##  Finished\n";
			break;
		case STARRAY:
			//Since the grammar doesn't provide the initialization of StArray
			//I just ignore it.
			//cout << "##  Find an extern StArray\n";
			saP = static_cast<StArray*>(syms[i]);
			printStrArrayDef(saP->getAddress(),saP->getLenListP(),saP->getStTypeAddr(),GLOBALVAR);
			break;
		default:
			return errorInfo(-65536);
		}
	}
	//cout << "##  A ExtVarDef is handled.\n";
	return OK;
}
/************************************************************************/
/*                               ExtFuncDef                             */
/************************************************************************/
int ExtFuncDef::errorInfo( int errorT )
{
	if( errorT = NORETURN )
		printErrorMessage(lineno,"Function Definition","No return stmtment");
	else 
		printErrorMessage(lineno,"Function Definition","Undefined error");
	return errorT;
}
int ExtFuncDef::codeGen()
{

	StSym *stP;
	StArray *saP;
	string curReg;
	llvmout << "\ndefine i32 " << head->getAddress() << '(';
	
	//cout << "##  In ExtFuncDef::codeGen()\n";	

	for( int i = 0; i < head->getArgCount(); ++i )
	{
		switch(head->getArgSym(i)->getType())
		{
		case INTSYM:
			head->getArgSym(i)->setReg(head->getArgSym(i)->getAddress());
			llvmout << (i == 0?"":",") << " i32 " 
				<< head->getArgSym(i)->getReg();
			break;
		case STRUCTSYM:
			//Call by value
			stP = static_cast<StSym*>(head->getArgSym(i));
			llvmout << (i == 0?" ":", ") << stP->getStTypeAddr()
				<< "* byval align 4 " << stP->getAddress();
			break;
		case INTARRAY:
			//Make it into  pointer
			static_cast<IntArray*>(head->getArgSym(i))->makePointer();
			llvmout << (i == 0?"":",") << " i32* "
				<<  head->getArgSym(i)->getAddress();
			break;
		case STARRAY:
			saP = static_cast<StArray*>(head->getArgSym(i));
			//Make it into a pointer
			saP->makePointer();
			llvmout << (i == 0?" ":", ") << saP->getStTypeAddr()
				<< "* " << saP->getAddress();
			break;
		default:
			return errorInfo(-65536);//Unknown error
		}
	}
	llvmout << ") {\n";

	//Handling main function
	if(head->getName() == "main" )
	{
		//cout << "##  Find Main\n";
		hasMain = true;
		printHeadOfMain();
		//cout << "##  Print Head of main finished.\n";
	}
	resetRegAndLabel();
	printLocalIntDef(getTempReg());//Return value is always *%1
	//cout << "##  Print local int def %1 for return value\n";
	//Arrange space for arguments
	for(int i = 0; i < head->getArgCount(); ++i )
		if( head->getArgSym(i)->getType() == INTSYM )
		{
			curReg = getTempReg();
			printLocalIntDef(curReg);
			head->getArgSym(i)->setAddress(curReg);

			printStore(curReg,head->getArgSym(i)->getReg(),"i32");
			head->getArgSym(i)->setInit();
		}

	//StmtBlock
	hasReturn = false;
	body->codeGen();
	if( !hasReturn )
		return errorInfo(NORETURN);
	delete body;

	//Print label
	printLabel(&retLabel);
	curReg = getTempReg();
	printLoad(curReg,"%.1","i32");
	llvmout << "  ret i32 " << curReg << "\n}\n";

	//cout << "##  Finished\n";
	return OK;
}

void globalInitCodeGen()
{
	llvmout << "\ndefine internal void @" << InitFunc << "() {\n";
	resetRegAndLabel();
	for( int i = 0; i < globalInitCode.size(); ++i )
	{
		globalInitCode[i]->codeGen();
		delete globalInitCode[i];
	}
	llvmout << "  ret void\n}\n\n";
}

void genAllCode()
{
	//Print head
	//cout << "##  In genAllCode\n";
	llvmout << "; ModuleID = \"" << inputFileName << "\"\n";
	//llvmout << "target datalayout = \"" << datalayout << "\"\n";
	//llvmout << "target triple = \"" << triple << "\"\n\n\n";
	//cout << "##  Finish printing head\n";
	//Print code
	SBT.allStDecGen();
	//cout << "##  " <<  extdefs.size() << endl;

	for(int i = 0; i < extdefs.size();++i )
		extdefs[i]->codeGen();
	llvmout << '\n';
	globalInitCodeGen();
	printWriteDef();
	printReadDef();
	printMemCpyDef();
	//Check
	if( !hasMain )
		cerr << "Error[" << (++errorCount) << "]: Can not find main function\n";
	if(errorCount)
	{
		cout << "Found " << errorCount << " errors.\n";
		cout << "\nFailed\n";
		llvmout << "\nERROR\n";
	}
	else
		cout << "\nOK\n";
}

Declaration *handleDec(specNode *spec, varNode *v, vector<Expr*>* init )
{
	Symbol *symP = SBT.addSym(spec,v);
	if(symP == NULL) printErrorMessage(yylineno,v->idName,"Multiple definitions.");
	Declaration *decP;

	decP = new Declaration();
	decP->s = symP;
	decP->inits = *init;
	delete init;
	return decP;
}
StTypeSym *handleStSpec( string *nm, vector<Definition*>* defs )
{
	vector<Symbol*> members;
	VarDef *vdefP;
	StTypeSym *sttP;

	//cout << "##  In handleStSpec()\n";
	for( int i = 0; i < defs->size(); ++i )
	{
		//cout << "##    Handling the " << i << "th definition.\n";

		vdefP = dynamic_cast<VarDef*>(defs->at(i));
		if(vdefP == NULL)
		{printErrorMessage(yylineno,*nm,"Undefined error");return NULL;} 
		members.insert(members.end(),vdefP->getSyms().begin(),vdefP->getSyms().end());
		delete defs->at(i);
		//cout << "##    Finished\n";
	}
	if( (sttP = SBT.addStType(*nm,members)) == NULL )
		printErrorMessage(yylineno,*nm,"Multiple definitions.");

	defs->clear();
	delete nm;
	delete defs;
	//cout << "##  A VarDef is handled\n";
	return sttP;
}
StTypeSym *handleStSpec(string *nm)
{
	Symbol *s = SBT.findSym(*nm,true);
	if( s == NULL )
	{
		printErrorMessage(yylineno,*nm,"Undefined symbol");
		return NULL;
	}
	else if( s->getType() != STTYPE ) 
	{
		printErrorMessage(yylineno,*nm,"Not struct-type");
		return NULL;
	}
	else
		return static_cast<StTypeSym*>(s);
	delete nm;
}
FuncSym *handleFunc(string *nm,vector<Symbol*>* paras)
{
	FuncSym *fP = SBT.addFunc(*nm,*paras);
	if( fP == NULL ) printErrorMessage(yylineno,*nm,"Multiple definitions of a function");
	delete nm;
	delete paras;
	return fP;
}
Symbol *handleAddSym(specNode *s, varNode *v)
{
	Symbol *symP = SBT.addSym(s,v);
	if( symP == NULL ) printErrorMessage(yylineno,v->idName,"Multiple definitions.");
	return symP;
}

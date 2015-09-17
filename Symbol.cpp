/*
version_2
Symbol.cpp:
	This file defines the symbol table class and the symbols used in symboltable.
*/
#include "Symbol.h"
#include "com.h"
using namespace std;

SymbolTable SBT;
string genAddress(string name, int id, bool glo )
{
	char buffer[12];
	sprintf(buffer, ".%d", id);
	if( glo )
		return "@"+name.substr(0,10)+buffer;
	else 
		return "%"+name.substr(0,10)+buffer;
}

/************************************************************************/
/*                             Symbol                                   */
/************************************************************************/
int Symbol::symCount = 0;
Symbol::Symbol(string nm /* = "" */, SymbolType ty /* = NONE */)
	: name(nm),type(ty),glo(false),init(false)
{
	idNum = symCount++;
}

void Symbol::setGlobal()
{
	glo = true;
	init = true;
	if(!memAdd.empty()) memAdd[0] = '@';
	else
		memAdd = genAddress(name,idNum,glo);
}
/************************************************************************/
/*                             IntSym                                   */
/************************************************************************/
IntSym::IntSym(string nm /* = "" */)
	:Symbol(nm,INTSYM)
{
	memAdd = genAddress(nm,idNum,glo);
}
/************************************************************************/
/*                              StTypeSym                               */
/************************************************************************/
StTypeSym::StTypeSym( string nm, vector<Symbol*> &m )
	: Symbol(nm,STTYPE)
{
	members = m;
	memAdd = genAddress(nm,idNum,glo);
}

int StTypeSym::findMemIndex(string s)
{
	for( int i = 0; i < members.size(); ++i )
		if( members[i]->getName() == s ) return i;
	return NOTFOUND;
}

int StTypeSym::decCodeGen()
{
	llvmout << getAddress() << " = type { ";
	for( int i = 0; i < members.size(); ++i )
		llvmout << (i==0?" ":", ") << "i32";
	llvmout << " }\n";
	return OK;
}

/************************************************************************/
/*                               FuncSym                                */
/************************************************************************/
FuncSym::FuncSym( string nm, vector<Symbol*> &arg )
	:Symbol(nm,FUNCSYM)
{
	argList = arg;
	glo = true;
	memAdd = "@"+nm;
}

Symbol *FuncSym::getArgSym( string s )
{
	for( int i = 0; i < argList.size(); ++i )
		if(argList[i]->getName() == s ) return argList[i];
	return NULL;
}
/************************************************************************/
/*                                 StSym                                */
/************************************************************************/
StSym::StSym( string nm /* = "" */, StTypeSym*stt /* = NULL */ )
	:Symbol(nm,STRUCTSYM), stType(stt)
{
	memAdd = genAddress(nm,idNum,glo);
}
/************************************************************************/
/*                                 ArraySym                             */
/************************************************************************/
ArraySym::ArraySym( string nm, SymbolType ty, vector<int> *l )
	:Symbol(nm,ty)
{
	lengthList = *l;
	memAdd = genAddress(nm,idNum,glo);
}
/************************************************************************/
/*                                  SymbolTable                         */
/************************************************************************/
SymbolTable::SymbolTable()
	:instantNum("",INSTANT)
{
	envStack.push_back(Environment());
}

IntSym *SymbolTable::addInt(string nm)
{
	//cout << "##  In SBT::addInt()\n";
	//cout << "##    Add symbol: " << nm << '\n';
	intList.push_back(IntSym(nm));
	envStack.back().push_back(&intList.back());
	if( envStack.size() == 1) 
		intList.back().setGlobal();
	return &intList.back();
}

StSym *SymbolTable::addSt( string nm, StTypeSym *stt )
{
	//cout << "##  In SBT::addSt()\n";
	//cout << "##    Add symbol: " << nm << '\n';
	stList.push_back(StSym(nm,stt));
	envStack.back().push_back(&stList.back());
	if( envStack.size() == 1)
		stList.back().setGlobal();
	return &stList.back();
}

IntArray *SymbolTable::addIntArray(string nm, vector<int> *i )
{
	//cout << "##  In SBT::addIntArray()\n";
	//cout << "##    Add symbol: " << nm << '\n';
	intArrayList.push_back(IntArray(nm,i));
	envStack.back().push_back(&intArrayList.back());
	if( envStack.size() == 1)
		intArrayList.back().setGlobal();
	return &intArrayList.back();
}

StArray *SymbolTable::addStArray(string nm, vector<int> *i, StTypeSym *stt)
{
	//cout << "##  In SBT::addStArray()\n";
	//cout << "##    Add symbol: " << nm << '\n';
	stArrayList.push_back(StArray(nm,i, stt));
	envStack.back().push_back(&stArrayList.back());
	if( envStack.size() == 1)
		stArrayList.back().setGlobal();
	return &stArrayList.back();
}

StTypeSym *SymbolTable::addStType(string nm, vector<Symbol*> &m)
{
	//cout << "##  In SBT::addStType()\n";
	//cout << "##    Add symbol: " << nm << '\n';
	if( findSym(nm,false) != NULL )
		return NULL;

	stTypeList.push_back(StTypeSym(nm,m));
	envStack.back().push_back(&stTypeList.back());
	return &stTypeList.back();
}
Symbol *SymbolTable::addSym(specNode *spec, varNode *var)
{
	if( findSym(var->idName,false) != NULL )
		return NULL;
	else if( spec->type == INTSYM && var->indexList.size() == 0 )
		return addInt(var->idName);
	else if( spec->type == INTSYM && var->indexList.size() > 0 )
		return addIntArray(var->idName,&(var->indexList));
	else if( spec->type == STRUCTSYM && var->indexList.size() == 0 )
		return addSt(var->idName,spec->stType);
	else if( spec->type == STRUCTSYM && var->indexList.size() > 0 )
		return addStArray(var->idName,&(var->indexList), spec->stType);
	else return NULL;
}
FuncSym *SymbolTable::addFunc(string nm, vector<Symbol*> &arg)
{
	//Function is always global
	if( findFuncSym(nm) != NULL) return NULL;
	funcList.push_back(FuncSym(nm,arg));
	envStack.front().push_back(&funcList.back());
	return &funcList.back();
}

IntSym *SymbolTable::addTempInt(string nm)
{
	intList.push_back(IntSym(nm));
	intList.back().setAddress("");//Temp integer has no address 
	intList.back().setInit();
	return &intList.back();
}
StSym *SymbolTable::addTempSt( string nm, StTypeSym *stt )
{
	stList.push_back(StSym(nm,stt));
	stList.back().setAddress(nm);
	return &stList.back();
}
IntArray *SymbolTable::addTempIA( string nm, vector<int> *i )
{
	intArrayList.push_back(IntArray(nm,i));
	intArrayList.back().setAddress(nm);
	return &intArrayList.back();
}
StArray *SymbolTable::addTempSA( string nm, vector<int> *i, StTypeSym *stt)
{
	stArrayList.push_back(StArray(nm,i,stt));
	stArrayList.back().setAddress(nm);
	return &stArrayList.back();
}
void SymbolTable::newEnv()
{
	envStack.push_back(Environment());
}

bool SymbolTable::leaveEnv()
{
	if(envStack.empty())
		return false;
	envStack.pop_back();
	return true;
}

Symbol *SymbolTable::findSym( string nm, bool backCheck)
{
	int i,j;
	int x = envStack.back().size();
	int y = envStack.size();
	for( i = envStack.back().size()-1; i >= 0; --i )
	{
		if( envStack.back()[i]->getName() == nm ) 
			return envStack.back().at(i);
	}
	if( backCheck == true )
		for( i = envStack.size()-2; i >= 0; --i )
			for( j = envStack[i].size()-1; j >= 0; --j )
				if((envStack[i][j]->getName() )== nm ) return envStack[i][j];
	return NULL;
}

FuncSym* SymbolTable::findFuncSym( string nm )
{
	list<FuncSym>::iterator i = funcList.begin();
	for(; i != funcList.end();++i)
		if(i->getName() == nm ) return &(*i);
	return NULL;
}

int SymbolTable::allStDecGen()
{
	list<StTypeSym>::iterator i;
	for( i = stTypeList.begin(); i != stTypeList.end(); ++i )
		i->decCodeGen();
	return OK;
}

//$$  Need modify
bool sameType( Symbol *s, Symbol *t )
{
	if( s->getType() != t->getType() )
	{
		if( (s->getType() == INTSYM && t->getType() == INSTANT) ||
			(t->getType() == INTSYM && s->getType() == INSTANT))
			return true;
		else return false;
	}
	switch(s->getType())
	{
	case INTSYM: return true;
	case STRUCTSYM:
		return (static_cast<StSym*>(s)->getStType() == static_cast<StSym*>(s)->getStType());
	case INTARRAY: return true;
	case STARRAY:
		return (static_cast<StArray*>(s)->getStType() == static_cast<StArray*>(s)->getStType());
	default:
		return true;
	}
}
%{
/*
myYacc.y
	This file deal with the definition of the small C grammar
*/
#include "com.h"
#include "ANTNode.h"
#include "Print.h"
#include <vector>

extern void InitInput();
extern int yylineno;
extern int errorCount;
extern char *yytext;

int parseOK = ERROR;
extern int yylex(void);
void yyerror(const char *s){printErrorMessage(yylineno,"Parse error",s);}
//extern int initInput();

specNode spec;
varNode var;
std::vector<Definition*> extdefs;
%}
%union {
  std::string *text;
  int value;
  std::vector<Definition*>* extdefList;
  std::vector<Declaration*>* decList;//DECS
  std::vector<Definition*>* defList;//DEFS
  std::vector<Symbol*>* symList;
  std::vector<Stmt*>* stmtList;
  std::vector<Expr*>* exprList;

  Definition *defP;
  Declaration *decP;
  specNode *specP;
  varNode *varP;
  FuncSym *funcP;
  Expr *exprP;
  Symbol *symP;
  StTypeSym *sttP;
  Stmt *stmtP;
  StmtBlock *stmtBP;
}

%type <value> PROGRAM
%type <extdefList> EXTDEFS
%type <defP> EXTDEF 
%type <decList> EXTVARS DECS
%type <specP> SPEC
%type <sttP> STSPEC 
%type <varP> VAR 
%type <funcP> FUNC
%type <text> OPTTAG
%type <symList> PARAS 
%type <symP> PARA 
%type <stmtP> STMT ESTMT
%type <stmtBP> STMTBLOCK
%type <decP> DEC
%type <defP> DEF
%type <defList> DEFS
%type <stmtList> STMTS
%type <exprList> INIT ARGS
%type <exprP> EXP
%token <value> INTEGER 
%token <text>ID
%token <value> STRUCT TYPE RETURN IF ELSE BREAK CONT FOR WRITE READ
%token <value> SEMI COMMA LC RC 
%right <value> AN ADDA SUBA MULA DIVA BANDA BXORA BORA SLA SRA
%left  <value> OR
%left  <value> AND
%left  <value> BOR
%left <value> BXOR
%left <value> BAND
%left <value> EQ NE
%left <value> GT GE LT LE
%left <value> SL SR
%left <value> ADD SUB
%left <value> MUL DIV MOD
%right <value> NOT BNOT UADD USUB
%left <value> LB RB LP RP DOT


%%

PROGRAM:
	EXTDEFS {parseOK = OK;}
	;
EXTDEFS:
	EXTDEFS EXTDEF  {$1->push_back($2); $$ = $1;}
	| { $$ = &extdefs;}
	;
EXTDEF:
	SPEC EXTVARS SEMI {$$ = new ExtVarDef($2);}
	|SPEC FUNC STMTBLOCK {$$ = new ExtFuncDef($2,$3);SBT.leaveEnv();}
	;
EXTVARS:
	DECS {$$ = $1;}
	| {$$ = NULL;}
	;
SPEC:
	TYPE { spec.type = INTSYM; spec.stType = NULL; $$ = &spec;}
	|STSPEC {spec.type = STRUCTSYM; spec.stType = $1; $$ = &spec;}
	;
STSPEC:
	STRUCT OPTTAG LC {SBT.newEnv();} DEFS RC {
			SBT.leaveEnv();
			$$ = handleStSpec($2, $5);
			 }
	|STRUCT ID { $$ = handleStSpec($2); }
	;
OPTTAG:
	ID {$$ = $1;}
	| { $$ = new std::string(""); }
	;
VAR:
	ID {var.idName = *$1;var.indexList.clear();$$ = &var; delete $1;}
	|VAR LB INTEGER RB {$1->indexList.push_back($3);$$ = $1;}
	;
FUNC:
	ID LP {SBT.newEnv();} PARAS RP { $$ = handleFunc($1,$4); }
	| ID LP RP { $$ = handleFunc($1);}
	;
PARAS:
	PARAS COMMA PARA {$1->push_back($3); $$ = $1;}
	|PARA { $$ = new std::vector<Symbol*>(); $$->push_back($1);}
	;
PARA:
	SPEC VAR { $$ = handleAddSym($1,$2);}
	;
STMTBLOCK:
	LC {SBT.newEnv();} DEFS STMTS RC {SBT.leaveEnv();$$ = new StmtBlock($3,$4);}
	;
STMTS:
	STMTS STMT { $1->push_back($2); $$ = $1; }
	| { $$ = new std::vector<Stmt*>(); }
	;
STMT:
	EXP SEMI { $$ = new ExprStmt($1);}
	|STMTBLOCK {$$ = $1;}
	|RETURN EXP SEMI { $$ = new RetStmt($2);  }
	|IF LP EXP RP STMT ESTMT { $$ = new IfStmt($3, $5, $6);}
	|FOR LP EXP SEMI EXP SEMI EXP RP STMT { $$ = new ForStmt($3,$5,$7,$9);}
	|CONT SEMI { $$ = new ContStmt();}
	|BREAK SEMI { $$ = new BreakStmt(); }
	|WRITE LP EXP RP SEMI {$$ = new WriteStmt($3);}
	|READ LP EXP RP SEMI { $$ = new ReadStmt($3);}
	;
ESTMT: 
	ELSE STMT { $$ = $2; }
	| { $$ = NULL;}
	;
DEFS:
	DEFS DEF { $1->push_back($2); $$ = $1; }
	| { $$ = new std::vector<Definition*>(); }
	;
DEF:
	SPEC DECS SEMI { $$ = new VarDef($2); }
	;
DECS:
	DECS COMMA DEC { $1->push_back($3); $$ = $1;}
	|DEC {$$ = new std::vector<Declaration*>(); $$->push_back($1);}
	;
DEC:
	VAR { $$ = handleDec(&spec,$1);}
	|VAR AN INIT { $$ = handleDec(&spec,$1,$3);}
	;
INIT:
	EXP {$$ = new std::vector<Expr*>(); $$->push_back($1);}
	|LC ARGS RC { $$ = $2;}
	;
EXP:
	EXP MUL EXP { $$ = new BinaryOp( $1, $3, MUL);  }
	|EXP DIV EXP { $$ = new BinaryOp( $1, $3, DIV);  }
	|EXP MOD EXP { $$ = new BinaryOp( $1, $3, MOD);  }
	|EXP ADD EXP { $$ = new BinaryOp( $1, $3, ADD);  }
	|EXP SUB EXP { $$ = new BinaryOp( $1, $3, SUB);  }
	|EXP SL EXP { $$ = new BinaryOp( $1, $3, SL);  }
	|EXP SR EXP { $$ = new BinaryOp( $1, $3, SR);  }
	|EXP GT EXP { $$ = new BinaryOp( $1, $3, GT); }
	|EXP LT EXP { $$ = new BinaryOp( $1, $3, LT);  }
	|EXP GE EXP { $$ = new BinaryOp( $1, $3, GE);  }
	|EXP LE EXP { $$ = new BinaryOp( $1, $3, LE);  }
	|EXP EQ EXP { $$ = new BinaryOp( $1, $3, EQ);  }
	|EXP NE EXP { $$ = new BinaryOp( $1, $3, NE);  }
	|EXP BAND EXP { $$ = new BinaryOp( $1, $3, BAND); }
	|EXP BXOR EXP { $$ = new BinaryOp( $1, $3, BXOR);  }
	|EXP BOR EXP { $$ = new BinaryOp( $1, $3, BOR);  }
	|EXP AND EXP { $$ = new BinaryOp( $1, $3, AND);  }
	|EXP OR EXP { $$ = new BinaryOp( $1, $3, OR); }
	|EXP AN EXP { $$ = new BinaryOp( $1, $3, AN);}
	|EXP ADDA EXP {$$ = new BinaryOp( $1, $3, ADDA);}
	|EXP SUBA EXP {$$ = new BinaryOp( $1, $3, SUBA);}
	|EXP MULA EXP { $$ = new BinaryOp( $1, $3, MULA);  }
	|EXP DIVA EXP { $$ = new BinaryOp( $1, $3, DIVA); }
	|EXP BANDA EXP { $$ = new BinaryOp( $1, $3, BAND);  }
	|EXP BXORA EXP { $$ = new BinaryOp( $1, $3, BXORA);  }
	|EXP BORA EXP { $$ = new BinaryOp( $1, $3, BORA);  }
	|EXP SLA EXP { $$ = new BinaryOp( $1, $3, SLA);  }
	|EXP SRA EXP { $$ = new BinaryOp( $1, $3, SRA);  }
	|SUB EXP %prec UADD  { $$ = new UnaryOp($2,SUB); }
	|NOT EXP  { $$ = new UnaryOp($2,NOT);  }
	|BNOT EXP { $$ = new UnaryOp($2,BNOT);  }
	|UADD EXP { $$ = new UnaryOp($2,UADD);  }
	|USUB EXP { $$ = new UnaryOp($2,USUB);  }
	|LP EXP RP  { $$ = $2; }
	|ID LP ARGS RP  { $$ = new FuncCall($1,$3); }
	|EXP LB EXP RB { $$ = new ArrayIndex($1,$3); }
    |ID { $$ = new VariableExp($1);}
	|EXP DOT ID  { $$ = new StMember($1,$3);}
	|INTEGER  { $$ = new InstantNum($1); }
	/*
	We give this production the same precedence to solve some RS conflicts;
	When conflicts occur between shitfting '-' and reducing as "EXP -> empty",
	we choose shifting according to the lower precedence of this production.
	*/
	| %prec OR { $$ = NULL; }
	;
ARGS:
	ARGS COMMA EXP { $1->push_back($3); $$ = $1; }
	|EXP { $$ = new std::vector<Expr*>(); $$->push_back($1); }
	;

%%


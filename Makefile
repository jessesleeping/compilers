CCC = g++
CCFLAGS= -O2
LEX = flex
LFLAGS= -8     
YACC= bison 
YFLAGS= -dy

scc: y.tab.o lex.yy.o ANTNode.o Symbol.o Print.o main.o
	${CCC} ${CCFLAGS} y.tab.o lex.yy.o ANTNode.o Symbol.o Print.o main.o -o scc -lfl

ANTNode.o: ANTNode.cpp
	${CCC} -c ANTNode.cpp 

Print.o : Print.cpp
	${CCC} -c Print.cpp

Symbol.o: Symbol.cpp
	${CCC} -c Symbol.cpp

y.tab.o: myYacc.y
	${YACC} ${YFLAGS} myYacc.y
	${CCC} ${CCFLAGS} y.tab.c -c 

lex.yy.o: myFlex.l
	${LEX} $(LFLAGS) myFlex.l
	${CCC} ${CCFLAGS} lex.yy.c -c 

main.o: main.cpp
	${CCC} -c main.cpp

clean:
	rm -f lex.yy.* y.tab.* *.o

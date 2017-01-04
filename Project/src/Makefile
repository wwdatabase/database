LEX = flex
YACC = bison
CC = g++

HEADER = bufmanager/BufPageManager.h bufmanager/FindReplace.h \
fileio/FileManager.h fileio/FileTable.h \
utils/compare.h utils/MyBitMap.h utils/MyHashMap.h utils/MyLinkList.h utils/pagedef.h \
rcdmana/transfer.h rcdmana/rm.h \
sysmana/SysMana.h \
IndexModule/BPlusTree.h \
qrylang/ql_manager.h \
Attr.h CondEntry.h Condition.h Expr.h SchemaEntry.h Schema.h SemValue.h Value.h \
Parser.tab.h \
OrderPack.h

SRC = SemValue.cpp \
Parser.tab.c lex.yy.c \
OrderPack.cpp

OBJ = SemValue.o \
Parser.tab.o lex.yy.o \
OrderPack.o

TARGET = ./nineSQL
LEXTARGET = lex.yy.c
YACCTARGET = Parser.tab.c Parser.tab.h

all : $(LEXTARGET) $(YACCTARGET) $(OBJ) $(TARGET)

$(LEXTARGET) : Lexer.l
	$(LEX) Lexer.l

$(YACCTARGET) : Parser.y
	$(YACC) -d Parser.y

$(OBJ) : $(SRC) $(HEADER)
	$(CC) -c $(SRC)

$(TARGET) : $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

clean :
	rm -rf $(LEXTARGET)
	rm -rf $(YACCTARGET)
	rm -rf $(OBJ)
	rm -rf $(TARGET)

%{
    #include <cstdio>
    #include <cstdlib>
    #include <iostream>
    #include <unistd.h>
    #include "SemValue.h"
    #include "OrderPack.h"
    #include "sysmana/SysMana.h"
    #define YYSTYPE SemValue
    void yyerror(const char*);
    void prompt();
    int yylex(void);
    extern FILE *yyin;
    bool isInterp = true;
    bool isTuo = false;
    using namespace std;

    DBT *filemanager = NULL;
    unsigned char h[61];
%}

%token INTEGER IDENTIFIER LITERAL
%token EXIT
%token CREATE DB DBS DROP USE SHOW TB TBS DESC
%token NOT IS NUL IN PRIMARY FOREIGN KEY CHECK REFER
%token INSERT INTO VALUES DELETE
%token WHERE UPDATE SET SELECT FROM LIKE
%token AND OR SUM AVG MAX MIN
%token INT CHAR VCHAR
%token LTE GTE NEQ

%%

Program         :   Program Stmt
                |   %empty
                ;

Stmt            :   error ';'
                    {
                        if (isInterp) std::cout << "\n\tSyntax Error " << std::endl;
                        else std::cout << "Syntax Error" << std::endl;
                        prompt();
                    }
                |   EXIT ';'
                    {
                        delete filemanager;
                        exit(0);
                    }
                |   CREATE DB IDENTIFIER ';'
                    {
                        OrderPack pack(OrderPack::CREATE_DB);
                        pack.dbname = $3.id;
                        pack.process();
                        prompt();
                    }
                |   DROP DB IDENTIFIER ';'
                    {
                        OrderPack pack(OrderPack::DROP_DB);
                        pack.dbname = $3.id;
                        pack.process();
                        prompt();
                    }
                |   USE IDENTIFIER ';'
                    {
                        OrderPack pack(OrderPack::USE);
                        pack.dbname = $2.id;
                        pack.process();
                        prompt();
                    }
                |   SHOW DBS ';'
                    {
                        OrderPack pack(OrderPack::SHOWDBS);
                        pack.process();
                        prompt();
                    }
                |   SHOW TBS ';'
                    {
                        OrderPack pack(OrderPack::SHOWTBS);
                        pack.process();
                        prompt();
                    }
                |   CREATE TB IDENTIFIER '(' AttrDefList ')' ';'
                    {
                        OrderPack pack(OrderPack::CREATE_TB);
                        pack.tbname = $3.id;
                        pack.schema = $5.schema;
                        pack.process();
                        prompt();
                    }
                |   DROP TB IDENTIFIER ';'
                    {
                        OrderPack pack(OrderPack::DROP_TB);
                        pack.tbname = $3.id;
                        pack.process();
                        prompt();
                    }
                |   DESC IDENTIFIER ';'
                    {
                        OrderPack pack(OrderPack::DESC);
                        pack.tbname = $2.id;
                        pack.process();
                        prompt();
                    }
                |   INSERT INTO IDENTIFIER VALUES ValueListList ';'
                    {
                        OrderPack pack(OrderPack::INSERT);
                        pack.tbname = $3.id;
                        pack.valuesList = $5.valuesList;
                        pack.process();
                        prompt();
                    }
                |   DELETE FROM IDENTIFIER WhereClause ';'
                    {
                        OrderPack pack(OrderPack::DELETE);
                        pack.tbname = $3.id;
                        pack.condition = $4.condition;
                        pack.process();
                        prompt();
                    }
                |   SELECT AttrList FROM TableList WhereClause ';'
                    {
                        OrderPack pack(OrderPack::SELECT);
                        pack.allAttrs = $2.allAttrs;
                        pack.attrs = $2.attrList;
                        pack.tables = $4.tableList;
                        pack.condition = $5.condition;
                        pack.process();
                        prompt();
                    }
                |   UPDATE IDENTIFIER SET SetList WhereClause ';'
                    {
                        OrderPack pack(OrderPack::UPDATE);
                        pack.tbname = $2.id;
                        pack.updateAttr = $4.updateAttr;
                        pack.updateValue = $4.updateValue;
                        pack.condition = $5.condition;
                        pack.process();
                        prompt();
                    }
                ;

WhereClause     :   %empty
                    {
                        $$.condition.clear();
                    }
                |   WHERE CondList
                    {
                        $$.condition = $2.condition;
                    }
                ;

AttrDefList     :   AttrDefList ',' AttrDefItem
                    {
                        $$.schema.process($3.schemaEntry);
                    }
                |   AttrDefItem
                    {
                        $$.schema = Schema();
                        $$.schema.process($1.schemaEntry);
                    }
                ;

AttrDefItem     :   IDENTIFIER Type '(' INTEGER ')'
                    {
                        $$.schemaEntry = SchemaEntry($1.id, $2.datatype, $4.length, 0);
                        $$.schemaEntry.entrykind = SchemaEntry::NORMAL;
                    }
                |   IDENTIFIER Type '(' INTEGER ')' NOT NUL
                    {
                        $$.schemaEntry = SchemaEntry($1.id, $2.datatype, $4.length, 1);
                        $$.schemaEntry.entrykind = SchemaEntry::NORMAL;
                    }
                |   PRIMARY KEY '(' IDENTIFIER ')'
                    {
                        $$.schemaEntry.primaryKey = $4.id;
                        $$.schemaEntry.entrykind = SchemaEntry::PRIMARY;
                    }
                |   CHECK '(' CondList ')'
                    {
                        $$.schemaEntry.constrain = $3.condition;
                        $$.schemaEntry.entrykind = SchemaEntry::CHECK;
                    }
                |   FOREIGN KEY '(' IDENTIFIER ')' REFER IDENTIFIER '(' IDENTIFIER ')'
                    {
                        $$.schemaEntry.foreignKey = $4.id;
                        $$.schemaEntry.foreignAttr = Attr($7.id, $9.id);
                        $$.schemaEntry.entrykind = SchemaEntry::FOREIGN;
                    }
                ;

Type            :   INT
                    {
                        $$.datatype = "int";
                    }
                |   CHAR
                    {
                        $$.datatype = "char";
                    }
                |   VCHAR
                    {
                        $$.datatype = "varchar";
                    }
                ;

ValueListList   :   '(' ValueList ')'
                    {
                        $$.valuesList.clear();
                        $$.valuesList.push_back($2.values);
                    }
                |   ValueListList ',' '(' ValueList ')'
                    {
                        $$.valuesList.push_back($4.values);
                    }
                ;

ValueList       :   ValueList ',' ValueItem
                    {
                        $$.values.push_back($3.value);
                    }
                |   ValueItem
                    {
                        $$.values.clear();
                        $$.values.push_back($1.value);
                    }
                ;

ValueItem       :   INTEGER
                    {
                        $$.value = Value(0, $1.length, "");
                    }
                |   LITERAL
                    {
                        $$.value = Value(1, 0, $1.literal);
                    }
                |   NUL
                    {
                        $$.value = Value(2, 0, "");
                    }
                ;

SetList         :   IDENTIFIER '=' ValueItem
                    {
                        $$.updateAttr.clear();
                        $$.updateValue.clear();
                        $$.updateAttr.push_back($1.id);
                        $$.updateValue.push_back($3.value);
                    }
                |   SetList ',' IDENTIFIER '=' ValueItem
                    {
                        $$.updateAttr.push_back($3.id);
                        $$.updateValue.push_back($5.value);
                    }
                ;

CondList        :   CondList AND Cond
                    {
                        $$.condition.add(Condition::AND, $3.condEntry);
                    }
                |   CondList OR Cond
                    {
                        $$.condition.add(Condition::OR, $3.condEntry);
                    }
                |   Cond
                    {
                        $$.condition = Condition($1.condEntry);
                    }
                ;

Cond            :   Expr '=' Expr
                    {
                        $$.condEntry = CondEntry(CondEntry::EQUAL, $1.expr, $3.expr);
                    }
                |   Expr '>' Expr
                    {
                        $$.condEntry = CondEntry(CondEntry::GREATER, $1.expr, $3.expr);
                    }
                |   Expr '<' Expr
                    {
                        $$.condEntry = CondEntry(CondEntry::LESS, $1.expr, $3.expr);
                    }
                |   Expr LTE Expr
                    {
                        $$.condEntry = CondEntry(CondEntry::LESSE, $1.expr, $3.expr);
                    }
                |   Expr GTE Expr
                    {
                        $$.condEntry = CondEntry(CondEntry::GREATERE, $1.expr, $3.expr);
                    }
                |   Expr NEQ Expr
                    {
                        $$.condEntry = CondEntry(CondEntry::NEQUAL, $1.expr, $3.expr);
                    }
                |   Attr IS NUL
                    {
                        $$.condEntry = CondEntry();
                        $$.condEntry.op = CondEntry::IS;
                        $$.condEntry.left = Expr(Expr::ATTR);
                        $$.condEntry.left.attr = $1.attr;
                    }
                |   Attr LIKE LITERAL
                    {
                        $$.condEntry = CondEntry();
                        $$.condEntry.op = CondEntry::LIKE;
                        $$.condEntry.left = Expr(Expr::ATTR);
                        $$.condEntry.left.attr = $1.attr;
                        $$.condEntry.mode = $3.literal;
                    }
                |   Attr IN '(' ValueList ')'
                    {
                        $$.condEntry = CondEntry();
                        $$.condEntry.op = CondEntry::IN;
                        $$.condEntry.left = Expr(Expr::ATTR);
                        $$.condEntry.left.attr = $1.attr;
                        $$.condEntry.values = $4.values;
                    }
                ;

Expr            :   Attr
                    {
                        $$.expr = Expr(Expr::ATTR);
                        $$.expr.attr = $1.attr;
                    }
                |   INTEGER
                    {
                        $$.expr = Expr(Expr::INTEGER);
                        $$.expr.integer = $1.length;
                    }
                |   LITERAL
                    {
                        $$.expr = Expr(Expr::LITERAL);
                        $$.expr.literal = $1.literal;
                    }
                ;

AttrList        :   '*'
                    {
                        $$.allAttrs = true;
                    }
                |   AttrList ',' AttrAggr
                    {
                        $$.allAttrs = false;
                        $$.attrList.push_back($3.attr);
                    }
                |   AttrAggr
                    {
                        $$.allAttrs = false;
                        $$.attrList.clear();
                        $$.attrList.push_back($1.attr);
                    }
                ;

AttrAggr        :   Attr
                    {
                        $$.attr = $1.attr;
                        $$.attr.aggr = Attr::NONE;
                    }
                |   SUM '(' Attr ')'
                    {
                        $$.attr = $3.attr;
                        $$.attr.aggr = Attr::SUM;
                    }
                |   AVG '(' Attr ')'
                    {
                        $$.attr = $3.attr;
                        $$.attr.aggr = Attr::AVG;
                    }
                |   MAX '(' Attr ')'
                    {
                        $$.attr = $3.attr;
                        $$.attr.aggr = Attr::MAX;
                    }
                |   MIN '(' Attr ')'
                    {
                        $$.attr = $3.attr;
                        $$.attr.aggr = Attr::MIN;
                    }
                ;

Attr            :   IDENTIFIER
                    {
                        $$.attr = Attr("", $1.id);
                    }
                |   IDENTIFIER '.' IDENTIFIER
                    {
                        $$.attr = Attr($1.id, $3.id);
                    }
                ;

TableList       :   TableList ',' IDENTIFIER
                    {
                        $$.tableList.push_back($3.id);
                    }
                |   IDENTIFIER
                    {
                        $$.tableList.clear();
                        $$.tableList.push_back($1.id);
                    }
                ;

%%

void prompt()
{
    if (isInterp) std::cout << "sql>\n";
}

void yyerror(const char *s)
{
}

int main(int argc, char** argv)
{
    if (argc > 2)
    {
        std::cout << "Usage: main [filename]" << std::endl;
        return -1;
    }

    FILE *pFile = NULL;
    if (argc == 2)
    {
        pFile = fopen(argv[1], "r");
        if (!pFile)
        {
            std::cout << "Input File not Exist!" << std::endl;
            return -1;
        }
    }

    filemanager = new DBT();

    if (argc == 2)
    {
        isInterp = false;
        yyin = pFile;
        yyparse();
    }
    if (argc == 1)
    {
        isInterp = true;
        prompt();
        yyparse();
    }
    return 0;
}

    #ifndef _SEM_VALUE_H
#define _SEM_VALUE_H

#include <string>
#include <vector>
#include "Schema.h"
#include "SchemaEntry.h"
#include "Value.h"
#include "Condition.h"
#include "CondEntry.h"
#include "Expr.h"
#include "Attr.h"

using std::string;

class SemValue
{
public:
    int code;
    string id;
    string literal;
    string datatype;
    int length;
    Schema schema;
    SchemaEntry schemaEntry;
    std::vector<Value> values;
    std::vector< std::vector<Value> > valuesList;
    Value value;
    Condition condition;
    CondEntry condEntry;
    Expr expr;
    Attr attr;
    std::vector<Attr> attrList;
    bool allAttrs;
    std::vector<string> tableList;
    std::vector<std::string> updateAttr;
    std::vector<Value> updateValue;

    static int keyword(int);
};

#endif

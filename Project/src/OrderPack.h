#ifndef _ORDER_PACK_H
#define _ORDER_PACK_H

#include <string>
#include <vector>
#include "Schema.h"
#include "Value.h"
#include "Condition.h"
#include "Attr.h"

class OrderPack
{
public:
    enum OrderType {CREATE_DB, DROP_DB, USE, CREATE_TB, DROP_TB, SHOWDBS, SHOWTBS, DESC, INSERT, DELETE, UPDATE, SELECT};

    OrderType type;
    std::string dbname;
    std::string tbname;
    Schema schema;
    std::vector<Value> values;
    std::vector< std::vector<Value> > valuesList;
    bool allAttrs;
    std::vector<Attr> attrs;
    std::vector<string> tables;
    Condition condition;
    std::vector<std::string> updateAttr;
    std::vector<Value> updateValue;

    OrderPack(OrderType ot)
    {
        type = ot;
    }
    void process();
};

#endif

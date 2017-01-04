#ifndef _COND_ENTRY_H
#define _COND_ENTRY_H

#include <string>
#include <vector>
#include "Expr.h"
#include "Value.h"

class CondEntry
{
public:
    enum CondArithOp {EQUAL, GREATER, LESS, IS, LIKE, IN, LESSE, GREATERE, NEQUAL};
    CondEntry() {}
    CondEntry(CondArithOp o, Expr l, Expr r)
    {
        op = o;
        left = l;
        right = r;
    }
    CondArithOp op;
    Expr left, right;
    std::string mode;
    std::vector<Value> values;
};

#endif

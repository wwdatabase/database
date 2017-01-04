#ifndef _CONDITION_H
#define _CONDITION_H

#include "CondEntry.h"
#include <vector>

class Condition
{
public:
    enum CondLogicOp {AND, OR};
    Condition() {}
    Condition(CondEntry &entry)
    {
        operands.push_back(entry);
    }
    void add(CondLogicOp op, CondEntry &entry)
    {
        operands.push_back(entry);
        opcodes.push_back(op);
    }
    void clear()
    {
        operands.clear();
        opcodes.clear();
    }
    std::vector<CondEntry> operands;
    std::vector<CondLogicOp> opcodes;
};

#endif

#ifndef _VALUE_H
#define _VALUE_H

#include <string>

class Value
{
public:
    Value() {}
    Value(int t, int i, std::string s)
    {
        type = t;
        integer = i;
        literal = s;
    }
    int type;
    int integer;
    std::string literal;
};

#endif

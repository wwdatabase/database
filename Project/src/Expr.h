#ifndef _EXPR_H
#define _EXPR_H

#include <string>
#include "Attr.h"

class Expr
{
public:
    enum ExprType {ATTR, INTEGER, LITERAL};
    Expr(ExprType t = Expr::INTEGER)
    {
        type = t;
    }
    ExprType type;
    int integer;
    std::string literal;
    Attr attr;
};

#endif

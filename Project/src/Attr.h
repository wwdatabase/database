#ifndef _ATTR_H
#define _ATTR_H

#include <string>

using std::string;

class Attr
{
public:
    enum Aggr {NONE, SUM, AVG, MAX, MIN};
    Attr() {}
    Attr(string tb, string at)
    {
        tbname = tb;
        attrname = at;
    }
    string tbname;
    string attrname;
    Aggr aggr;
};

#endif

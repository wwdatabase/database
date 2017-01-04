#ifndef _SCHEMA_ENTRY_H
#define _SCHEMA_ENTRY_H

#include "Condition.h"
#include "Attr.h"
#include <string>

using std::string;

class SchemaEntry
{
public:
    enum EntryKind {NORMAL, PRIMARY, CHECK, FOREIGN};
    SchemaEntry() {}
    SchemaEntry(string& f, string& t, int len, int notN)
    {
        field = f;
        type = t;
        length = len;
        notNull = notN;
    }
    string field;
    string type;
    int length;
    bool notNull;
    bool isPrimary;
    bool isForeign;
    bool hasConstrain;
    EntryKind entrykind;
    string primaryKey;
    string foreignKey;
    Condition constrain;
    Attr foreignAttr;
};

#endif

#ifndef _SCHEMA_H
#define _SCHEMA_H

#include <vector>
#include <string>
#include "SchemaEntry.h"
#include "Condition.h"

class Schema
{
public:
    void process(SchemaEntry& entry)
    {
        switch (entry.entrykind)
        {
        case SchemaEntry::NORMAL :
            entry.isPrimary = false;
            entry.isForeign = false;
            entry.hasConstrain = false;
            entries.push_back(entry);
            break;
        case SchemaEntry::PRIMARY :
            for (int i = 0; i < size(); i++)
                if (entries[i].field == entry.primaryKey)
                {
                    entries[i].isPrimary = true;
                    break;
                }
            break;
        case SchemaEntry::CHECK :
            constrain = entry.constrain;
            break;
        case SchemaEntry::FOREIGN :
            for (int i = 0; i < size(); i++)
                if (entries[i].field == entry.foreignKey)
                {
                    entries[i].isForeign = true;
                    entries[i].foreignAttr = entry.foreignAttr;
                    break;
                }
            break;
        }
    }
    int size()
    {
        return entries.size();
    }
    std::vector<SchemaEntry> entries;
    Condition constrain;
};

#endif

#include <iostream>
#include "parser.h"

class QL_Manager {
 public:

    // Constructor
    QL_Manager (SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm);
    ~QL_Manager ();                         // Destructor

    RC Select (
        const vector<RelAttr> selAttrs,   // attrs in Select clause
        const vector<string> relations, // relations in From clause
        const vector<Condition> conditions);  
                                        // conditions in Where clause

    RC Insert (const char  *relName,    // relation to insert into
        int nValues,                    // # values to insert
        const Value values[]);          // values to insert

    RC Delete (const char *relName,     // relation to delete from
        int nConditions,      // # conditions in Where clause
        const Condition conditions[]);  // conditions in Where clause

    RC Update (const char *relName,     // relation to update
        const RelAttr &updAttr,   // attribute to update
        const int bIsValue,   // 0/1 if RHS of = is attribute/value
        const RelAttr &rhsRelAttr,      // attr on RHS of =
        const Value &rhsValue,          // value on RHS of =
        int   nConditions,        // # conditions in Where clause
        const Condition conditions[]);  // conditions in Where clause
};

using namespace std;

QL_Manager::QL_Manager (SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm) {}

QL_Manager::~QL_Manager() {}

RC QL_Manager::Select ( const vector<RelAttr> selAttrs, 
            const vector<char *> relations, 
            const vector<Condition> conditions) {
    
}

// 
// RC Select ( int nSelAttrs,
//             const RelAttr selAttrs[],
//             int nRelations,    
//             const char * const relations[],
//             int nConditions,   
//             const Condition conditions[]) {
//     
// }
// 
RC Insert ( const char  *relName, 
            int nValues,                
            const Value values[]) {
}

RC Delete ( const char *relName, 
            int nConditions,     
            const Condition conditions[]) {
}

RC Update ( const char *relName, 
            const RelAttr &updAttr, 
            const int bIsValue, 
            const RelAttr &rhsRelAttr,   
            const Value &rhsValue,        
            int   nConditions, 
            const Condition conditions[]) {
}


struct RelAttr {
  char *relName;     // relation name (may be NULL) 
  char *attrName;    // attribute name              
  friend ostream &operator<<(ostream &s, const RelAttr &ra);
};

struct Value {
  AttrType type;     // type of value               
  void     *data;    // value                       
  friend ostream &operator<<(ostream &s, const Value &v);
};

struct Condition {
  RelAttr lhsAttr;      // left-hand side attribute                     
  CompOp  op;           // comparison operator                          
  int     bRhsIsAttr;   // TRUE if right-hand side is an attribute
                        //   and not a value
  RelAttr rhsAttr;      // right-hand side attribute if bRhsIsAttr = TRUE
  Value   rhsValue;     // right-hand side value if bRhsIsAttr = FALSE
  friend ostream &operator<<(ostream &s, const Condition &c);
};

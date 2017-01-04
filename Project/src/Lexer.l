%{
    #include <cstdlib>
    #include <iostream>
    #include "SemValue.h"
    #define YYSTYPE SemValue
    #include "Parser.tab.h"
    void yyerror(const char*);
    std::string str;
    using namespace std;
    extern bool isInterp;
%}

NEWLINE     (\r|\n|\r\n)
DIGIT       ([0-9])
INTEGER     ({DIGIT}+)
IDENTIFIER  ([A-Za-z][_0-9A-Za-z]*)
OPERATOR    (","|"."|"*"|"("|")"|"="|"<"|">"|";")
WHITESPACE	([ \t]+)
EXIT        ("EXIT"|"exit")
LTE         ("<=")
GTE         (">=")
NEQ         ("<>")

CREATE      ("CREATE"|"create")
DB          ("DATABASE"|"database")
DBS         ("DATABASES"|"databases")
DROP        ("DROP"|"drop")
USE         ("USE"|"use")
SHOW        ("SHOW"|"show")
TBS         ("TABLES"|"tables")
TB          ("TABLE"|"table")
DESC        ("DESC"|"desc")
NOT         ("NOT"|"not")
IS          ("IS"|"is")
NULL        ("NULL"|"null")
IN          ("IN"|"in")
PRIMARY     ("PRIMARY"|"primary")
FOREIGN     ("FOREIGN"|"foreign")
KEY         ("KEY"|"key")
CHECK       ("CHECK"|"check")
REFER       ("REFERENCES"|"references")
INSERT      ("INSERT"|"insert")
INTO        ("INTO"|"into")
VALUES      ("VALUES"|"values")
DELETE      ("DELETE"|"delete")
WHERE       ("WHERE"|"where")
UPDATE      ("UPDATE"|"update")
SET         ("SET"|"set")
SELECT      ("SELECT"|"select")
FROM        ("FROM"|"from")
LIKE        ("LIKE"|"like")
AND         ("AND"|"and")
OR          ("OR"|"or")
SUM         ("SUM"|"sum")
AVG         ("AVG"|"avg")
MAX         ("MAX"|"max")
MIN         ("MIN"|"min")

INT         ("INT"|"int")
CHAR        ("CHAR"|"char")
VCHAR       ("VARCHAR"|"varchar")

%x SS
%x S

%%

{WHITESPACE}    { /* Just ignore */                     }
{NEWLINE}       { if (isInterp) std::cout << "\t";      }
{EXIT}          { return EXIT;                          }

{CREATE}        { return SemValue::keyword(CREATE);     }
{DB}            { return SemValue::keyword(DB);         }
{DBS}           { return SemValue::keyword(DBS);        }
{DROP}          { return SemValue::keyword(DROP);       }
{USE}           { return SemValue::keyword(USE);        }
{SHOW}          { return SemValue::keyword(SHOW);       }
{TB}            { return SemValue::keyword(TB);         }
{TBS}           { return SemValue::keyword(TBS);        }
{DESC}          { return SemValue::keyword(DESC);       }
{NOT}           { return SemValue::keyword(NOT);        }
{IS}            { return SemValue::keyword(IS);         }
{NULL}          { return SemValue::keyword(NUL);        }
{IN}            { return SemValue::keyword(IN);         }
{PRIMARY}       { return SemValue::keyword(PRIMARY);    }
{FOREIGN}       { return SemValue::keyword(FOREIGN);    }
{KEY}           { return SemValue::keyword(KEY);        }
{CHECK}         { return SemValue::keyword(CHECK);      }
{REFER}         { return SemValue::keyword(REFER);      }
{INSERT}        { return SemValue::keyword(INSERT);     }
{INTO}          { return SemValue::keyword(INTO);       }
{VALUES}        { return SemValue::keyword(VALUES);     }
{DELETE}        { return SemValue::keyword(DELETE);     }
{WHERE}         { return SemValue::keyword(WHERE);      }
{UPDATE}        { return SemValue::keyword(UPDATE);     }
{SET}           { return SemValue::keyword(SET);        }
{SELECT}        { return SemValue::keyword(SELECT);     }
{FROM}          { return SemValue::keyword(FROM);       }
{LIKE}          { return SemValue::keyword(LIKE);       }
{AND}           { return SemValue::keyword(AND);        }
{OR}            { return SemValue::keyword(OR);         }
{SUM}           { return SemValue::keyword(SUM);        }
{AVG}           { return SemValue::keyword(AVG);        }
{MAX}           { return SemValue::keyword(MAX);        }
{MIN}           { return SemValue::keyword(MIN);        }

{INT}           { return SemValue::keyword(INT);        }
{CHAR}          { return SemValue::keyword(CHAR);       }
{VCHAR}         { return SemValue::keyword(VCHAR);      }


{OPERATOR}      { return *yytext;                       }
{LTE}           { return SemValue::keyword(LTE);        }
{GTE}           { return SemValue::keyword(GTE);        }
{NEQ}           { return SemValue::keyword(NEQ);        }

{IDENTIFIER}    {
                    yylval = SemValue();
                    yylval.code = IDENTIFIER;
                    yylval.id = std::string(yytext);
                    return IDENTIFIER;
                }
{INTEGER}       {
                    yylval = SemValue();
                    yylval.code = INTEGER;
                    yylval.length = atoi(yytext);
                    return INTEGER;
                }

<INITIAL>\"     {
                    str = std::string();
                    BEGIN(SS);
                }
<INITIAL>\'     {
                    str = std::string();
                    BEGIN(S);
                }

<SS>\"          {
                    BEGIN(INITIAL);
                    yylval = SemValue();
                    yylval.code = LITERAL;
                    yylval.literal = str;
                    return LITERAL;
                }
<S>\'           {
                    BEGIN(INITIAL);
                    yylval = SemValue();
                    yylval.code = LITERAL;
                    yylval.literal = str;
                    return LITERAL;
                }
<SS>.           {
                    str += char(*yytext);
                }
<S>.            {
                    str += char(*yytext);
                }
.               {
                    return 256;
                }
%%

int yywrap(void)
{
    return 1;
}

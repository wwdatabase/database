/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_PARSER_TAB_H_INCLUDED
# define YY_YY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    INTEGER = 258,
    IDENTIFIER = 259,
    LITERAL = 260,
    EXIT = 261,
    CREATE = 262,
    DB = 263,
    DBS = 264,
    DROP = 265,
    USE = 266,
    SHOW = 267,
    TB = 268,
    TBS = 269,
    DESC = 270,
    NOT = 271,
    IS = 272,
    NUL = 273,
    IN = 274,
    PRIMARY = 275,
    FOREIGN = 276,
    KEY = 277,
    CHECK = 278,
    REFER = 279,
    INSERT = 280,
    INTO = 281,
    VALUES = 282,
    DELETE = 283,
    WHERE = 284,
    UPDATE = 285,
    SET = 286,
    SELECT = 287,
    FROM = 288,
    LIKE = 289,
    AND = 290,
    OR = 291,
    SUM = 292,
    AVG = 293,
    MAX = 294,
    MIN = 295,
    INT = 296,
    CHAR = 297,
    VCHAR = 298,
    LTE = 299,
    GTE = 300,
    NEQ = 301
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */

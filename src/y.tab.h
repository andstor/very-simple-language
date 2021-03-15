/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     RSHIFT = 258,
     LSHIFT = 259,
     UMINUS = 260,
     FUNC = 261,
     PRINT = 262,
     RETURN = 263,
     CONTINUE = 264,
     IF = 265,
     THEN = 266,
     ELSE = 267,
     WHILE = 268,
     DO = 269,
     OPENBLOCK = 270,
     CLOSEBLOCK = 271,
     ASSIGNMENT = 272,
     VAR = 273,
     NUMBER = 274,
     IDENTIFIER = 275,
     STRING = 276
   };
#endif
/* Tokens.  */
#define RSHIFT 258
#define LSHIFT 259
#define UMINUS 260
#define FUNC 261
#define PRINT 262
#define RETURN 263
#define CONTINUE 264
#define IF 265
#define THEN 266
#define ELSE 267
#define WHILE 268
#define DO 269
#define OPENBLOCK 270
#define CLOSEBLOCK 271
#define ASSIGNMENT 272
#define VAR 273
#define NUMBER 274
#define IDENTIFIER 275
#define STRING 276




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;


%{
#include <vslc.h>
%}

%option noyywrap
%option array
%option yylineno

WHITESPACE [\t\n\r\v\ ]
COMMENT \/\/[^\n]*\n
DIGIT [0-9]
LETTER [A-Za-z_]
ESCAPED \"([^\"\n]|\\\")*\"

%%

{WHITESPACE}+   {}
{COMMENT}+      {}
def             { return FUNC; }
begin           { return OPENBLOCK; }
end             { return CLOSEBLOCK; }
return          { return RETURN; }
print           { return PRINT; }
if              { return IF ; }
then            { return THEN; }
else            { return ELSE; }
while           { return WHILE; }
do              { return DO; }
continue        { return CONTINUE; }
var             { return VAR; }
:=              { return ASSIGNMENT; }
"<<"            { return LSHIFT; }              
">>"            { return RSHIFT; }
{DIGIT}+        { return NUMBER;  }
{ESCAPED}       { return STRING;  }
{LETTER}({LETTER}|{DIGIT})* { return IDENTIFIER; }
.               { return yytext[0]; }

%%

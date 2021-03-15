%{
#include <vslc.h>
#define DUMP_TOKENS

#define CREATE_NODE(node, type, data, level, ...) do { \
    node_init ( node = malloc(sizeof(node_t)), type, data, level, __VA_ARGS__); \
} while ( false )
%}

%left '|'
%left '^'
%left '&'
%left LSHIFT RSHIFT
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS
%right '~'
	//%expect 1

%token FUNC PRINT RETURN CONTINUE IF THEN ELSE WHILE DO OPENBLOCK CLOSEBLOCK ASSIGNMENT
%token VAR NUMBER IDENTIFIER STRING LSHIFT RSHIFT

%%
program : global_list {
            root = (node_t *) malloc ( sizeof(node_t) );
            node_init ( root, PROGRAM, NULL, 1, $1 );
          };
global_list : global             { CREATE_NODE ( $$, GLOBAL_LIST, NULL, 1, $1 ); }
            | global_list global { CREATE_NODE ( $$, GLOBAL_LIST, NULL, 2, $1, $2 ); }
            ;
global : function    { CREATE_NODE ( $$, GLOBAL, NULL, 1, $1); }
       | declaration { CREATE_NODE ( $$, GLOBAL, NULL, 1, $1 ); }
       ;
statement_list : statement                { CREATE_NODE ( $$, STATEMENT_LIST, NULL, 1, $1 );}
               | statement_list statement { CREATE_NODE ( $$, STATEMENT_LIST, NULL, 2, $1, $2 );}
               ;
print_list : print_item                { CREATE_NODE ( $$, PRINT_LIST, NULL, 1, $1 ); }
           | print_list ',' print_item { CREATE_NODE ( $$, PRINT_LIST, NULL, 2, $1, $3 );}
           ;
expression_list : expression                     { CREATE_NODE ( $$, EXPRESSION_LIST, NULL, 1, $1 ); }
                | expression_list ',' expression { CREATE_NODE ( $$, EXPRESSION_LIST, NULL, 2, $1, $3 ); }
                ;
variable_list : identifier                   { CREATE_NODE ( $$, VARIABLE_LIST, NULL, 1, $1 ); }
              | variable_list ',' identifier { CREATE_NODE ( $$, VARIABLE_LIST, NULL, 2, $1, $3 ); }
              ;
argument_list : expression_list { CREATE_NODE ( $$, ARGUMENT_LIST, NULL, 1, $1 ); }
              | /* ε */         { $$ = NULL; }
              ;
parameter_list : variable_list { CREATE_NODE ( $$, PARAMETER_LIST, NULL, 1, $1 ); }
               | /* ε */       { $$ = NULL; }
               ;
declaration_list : declaration                  { CREATE_NODE ( $$, DECLARATION_LIST, NULL, 1, $1 ); }
                 | declaration_list declaration { CREATE_NODE ( $$, DECLARATION_LIST, NULL, 2, $1, $2); }
                 ;
function : FUNC identifier '(' parameter_list ')' statement { CREATE_NODE ( $$, FUNCTION, NULL, 3, $2, $4, $6 ); };
statement : assignment_statement { CREATE_NODE ($$, STATEMENT, NULL, 1, $1); }
          | return_statement { CREATE_NODE ($$, STATEMENT, NULL, 1, $1); }
          | print_statement { CREATE_NODE ($$, STATEMENT, NULL, 1, $1); }
          | if_statement { CREATE_NODE ($$, STATEMENT, NULL, 1, $1); }
          | while_statement { CREATE_NODE ($$, STATEMENT, NULL, 1, $1); }
          | null_statement { CREATE_NODE ($$, STATEMENT, NULL, 1, $1); }
          | block { CREATE_NODE ($$, STATEMENT, NULL, 1, $1); }
          ;
block : OPENBLOCK declaration_list statement_list CLOSEBLOCK { CREATE_NODE ($$, BLOCK, NULL, 2, $2, $3); }
      | OPENBLOCK statement_list CLOSEBLOCK { CREATE_NODE ($$, BLOCK, NULL, 1, $2); }
      ;
assignment_statement : identifier ASSIGNMENT expression { CREATE_NODE ($$, ASSIGNMENT_STATEMENT, NULL, 2, $1, $3); };
return_statement : RETURN expression { CREATE_NODE ($$, RETURN_STATEMENT, NULL, 1, $2); };
print_statement : PRINT print_list { CREATE_NODE ($$, PRINT_STATEMENT, NULL, 1, $2); };
null_statement : CONTINUE { CREATE_NODE ($$, NULL_STATEMENT, NULL, 0, NULL); };
if_statement : IF relation THEN statement { CREATE_NODE ($$, IF_STATEMENT, NULL, 2, $2, $4); }
             | IF relation THEN statement ELSE statement { CREATE_NODE ($$, IF_STATEMENT, NULL, 3, $2, $4, $6); }
             ;
while_statement : WHILE relation DO statement { CREATE_NODE ($$, WHILE_STATEMENT, NULL, 2, $2, $4); };
relation : expression '=' expression { CREATE_NODE ($$, RELATION, strdup("="), 2, $1, $3); }
         | expression '<' expression { CREATE_NODE ($$, RELATION, strdup("<"), 2, $1, $3); }
         | expression '>' expression { CREATE_NODE ($$, RELATION, strdup(">"), 2, $1, $3); }
         ;
expression : expression '|' expression { CREATE_NODE ($$, EXPRESSION, strdup("|"), 2, $1, $3); }
           | expression '^' expression { CREATE_NODE ($$, EXPRESSION, strdup("^"), 2, $1, $3); }
           | expression '&' expression { CREATE_NODE ($$, EXPRESSION, strdup("&"), 2, $1, $3); }
           | expression LSHIFT expression { CREATE_NODE ($$, EXPRESSION, strdup("<<"), 2, $1, $3); }
           | expression RSHIFT expression { CREATE_NODE ($$, EXPRESSION, strdup(">>"), 2, $1, $3); }
           | expression '+' expression { CREATE_NODE ($$, EXPRESSION, strdup("+"), 2, $1, $3); }
           | expression '-' expression { CREATE_NODE ($$, EXPRESSION, strdup("-"), 2, $1, $3); }
           | expression '*' expression { CREATE_NODE ($$, EXPRESSION, strdup("*"), 2, $1, $3); }
           | expression '/' expression { CREATE_NODE ($$, EXPRESSION, strdup("/"), 2, $1, $3); }
           | '-' expression { CREATE_NODE ($$, EXPRESSION, strdup("-"), 1, $2); }
           | '~' expression { CREATE_NODE ($$, EXPRESSION, strdup("~"), 1, $2); }
           | '(' expression ')' { CREATE_NODE ($$, EXPRESSION, NULL, 1, $2); }
           | number { CREATE_NODE ($$, EXPRESSION, NULL, 1, $1); }
           | identifier { CREATE_NODE ($$, EXPRESSION, NULL, 1, $1); }
           | identifier '(' argument_list ')' { CREATE_NODE ($$, EXPRESSION, NULL, 2, $1, $3); }
           ;
declaration : VAR variable_list { CREATE_NODE ($$, DECLARATION, NULL, 1, $2); };
print_item : expression { CREATE_NODE ($$, PRINT_ITEM, NULL, 1, $1); }
           | string { CREATE_NODE ($$, PRINT_ITEM, NULL, 1, $1); };
identifier : IDENTIFIER { CREATE_NODE ($$, IDENTIFIER_DATA, strdup(yytext), 0, NULL); };
number : NUMBER {
          CREATE_NODE ($$, NUMBER_DATA, calloc(1, sizeof(int64_t)), 0, NULL);
          *(int64_t *)$$->data = strtol(yytext, NULL, 10);
         };
string : STRING { CREATE_NODE ($$, STRING_DATA, strdup(yytext), 0, NULL); };

%%

int
yyerror ( const char *error )
{
    fprintf ( stderr, "%s on line %d\n", error, yylineno );

    exit ( EXIT_FAILURE );
}

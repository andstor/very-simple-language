#include <vslc.h>

// Externally visible, for the generator
extern tlhash_t *global_names;
extern char **string_list;
extern size_t n_string_list,stringc;

/* External interface */
void create_symbol_table(void);
void print_symbol_table(void);
void print_symbols(void);
void print_bindings(node_t *root);
void destroy_symbol_table(void);
void find_globals(void);
void bind_names(symbol_t *function, node_t *root);
void destroy_symtab(void);

/* Stack of tables for local scopes */
static tlhash_t **scopes = NULL;    /* Dynamic array of tables */
static size_t
    n_scopes = 1,       /* Size of the table */
    scope_depth = 0;    /* Number of scopes on stack */


void
create_symbol_table ( void )
{
  find_globals();
  size_t n_globals = tlhash_size ( global_names );
  symbol_t *global_list[n_globals];
  tlhash_values ( global_names, (void **)&global_list );
  for ( size_t i=0; i<n_globals; i++ )
      if ( global_list[i]->type == SYM_FUNCTION )
          bind_names ( global_list[i], global_list[i]->node );
}


void
print_symbol_table ( void )
{
	print_symbols();
	print_bindings(root);
}


void
print_symbols ( void )
{
    printf ( "String table:\n" );
    for ( size_t s=0; s<stringc; s++ )
        printf  ( "%zu: %s\n", s, string_list[s] );
    printf ( "-- \n" );

    printf ( "Globals:\n" );
    size_t n_globals = tlhash_size(global_names);
    symbol_t *global_list[n_globals];
    tlhash_values ( global_names, (void **)&global_list );
    for ( size_t g=0; g<n_globals; g++ )
    {
        switch ( global_list[g]->type )
        {
            case SYM_FUNCTION:
                printf (
                    "%s: function %zu:\n",
                    global_list[g]->name, global_list[g]->seq
                );
                if ( global_list[g]->locals != NULL )
                {
                    size_t localsize = tlhash_size( global_list[g]->locals );
                    printf (
                        "\t%zu local variables, %zu are parameters:\n",
                        localsize, global_list[g]->nparms
                    );
                    symbol_t *locals[localsize];
                    tlhash_values(global_list[g]->locals, (void **)locals );
                    for ( size_t i=0; i<localsize; i++ )
                    {
                        printf ( "\t%s: ", locals[i]->name );
                        switch ( locals[i]->type )
                        {
                            case SYM_PARAMETER:
                                printf ( "parameter %zu\n", locals[i]->seq );
                                break;
                            case SYM_LOCAL_VAR:
                                printf ( "local var %zu\n", locals[i]->seq );
                                break;
                        }
                    }
                }
                break;
            case SYM_GLOBAL_VAR:
                printf ( "%s: global variable\n", global_list[g]->name );
                break;
        }
    }
    printf ( "-- \n" );
}


void
print_bindings ( node_t *root )
{
    if ( root == NULL )
        return;
    else if ( root->entry != NULL )
    {
        switch ( root->entry->type )
        {
            case SYM_GLOBAL_VAR: 
                printf ( "Linked global var '%s'\n", root->entry->name );
                break;
            case SYM_FUNCTION:
                printf ( "Linked function %zu ('%s')\n",
                    root->entry->seq, root->entry->name
                );
                break; 
            case SYM_PARAMETER:
                printf ( "Linked parameter %zu ('%s')\n",
                    root->entry->seq, root->entry->name
                );
                break;
            case SYM_LOCAL_VAR:
                printf ( "Linked local var %zu ('%s')\n",
                    root->entry->seq, root->entry->name
                );
                break;
        }
    } else if ( root->type == STRING_DATA ) {
        size_t string_index = *((size_t *)root->data);
        if ( string_index < stringc )
            printf ( "Linked string %zu\n", *((size_t *)root->data) );
        else
            printf ( "(Not an indexed string)\n" );
    }
    for ( size_t c=0; c<root->n_children; c++ )
        print_bindings ( root->children[c] );
}


void
destroy_symbol_table ( void )
{
      destroy_symtab();
}


/* Insert strings in the dynamic table
 * and resize (double) it's capacity when/if it runs out
 */
static void
add_string ( node_t *node )
{
    /* Resize the table by a factor of 2 if it is full */
    if ( stringc >= n_string_list )
    {
        n_string_list *= 2;
        string_list = realloc ( string_list, n_string_list * sizeof(char *) );
    }
    /* Add pointer to string from node to the string table */
    string_list[stringc] = node->data;
    /* Replace node's data with string table index */
    node->data = malloc ( sizeof(size_t) );
    *(size_t *)node->data = stringc;
    stringc++;   
}

/* Create a new local symbol table and push it on stack.
 * Count the present nesting depth, and resize stack if it is full
 */
static void
push_scope ( void )
{
    if ( scopes == NULL )
        scopes = malloc ( n_scopes * sizeof(tlhash_t *) );
    tlhash_t *new_scope = malloc ( sizeof(tlhash_t) );
    tlhash_init ( new_scope, 32 );
    scopes[scope_depth] = new_scope;

    scope_depth += 1;
    if ( scope_depth >= n_scopes )
    {
        n_scopes *= 2;
        scopes = realloc ( scopes, n_scopes*sizeof(tlhash_t **) );
    }
}

/* Remove and finalize the table for a local scope */
static void
pop_scope ( void )
{
    scope_depth -= 1;
    tlhash_finalize ( scopes[scope_depth] );
    free ( scopes[scope_depth] );
    scopes[scope_depth] = NULL;
}

/* Lookup name on the local symbol stack from top down */
static symbol_t *
lookup_local ( char *name )
{
    symbol_t *result = NULL;
    size_t depth = scope_depth;
    while ( result == NULL && depth > 0 )
    {
        depth -= 1;
        tlhash_lookup ( scopes[depth], name, strlen(name), (void **)&result );
    }
    return result;
}


void
find_globals ( void )
{
    /* Init dynamic list */
    global_names = malloc ( sizeof(tlhash_t) );
    tlhash_init ( global_names, 32 );
    string_list = malloc ( n_string_list * sizeof(char * ) );
    
    node_t *list = root->children[0];
    size_t seq = 0;
    /* Itterate through all the children of the root node */
    for (int i = 0; i < list->n_children; i++)
    {
        node_t  
            *global = list->children[i],
            *name_list;
        symbol_t *symbol;

        switch(global->type) 
        {
            /* Functions */
            case FUNCTION:
                symbol = malloc ( sizeof(symbol_t) );
                *symbol = (symbol_t) {
                    .name = global->children[0]->data,
                    .type = SYM_FUNCTION,
                    .node = global->children[2],
                    .seq = seq,
                    .nparms = 0,
                    .locals = malloc(sizeof(tlhash_t))
                };
                tlhash_init ( symbol->locals, 32 );

                /* Handle function parameters (optimisation) */
                if (global->children[1] != NULL) {
                    symbol->nparms = global->children[1]->n_children;
                    for (int i = 0; i < symbol->nparms; i++)
                    {
                        node_t *param = global->children[1]->children[i];
                        symbol_t *psymbol = malloc ( sizeof(symbol_t) );
                        *psymbol = (symbol_t) {
                            .name = param->data,
                            .type = SYM_PARAMETER,
                            .node = NULL,
                            .seq = i,
                            .nparms = 0,
                            .locals = NULL
                        };
                        tlhash_insert(symbol->locals, psymbol->name, strlen(psymbol->name), psymbol);
                    }
                }

                tlhash_insert(global_names, symbol->name, strlen(symbol->name), symbol);
                seq++;
                break;
            
            /* Global variables */
            case DECLARATION:
                name_list = global->children[0];
                for (int i = 0; i < name_list->n_children; i++) {
                    symbol = malloc(sizeof(symbol_t));
                    *symbol = (symbol_t) {
                        .name = name_list->children[i]->data,
                        .type = SYM_GLOBAL_VAR,
                        .node = NULL,
                        .seq = 0,
                        .nparms = 0,
                        .locals = NULL
                    };
                    tlhash_insert(global_names, symbol->name, strlen(symbol->name), symbol);
                }
                break;
        }
    }
}

/* Look up and handle local definitions and name uses */
void
bind_names ( symbol_t *function, node_t *root )
{
    if (root == NULL)
        return;
    
    switch (root->type)
    {
        node_t *name_list;
        symbol_t *entry;
    
    /* Blocks defines a new nested scope, recur into it's statements */
    case BLOCK:
        push_scope();
        for ( size_t i = 0; i < root->n_children; i++ )
                bind_names ( function, root->children[i] );
        pop_scope();
        break;
    
    /* Declarations creates new symbol entries on both the local stack (indexed by strings), 
     * and in the function's table indexed index number.
     */
    case DECLARATION:
        name_list = root->children[0];
        for ( uint64_t i = 0; i < name_list->n_children; i++ )
        {
            node_t *var_node = name_list->children[i];
            size_t local_num =
                tlhash_size(function->locals) - function->nparms;
            symbol_t *symbol = malloc ( sizeof(symbol_t) );
            *symbol = (symbol_t) {
                .type = SYM_LOCAL_VAR,
                .name = var_node->data,
                .node = NULL,
                .seq = local_num,
                .nparms = 0,
                .locals = NULL
            };
            
            /* Insert into local stack table at curent stack (block) level, indexed by name (string) */
            tlhash_insert (
                scopes[scope_depth-1], symbol->name, strlen(symbol->name), symbol
            );
            
            /* Index function's table on index number instead of string,
             * to avoid name clashes. This is to retain pointers to the
             * symbols after all names are bound, the local scope tables
             * disappear as the scopes end.
             */
            tlhash_insert (
                function->locals, &local_num, sizeof(size_t), symbol
            );
        }
        break;

        /* Identifiers corresponds to uses in expressions.
         * Look up the symbol, and attach it to the tree node.
         */
        case IDENTIFIER_DATA:
            /* Local variable? */
            entry = lookup_local ( root->data );

            /* Parameter? */
            if ( entry == NULL )
                tlhash_lookup (
                    function->locals, root->data, strlen(root->data), (void**)&entry
                );

            /* Global name? */
            if ( entry == NULL )
                tlhash_lookup (
                    global_names, root->data, strlen(root->data), (void**)&entry
                );

            /* No lokups matched. Crash and burn! */
            if ( entry == NULL )
            {
                fprintf ( stderr, "Identifier '%s' does not exist in scope\n",
                    (char *)root->data
                );
                exit ( EXIT_FAILURE );
            }

            /* Attatch symbol to node */
            root->entry = entry;
            break;

        /* Strings are to be added to the string table */
        case STRING_DATA:
            add_string ( root );
            break;

        /* Otherwise, recur into the node's children */
        default:
            for ( size_t i = 0; i < root->n_children; i++ )
                bind_names ( function, root->children[i] );
            break;
    }
}

/* Remove dynamically allocated symbol table data */
void
destroy_symtab ( void )
{
    /* Remove string table data */
    for ( size_t i = 0; i < stringc; i++ )
        free ( string_list[i] );
    free ( string_list );

    /* Remove the global symbol table and local symbols in all function entries */
    size_t n_globals = tlhash_size ( global_names );
    symbol_t *global_list[n_globals];
    tlhash_values ( global_names, (void **)&global_list );
    for ( size_t j = 0; j < n_globals; j++ )
    {
        symbol_t *global = global_list[j];
        if ( global->locals != NULL )
        {
            size_t n_locals = tlhash_size ( global->locals );
            symbol_t *locals[n_locals];
            tlhash_values ( global->locals, (void **)&locals );
            for ( size_t k = 0; k < n_locals; k++ )
                free ( locals[k] );
            tlhash_finalize ( global->locals );
            free ( global->locals );
        }
        free ( global );
    }
    tlhash_finalize ( global_names );
    free ( global_names );
    free ( scopes );
}
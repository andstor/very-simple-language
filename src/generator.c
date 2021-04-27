#include "vslc.h"

#define MIN(a,b) (((a)<(b)) ? (a):(b))
static const char *record[6] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
};

static void generate_expression ( node_t *node, symbol_t *function);
static void generate_node ( node_t *node, symbol_t *function);

size_t condcnt;
size_t loopcnt;


static void
generate_stringtable ( void )
{
    /* These can be used to emit numbers, strings and a run-time
     * error msg. from main
     */ 
    puts ( ".data" );
    puts ( "_intout: .asciz \"\%ld \"" );
    puts ( "_strout: .asciz \"\%s \"" );
    puts ( "_errout: .asciz \"Wrong number of arguments\"" );

    /* Handle the strings from the program */
    for ( size_t i = 0; i < stringc; i++)
        printf ( "STR%zu: .string %s\n", i, string_list[i] );
}

static void
generate_global_variables ( void )
{
    puts ( ".data" ); //puts ( ".section .data" );
    size_t n_globals = tlhash_size ( global_names );
    symbol_t *global_list[n_globals];
    tlhash_values ( global_names, (void **)&global_list );
    for ( size_t i = 0; i < n_globals; i++ )
    {
        if ( global_list[i]->type == SYM_GLOBAL_VAR )
            printf ( "__%s: .zero 8\n", global_list[i]->name );
    }
}

static void
generate_main ( symbol_t *first )
{
    puts ( ".globl _main" );
    puts ( ".text" );
    puts ( "_main:" );
    puts ( "\tpushq\t%rbp" );
    puts ( "\tmovq\t%rsp, %rbp" );

    puts ( "\tsubq\t$1, %rdi" );
    printf ( "\tcmpq\t$%zu,%%rdi\n", first->nparms );
    puts ( "\tjne\t\tABORT" );
    puts ( "\tcmpq\t$0, %rdi" );
    puts ( "\tjz\t\tSKIP_ARGS" );

    puts ( "\tmovq\t%rdi, %rcx" );
    printf ( "\taddq\t$%zu, %%rsi\n", 8*first->nparms );
    puts ( "PARSE_ARGV:" );
    puts ( "\tpushq\t%rcx" );
    puts ( "\tpushq\t%rsi" );

    puts ( "\tmovq\t(%rsi), %rdi" );
    puts ( "\tmovq\t$0, %rsi" );
    puts ( "\tmovq\t$10, %rdx" );
    puts ( "\tcall\t_strtol" );

    /*  Now a new argument is an integer in rax */
    puts ( "\tpopq\t%rsi" );
    puts ( "\tpopq\t%rcx" );
    puts ( "\tpushq\t%rax" );
    puts ( "\tsubq\t$8, %rsi" );
    puts ( "\tloop\tPARSE_ARGV" );

    /* Now the arguments are in order on stack */
    for ( int arg=0; arg<MIN(6,first->nparms); arg++ )
        printf ( "\tpopq\t%s\n", record[arg] );

    puts ( "SKIP_ARGS:" );
    printf ( "\tcall\t__%s\n", first->name );
    puts ( "\tjmp\t\tEND" );
    puts ( "ABORT:" );
    puts ( "\tleaq\t_errout(%rip), %rdi" );
    puts ( "\tcall\t_puts" );

    puts ( "END:" );
    puts ( "\tmovq\t%rax, %rdi" );
    puts ( "\tandq\t$-16, %rsp" ); // Ensure 16-byte alignment
    puts ( "\tcall\t_exit" );
}

static void
generate_identifier ( node_t *node, symbol_t *function)
{

    symbol_t *symbol = node->entry;
    switch ( symbol->type )
    {

        case SYM_GLOBAL_VAR:
            printf ( "__%s(%%rip)", symbol->name );
            break;
        
        case SYM_PARAMETER:
            if ( symbol->seq+1 > 6 )
                printf ( "%ld(%%rbp)", 8+8*(symbol->seq+1 - 6) ); // Params are in parent function's stack
            else
                printf ( "%ld(%%rbp)", -8*(symbol->seq+1) ); // Params are in current function's stack
            break;
        case SYM_LOCAL_VAR:
            if ( function->nparms >= 6 ) { // Params have been saved onto the stack
                printf ( "%ld(%%rbp)", -8*(6+symbol->seq+1) );
            }
            else 
            { // All params saved in registers
                printf ( "%ld(%%rbp)", -8*(function->nparms + symbol->seq+1) );
            }
            break;
    }
}


void generate_function_call(node_t* node, symbol_t *function) {
    int64_t param_count = node->children[1] == NULL ? 0 : node->children[1]->n_children;
    
    // Save parameter registers // Todo: only save needed registers
    for ( int i = 0; i < 6; i++ )
    {
        printf ( "\tpushq\t%s\n", record[i] );
    }
    // Put arguments to stack
    for ( int i = param_count - 1; i >= 0; i-- )
    {
        generate_expression ( node->children[1]->children[i], function );
        puts ( "\tpushq\t%rax" );
    }
    // Load stack values into relevant parameter registers
    for ( int param = 0; param < MIN(6, param_count); param++ ) {
        printf ( "\tpopq\t%s\n", record[param] );
    }
    // Call the function
    printf("\tcall\t__%s\n", (char *) node->children[0]->data);
    // Reset the stack if > 6 parameters
    if (param_count > 6 )
    {
        printf ( "\taddq\t$%llu, %%rsp\n", 8*(param_count - 6) );
    }
    // Reset parameter registers
    for ( int i = 6; i > 0; i-- ) 
    {
        printf ( "\tpopq\t%s\n", record[i-1] );
    }
}

static void
generate_expression ( node_t *expr, symbol_t *function )
{
    char *op = ((char *)expr->data);
    
    if ( expr->type == IDENTIFIER_DATA )
    {
        printf ( "\tmovq\t" );
        generate_identifier ( expr, function );
        printf ( ", %%rax\n" );
    }
    else if ( expr->type == NUMBER_DATA )
    {
        printf ( "\tmovq\t$%lld, %%rax\n", *(int64_t *)expr->data );
    }
    else if ( expr->n_children == 1 )
    {
        generate_expression ( expr->children[0], function );
        switch ( *op )
        {
        case '-':
            puts ( "\tnegq\t%rax" );
            break;
        case '~':
            puts ( "\tnotq\t%rax" );
            break;
        }
    }
    else if ( expr->n_children == 2 )
    {
        if ( expr->data != NULL )
        {
            switch ( *op )
            {
                case '+':
		            generate_expression ( expr->children[0], function );
                    puts ( "\tpushq\t%rax" );
                    generate_expression ( expr->children[1], function );
                    puts ( "\taddq\t%rax, (%rsp)" );
                    puts ( "\tpopq\t%rax" );
                    break;
                case '-':
		            generate_expression ( expr->children[0], function );
                    puts ( "\tpushq\t%rax" );
                    generate_expression ( expr->children[1], function );
                    puts ( "\tsubq\t%rax, (%rsp)" );
                    puts ( "\tpopq\t%rax" );
                    break;
                case '*':
                    puts ( "\tpushq\t%rdx" );
                    generate_expression ( expr->children[1], function );
                    puts ( "\tpushq\t%rax" );
                    generate_expression ( expr->children[0], function );
                    puts ( "\timulq\t(%rsp)" ); // Multiplies value at spack pointer with %rax and stores result in %rdx:%rax
                    puts ( "\tpopq\t%rdx" );
                    puts ( "\tpopq\t%rdx" );
                    break;
                case '/':
                    puts ( "\tpushq\t%rdx" );
                    generate_expression ( expr->children[1], function );
                    puts ( "\tpushq\t%rax" );
                    generate_expression ( expr->children[0], function );
                    puts ( "\tcqo" ); // Sign extend to 128-bit rdx:rax.
                    puts ( "\tidivq\t(%rsp)" ); // Divide %rdx:%rax by value at spack pointer, stores result in %rax and reminder in %rdx.
                    puts ( "\tpopq\t%rdx" );
                    puts ( "\tpopq\t%rdx" );
                    break;
                case '|':
		            generate_expression ( expr->children[0], function );
                    puts ( "\tpushq\t%rax" );
                    generate_expression ( expr->children[1], function );
                    puts ( "\torq\t\t%rax, (%rsp)" );
                    puts ( "\tpopq\t%rax" );
                    break;
                case '&':
		            generate_expression ( expr->children[0], function );
                    puts ( "\tpushq\t%rax" );
                    generate_expression ( expr->children[1], function );
                    puts ( "\tandq\t%rax, (%rsp)" );
                    puts ( "\tpopq\t%rax" );
                    break;
                case '^':
		            generate_expression ( expr->children[0], function );
                    puts ( "\tpushq\t%rax" );
                    generate_expression ( expr->children[1], function );
                    puts ( "\txorq\t\t%rax, (%rsp)" );
                    puts ( "\tpopq\t%rax" );
                    break;
                //TODO: handle rest of arithmetics and refactor
            }

            if ((strcmp( op,">>") == 0 ))
            {
                generate_expression ( expr->children[0], function );
                puts ( "\tpushq\t%rax" );
                generate_expression ( expr->children[1], function );
                puts ( "\tmovb\t%al, %cl" );
                puts( "\tshrq\t%cl, (%rsp)" );
                puts ( "\tpopq\t%rax" );
            }
            else if (( strcmp( op,"<<") == 0 ))
            {
                generate_expression ( expr->children[0], function );
                puts ( "\tpushq\t%rax" );
                generate_expression ( expr->children[1], function );
                puts ( "\tmovb\t%al, %cl" );
                puts( "\tshlq\t%cl, (%rsp)" );
                puts ( "\tpopq\t%rax" );
            } else {
                    // do nothing
            }
        }
        else // Function call
        {
            generate_function_call(expr, function);
        }
    }
}

static void
generate_assignment_statement ( node_t *statement, symbol_t *function)
{
    generate_expression ( statement->children[1], function );
    
    // Copy expression result (%rax) into identifier address
    printf ( "\tmovq\t%%rax, " );
    generate_identifier ( statement->children[0], function );
    printf ( "\n" );
}

generate_print_statement ( node_t *statement, symbol_t *function )
{
    for ( size_t i = 0; i < statement->n_children; i++ )
    {
        node_t *item = statement->children[i];
        switch ( item->type )
        {
            case STRING_DATA:
                printf ( "\tleaq\tSTR%zu(%%rip), %%rsi\n", *((size_t *)item->data) );
                puts ( "\tleaq\t_strout(%rip), %rdi" );
                puts ( "\tcall\t_printf" );
                break;
            case NUMBER_DATA:
                printf ("\tmovq\t$%lld, %%rsi\n", *((int64_t *)item->data) );
                puts ( "\tleaq\t_intout(%rip), %rdi" );
                puts ( "\tcall\t_printf" );
                break;

            case IDENTIFIER_DATA:
                printf ( "\tmovq\t" );
                generate_identifier ( item, function );
                printf ( ", %%rsi\n" );
                puts ( "\tleaq\t_intout(%rip), %rdi" );
                puts ( "\tcall\t_printf" );
                break;

            case EXPRESSION:           
	            generate_expression ( item, function );
                puts ( "\tmovq\t%rax, %rsi" );
                puts ( "\tleaq\t_intout(%rip), %rdi" );
                puts ( "\tcall\t_printf" );

                break;
        }
    }
    puts ( "\tmovq\t$'\\n', %rdi" );
    puts ( "\tcall\t_putchar" );
}

static void
generate_if_statement ( node_t *statement, symbol_t *function )
{
    condcnt++;
    size_t current_condcnt = condcnt;

    // Evaluate condition
    puts ( "\tpushq\t%rdx" );
    generate_expression ( statement->children[0]->children[0], function );
    puts ( "\tpushq\t%rax" );
    generate_expression ( statement->children[0]->children[1], function);
    puts ( "\tpopq\t%rdx" );
    puts ( "\tcmpq\t%rax, %rdx" );
    puts ( "\tpopq\t%rdx" );

    // Jump to end or else if if-condition is false
    switch ( *((char *)statement->children[0]->data) ) {
        case '=':
            if ( statement->n_children == 2 ) {
                printf ( "\tjne\t\tENDIF%lu\n", current_condcnt );
            } else {
                printf ( "\tjne\t\tELSE%lu\n", current_condcnt );
            }
            break;
        case '<':
            if ( statement->n_children == 2 ) {
                printf ( "\tjge\t\tENDIF%lu\n", current_condcnt );
            } else {
                printf ( "\tjge\t\tELSE%lu\n", current_condcnt );
            }
            break;
        case '>':
            if ( statement->n_children == 2 ) {
                printf ( "\tjle\t\tENDIF%lu\n", current_condcnt );
            } else {
                printf ( "\tjle\t\tELSE%lu\n", current_condcnt );
            }
            break;
    }

    // Generate if statement body
    generate_node ( statement->children[1], function );
    // Generate else statement
    if ( statement->n_children == 3 ) {
        printf ( "\tjmp\t\tENDIF%lu\n", current_condcnt );
        printf ( "ELSE%lu:\n", current_condcnt );
        // Generate else statement body
	    generate_node ( statement->children[2], function );
    }
    // End if statement
    printf ( "ENDIF%lu:\n", current_condcnt );
}

static void
generate_while_statement ( node_t *statement, symbol_t *function )
{
    loopcnt++;
    size_t loop_count = loopcnt;
    
    printf ( "WHILE%lu:\n", loop_count );
    puts ( "\tpushq\t%rdx" );
    generate_expression ( statement->children[0]->children[0], function );
    puts ( "\tpushq\t%rax" );
    generate_expression ( statement->children[0]->children[1], function );
    puts ( "\tpopq\t%rdx" );
    puts ( "\tcmpq\t%rax, %rdx" );
    puts ( "\tpopq\t%rdx" );

    switch ( *((char *)statement->children[0]->data) )
    {
    case '=':
	    printf ( "\tjne\t\tENDWHILE%lu\n", loop_count );
	    break;
    case '<':
	    printf ( "\tjge\t\tENDWHILE%lu\n", loop_count );
	   break;
    case '>':
        printf ( "\tjle\t\tENDWHILE%lu\n", loop_count );
        break;
    }
    
    generate_node ( statement->children[1], function );
    printf ( "\tjmp\t\tWHILE%lu\n", loop_count );
    printf ( "ENDWHILE%lu:\n", loop_count );
}

static void
generate_node ( node_t *node, symbol_t *function)
{
    switch (node->type)
    {
    case ASSIGNMENT_STATEMENT:
	    generate_assignment_statement ( node, function );
        break;
    case RETURN_STATEMENT:
	    generate_expression ( node->children[0], function);
        puts ( "\tleave" );
        puts( "\tret" );
        break;
    case PRINT_STATEMENT:
	    generate_print_statement ( node, function );
        break;
    case NULL_STATEMENT:
	    printf ( "\tjmp\tWHILE%lu\n", loopcnt );
	    break;
    case IF_STATEMENT:
	    generate_if_statement ( node, function );
        break;
    case WHILE_STATEMENT:
	    generate_while_statement ( node, function );
	    break;
    default:
        // Recurse into children
	    for (int i = 0; i < node->n_children; i++) {
            generate_node(node->children[i], function);
        }
        break;
    }
}


static void
generate_function ( symbol_t *function )
{
    printf ( "__%s:\n", function->name );
    puts ( "\tpushq\t%rbp"); // Push the base pointer onto the stack
    puts ( "\tmovq\t%rsp, %rbp"); // Move the current stack pointer to the new base pointer

    // Save arguments in local stack frame
    for ( size_t arg=0; arg<MIN(6, function->nparms); arg++ )
            printf ( "\tpushq\t%s\n", record[arg] );
    
    // Make space for local variables
    size_t n_locals = tlhash_size(function->locals) - function->nparms;
    printf ( "\tsubq\t$%ld, %%rsp\n", 8*n_locals );
    // Align stack to 16 bytes if odd # of locals/arguments
    if ( (tlhash_size(function->locals)&1) == 1 )
        puts ( "\tsubq\t$8, %rsp" );

    generate_node ( function->node, function );
}

void
generate_program ( void )
{
    generate_stringtable();
    generate_global_variables();
    
    size_t n_globals = tlhash_size ( global_names );
    symbol_t *global_list[n_globals];
    tlhash_values ( global_names, (void **)&global_list );
    for (size_t i = 0; i < n_globals; i++)
    {
        if ( global_list[i]->type == SYM_FUNCTION && global_list[i]->seq == 0 ) {
            generate_main(global_list[i]);
            break;
        }
    }

    for ( size_t i=0; i<tlhash_size(global_names); i++ )
    {
        if ( global_list[i]->type == SYM_FUNCTION )
            generate_function ( global_list[i] );
    }
}

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vslc.h>

node_t *root;
tlhash_t *global_names;     // Symbol table        
char **string_list;         // List of strings in the source
size_t n_string_list = 8;   // Initial string list capacity (grow on demand)                                            
size_t stringc = 0;         // Initial string count

int
main ( int argc, char **argv )
{
    // Available flags options
    int tree_flag = 0;
    int symtab_flag = 0;
    int generate_flag = 0;
    int index;
    int c;

    opterr = 0;
    while ((c = getopt (argc, argv, "tsg")) != -1)
        switch (c)
        {
        case 't': // Output tree
            tree_flag = 1;
            break;
        case 's': // Output symbol table
            symtab_flag = 1;
            break;
        case 'g': // Generate assembly code
            generate_flag = 1;
            break;
        case '?':
            if (isprint (optopt))
              fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
              fprintf (stderr,
                      "Unknown option character `\\x%x'.\n",
                      optopt);
            return 1;
        default:
          abort ();
        }
    
    
    for (index = optind; index < argc; index++)
        printf ("Non-option argument %s\n", argv[index]);

    yyparse();
    simplify_tree ( &root, root );
    if (tree_flag)
        node_print ( root, 0 );
    create_symbol_table();
    if (symtab_flag)
        print_symbol_table();
    if (generate_flag || (!tree_flag && !symtab_flag))
        generate_program(); // Generate the program
    destroy_subtree ( root );
	destroy_symbol_table ();
}

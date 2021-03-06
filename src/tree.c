#include <vslc.h>


void
node_print ( node_t *root, int nesting )
{
    if ( root != NULL )
    {
        /* Print the type of node indented by the nesting level */
        printf ( "%*c%s", nesting, ' ', node_string[root->type] );

        /* For identifiers, strings, expressions and numbers,
         * print the data element also
         */
        if (root->data !=  NULL) {
            if ( root->type == IDENTIFIER_DATA ||
                root->type == STRING_DATA ||
                root->type == EXPRESSION ||
                root->type == RELATION ) 
                printf ( "(%s)", (char *) root->data );
            else if ( root->type == NUMBER_DATA )
                printf ( "(%lld)", *((int64_t *)root->data) );
        }
        /* Make a new line, and traverse the node's children in the same manner */
        putchar ( '\n' );
        for ( int64_t i=0; i<root->n_children; i++ )
            node_print ( root->children[i], nesting+1 );
    }
    else
        printf ( "%*c%p\n", nesting, ' ', root );
}


/* Take the memory allocated to a node and fill it in with the given elements */
void
node_init (node_t *nd, node_index_t type, void *data, uint64_t n_children, ...)
{
    nd->type = type;
    nd->data = data;
    nd->n_children = n_children;
    nd->children = (node_t **) malloc ( n_children * sizeof(node_t *));
    
    va_list (child_list);
    va_start ( child_list, n_children );
    for ( uint64_t i=0; i<n_children; i++ ) {
        nd->children[i] = va_arg ( child_list, node_t * );
    }
    va_end ( child_list );
}


/* Remove a node and its contents */
void
node_finalize ( node_t *discard )
{
    if (discard != NULL) {
        free ( discard->data);
        free ( discard->children);
        free ( discard );
    }
}

/* Recursively remove the entire tree rooted at a node */
void
destroy_subtree ( node_t *discard )
{
    if (discard != NULL) {
        for (uint64_t i = 0; i<discard->n_children; i++)
            destroy_subtree ( discard->children[i]);
        node_finalize ( discard );
    }
}

void
simplify_tree ( node_t **simplified, node_t *root )
{
    if (root == NULL)
        return ;

    /* Simplify subtrees */
    for (uint64_t i = 0; i < root->n_children; i++)
        simplify_tree(&root->children[i], root->children[i]);

    node_t *result = root;
    switch (root->type)
    {
    /* Remove nodes of purely syntactic value */
    case PARAMETER_LIST:
    case ARGUMENT_LIST:
    case STATEMENT:
    case PRINT_ITEM:
    case GLOBAL:
        result = root->children[0];
        node_finalize(root);
        break;
    case PRINT_STATEMENT:
        result = root->children[0];
        result->type = PRINT_STATEMENT;
        node_finalize(root);
        break;
    
    /* 
     * Flatten list structures
     * Take left child and append the right child.
     * Replace root with left.
     */
    case STATEMENT_LIST:
    case DECLARATION_LIST:
    case GLOBAL_LIST:
    case PRINT_LIST:
    case EXPRESSION_LIST:
    case VARIABLE_LIST:
        if (root->n_children > 1)
        {
            result = root->children[0];
            result->n_children += 1;
            result->children = realloc(
                result->children, result->n_children * sizeof(node_t *)
            );
            result->children[result->n_children - 1] = root->children[1];
            node_finalize(root);
        }
        break;

    /* Resolve constant Expressions */
    case EXPRESSION:
        switch (root->n_children) {
            case 1:
                /* Remove node of purely syntactic value */
                if ( root->children[0]->type == NUMBER_DATA ) {
                    result = root->children[0];

                    /* Handle urnary minus */
                    if ( root->data != NULL )
                        *((int64_t *)result->data) *= -1;
                    node_finalize (root);
                }
                /* Remove null expressions */
                else if ( root->data == NULL )
                {
                        result = root->children[0];
                        node_finalize (root);
                }
                break;
            
            case 2:
                /* Calculate arithmetic operators */
                if ( root->children[0]->type == NUMBER_DATA &&
                    root->children[1]->type == NUMBER_DATA
                ) {
                    result = root->children[0];
                    int64_t
                        *x = result->data,
                        *y = root->children[1]->data;
                    char *cc = ((char *)root->data);

                    if ((strcmp(cc,"+") == 0)) {
                        *x += *y; 
                    } else if ((strcmp(cc,"-") == 0)) {
                        *x -= *y; 
                    } else if ((strcmp(cc,"*") == 0)) {
                        *x *= *y; 
                    } else if ((strcmp(cc,"/") == 0)) {
                        *x /= *y; 
                    } else if ((strcmp(cc,"<<") == 0)) {
                        *x =  *x << *y; 
                    } else if ((strcmp(cc,">>") == 0)) {
                        *x =  *x >> *y; 
                    } else if ((strcmp(cc,"&") == 0)) {
                        *x =  *x & *y;
                    } else if ((strcmp(cc,"^") == 0)) {
                        *x =  *x ^ *y;
                    } else if ((strcmp(cc,"|") == 0)) {
                        *x =  *x | *y;
                    } else {
                    // do nothing
                    }
                    node_finalize ( root->children[1] );
                    node_finalize ( root );
                }
                break;
            
            default:
                break;
        }

        break;
    default:
        break;
    }

    *simplified = result;
}

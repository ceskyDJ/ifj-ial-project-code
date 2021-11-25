/**
 * @file context.h
 * Context for lexical and syntactical analysis.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 */

struct context {
    symstack_t *symstack;
    kwtable_t *kwtable;
    string_t *string;
};

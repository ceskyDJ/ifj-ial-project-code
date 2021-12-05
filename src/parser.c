/**
 * @file parser.c
 * Syntactical top to bottom analysis implementation.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 */

#include "context.h"
#include "exit_codes.h"
//#include "expr_parser.h"
#include "kwtable.h"
#define LOG_LEVEL ERROR
#include "logger.h"
#include "scanner.h"
#include "string_factory.h"
#include "symtable.h"
#include "symstack.h"
#include "token.h"

#include <errno.h>

static void debug_keyword(keyword_t kw)
{
    switch(kw) {
        case KW_DO: LOG_DEBUG_M("KEYWORD DO"); break;
        case KW_ELSE: LOG_DEBUG_M("KEYWORD ELSE"); break;
        case KW_END: LOG_DEBUG_M("KEYWORD END"); break;
        case KW_FUNCTION: LOG_DEBUG_M("KEYWORD FUNCTION"); break;
        case KW_GLOBAL: LOG_DEBUG_M("KEYWORD GLOBAL"); break;
        case KW_IF: LOG_DEBUG_M("KEYWORD IF"); break;
        case KW_INTEGER: LOG_DEBUG_M("KEYWORD INTEGER"); break;
        case KW_LOCAL: LOG_DEBUG_M("KEYWORD LOCAL"); break;
        case KW_NIL: LOG_DEBUG_M("KEYWORD NIL"); break;
        case KW_NUMBER: LOG_DEBUG_M("KEYWORD NUMBER"); break;
        case KW_REQUIRE: LOG_DEBUG_M("KEYWORD REQUIRE"); break;
        case KW_RETURN: LOG_DEBUG_M("KEYWORD RETURN"); break;
        case KW_STRING: LOG_DEBUG_M("KEYWORD STRING"); break;
        case KW_THEN: LOG_DEBUG_M("KEYWORD THEN"); break;
        case KW_WHILE: LOG_DEBUG_M("KEYWORD WHILE"); break;

        default: LOG_DEBUG_M("INVALID KEYWORD");
    }
}

static void debug_token(token_t token)
{
    switch(token.type) {
        case IDENTIFIER: LOG_DEBUG("IDENTIFIER %s", token.identifier->name); break;
        case KEYWORD: debug_keyword(*token.keyword); break;
        case INTEGER: LOG_DEBUG("INTEGER %d", token.integer); break;
        case NUMBER: LOG_DEBUG("NUMBER %f", token.number); break;
        case STRING: LOG_DEBUG("STRING %s", token.string); break;
        case ASSIGNMENT: LOG_DEBUG_M("="); break;
        case ADDITION: LOG_DEBUG_M("+"); break;
        case SUBTRACTION: LOG_DEBUG_M("-"); break;
        case MULTIPLICATION: LOG_DEBUG_M("*"); break;
        case DIVISION: LOG_DEBUG_M("/"); break;
        case INT_DIVISION: LOG_DEBUG_M("//"); break;
        case CONCAT: LOG_DEBUG_M(".."); break;
        case STRLEN: LOG_DEBUG_M("#"); break;
        case LT: LOG_DEBUG_M("<"); break;
        case GT: LOG_DEBUG_M(">"); break;
        case LEQ: LOG_DEBUG_M("<="); break;
        case GEQ: LOG_DEBUG_M(">="); break;
        case EQ: LOG_DEBUG_M("=="); break;
        case NEQ: LOG_DEBUG_M("~="); break;
        case TYPE_SPEC: LOG_DEBUG_M(":"); break;
        case COMMA: LOG_DEBUG_M(","); break;
        case LEFT_PAR: LOG_DEBUG_M("("); break;
        case RIGHT_PAR: LOG_DEBUG_M(")"); break;
        case END: LOG_DEBUG_M("$"); break;
        default: LOG_DEBUG_M("INVALID TOKEN");
    }
}

static void debug_identifier(identifier_t *id)
{
    if (id->type == FUNCTION) {
        LOG_DEBUG("name: '%s' param: '%s' retval: '%s' defined: %d",
                id->name, id->fun.param, id->fun.retval, id->fun.defined);
    } else if (id->type == VARIABLE) {
        LOG_DEBUG("name: '%s' type: '%c' init: '%d' used: %d",
                id->name, id->var.type, id->var.init, id->var.used);
    } else {
        LOG_DEBUG_M("IDENTIFIER TYPE NOT SET");
    }
}

static token_t term(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == IDENTIFIER) {
        if (token.identifier->type != VARIABLE) {
            LOG_ERROR("%s is not an existing variable", token.identifier->name);
            exit(EFUNCALL);
        }
        LOG_DEBUG_M("id ok");
        debug_token(token);
        token = get_next_token(ctx);
        return token;

    } else if (token.type == INTEGER) {
        LOG_DEBUG_M("TYPE integer ok");
        token = get_next_token(ctx);
        return token;

    } else if (token.type == NUMBER) {
        LOG_DEBUG_M("TYPE number ok");
        token = get_next_token(ctx);
        return token;

    } else if (token.type == STRING) {
        LOG_DEBUG_M("TYPE string ok");
        free(token.string);
        token = get_next_token(ctx);
        return token;

    } else if (token.type == KEYWORD) {
        if (*token.keyword == KW_NIL) {
            LOG_DEBUG_M("TYPE/KW nil ok");
            token = get_next_token(ctx);
            return token;
        }
    }

    exit(ESYNTAX);
}

static token_t term_seq_1(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == RIGHT_PAR) {
        LOG_DEBUG_M(") unget and return");
        unget_token(token);
        return token;

    } else if (token.type == COMMA) {
        LOG_DEBUG_M(", ok");
        token = get_next_token(ctx);
        token = term(token, ctx);
        token = term_seq_1(token, ctx);
        return token;
    }

    exit(ESYNTAX);
}

static token_t term_seq(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == IDENTIFIER) {
        LOG_DEBUG_M("id diving");
        token = term(token, ctx);
        token = term_seq_1(token, ctx);
        return token;

    } else if (token.type == RIGHT_PAR) {
        LOG_DEBUG_M(") unget and return");
        unget_token(token);
        return token;

    } else if (token.type == INTEGER) {
        LOG_DEBUG_M("TYPE integer diving");
        token = term(token, ctx);
        token = term_seq_1(token, ctx);
        return token;

    } else if (token.type == NUMBER) {
        LOG_DEBUG_M("TYPE number diving");
        token = term(token, ctx);
        token = term_seq_1(token, ctx);
        return token;

    } else if (token.type == STRING) {
        LOG_DEBUG_M("TYPE string diving");
        free(token.string);
        token = term(token, ctx);
        token = term_seq_1(token, ctx);
        return token;

    } else if (token.type == KEYWORD) {
        if (*token.keyword == KW_NIL) {
            LOG_DEBUG_M("TYPE/KW nil diving");
            token = term(token, ctx);
            token = term_seq_1(token, ctx);
            return token;
        }
    }

    exit(ESYNTAX);
}

static token_t call(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == IDENTIFIER) {
        if (token.identifier->type == FUNCTION) {
            LOG_DEBUG_M("id function ok");

            token = get_next_token(ctx);

            if (token.type != LEFT_PAR)
                exit(ESYNTAX);

            LOG_DEBUG_M("( ok");

            token = get_next_token(ctx);

            // TODO check types of terms match types in param
            token = term_seq(token, ctx);

            token = get_next_token(ctx);

            if (token.type != RIGHT_PAR)
                exit(ESYNTAX);

            LOG_DEBUG_M(") ok");

            return token;
        }
        LOG_ERROR("%s() not declared/defined", token.identifier->name);
    }

    exit(ESYNTAX);
}

static token_t type(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == KEYWORD) {
        if (*token.keyword == KW_INTEGER) {
            LOG_DEBUG_M("integer ok");
            if (ctx->saved_id->type == VARIABLE) {
                ctx->saved_id->var.type = VAR_INTEGER;
            }
            token = get_next_token(ctx);
            return token;

        } else if (*token.keyword == KW_NUMBER) {
            LOG_DEBUG_M("number ok");
            if (ctx->saved_id->type == VARIABLE) {
                ctx->saved_id->var.type = VAR_NUMBER;
            }
            token = get_next_token(ctx);
            return token;

        } else if (*token.keyword == KW_STRING) {
            if (ctx->saved_id->type == VARIABLE) {
                ctx->saved_id->var.type = VAR_STRING;
            }
            LOG_DEBUG_M("string ok");
            token = get_next_token(ctx);
            return token;
        }
    }

    exit(ESYNTAX);
}

static token_t type_list_1(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == COMMA) {
        LOG_DEBUG_M(", ok");
        token = get_next_token(ctx);

        if (token.type == KEYWORD) {
            if (*token.keyword == KW_INTEGER) {
                LOG_DEBUG_M("integer diving");
                token = type(token, ctx);
                string_appendc(ctx->param, 'i');
                token = type_list_1(token, ctx);
                return token;

            } else if (*token.keyword == KW_NUMBER) {
                LOG_DEBUG_M("number diving");
                token = type(token, ctx);
                string_appendc(ctx->param, 'n');
                token = type_list_1(token, ctx);
                return token;

            } else if (*token.keyword == KW_STRING) {
                LOG_DEBUG_M("string diving");
                token = type(token, ctx);
                string_appendc(ctx->param, 's');
                token = type_list_1(token, ctx);
                return token;
            }
        }
    } else if (token.type == RIGHT_PAR) {
        LOG_DEBUG_M(") unget and return");
        unget_token(token);
        return token;
    }

    exit(ESYNTAX);
}

static token_t type_list(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == KEYWORD) {
        if (*token.keyword == KW_INTEGER) {
            LOG_DEBUG_M("integer diving");
            token = type(token, ctx);
            string_appendc(ctx->param, 'i');
            token = type_list_1(token, ctx);
            return token;

        } else if (*token.keyword == KW_NUMBER) {
            LOG_DEBUG_M("number diving");
            token = type(token, ctx);
            string_appendc(ctx->param, 'n');
            token = type_list_1(token, ctx);
            return token;

        } else if (*token.keyword == KW_STRING) {
            LOG_DEBUG_M("string diving");
            token = type(token, ctx);
            string_appendc(ctx->param, 's');
            token = type_list_1(token, ctx);
            return token;
        }
    } else if (token.type == RIGHT_PAR) {
        LOG_DEBUG_M(") unget and return");
        unget_token(token);
        return token;
    }

    exit(ESYNTAX);
}

static token_t fun_ret_list_1(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == COMMA) {
        LOG_DEBUG_M(", ok");
        token = get_next_token(ctx);

        if (token.type == KEYWORD) {
            if (*token.keyword == KW_INTEGER) {
                LOG_DEBUG_M("integer diving");
                token = type(token, ctx);
                string_appendc(ctx->retval, 'i');
                token = fun_ret_list_1(token, ctx);
                return token;

            } else if (*token.keyword == KW_NUMBER) {
                LOG_DEBUG_M("number diving");
                token = type(token, ctx);
                string_appendc(ctx->retval, 'n');
                token = fun_ret_list_1(token, ctx);
                return token;

            } else if (*token.keyword == KW_STRING) {
                LOG_DEBUG_M("string diving");
                token = type(token, ctx);
                string_appendc(ctx->retval, 's');
                token = fun_ret_list_1(token, ctx);
                return token;
            }
        }
    } else if (token.type == IDENTIFIER) {
        LOG_DEBUG_M("id unget and return");
        unget_token(token);
        return token;

    } else if (token.type == END) {
        LOG_DEBUG_M("$ unget and return");
        unget_token(token);
        return token;

    } else if (token.type == KEYWORD) {
        if (*token.keyword == KW_GLOBAL) {
            LOG_DEBUG_M("global unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_FUNCTION) {
            LOG_DEBUG_M("function unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_RETURN) {
            LOG_DEBUG_M("return unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_END) {
            LOG_DEBUG_M("end unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_LOCAL) {
            LOG_DEBUG_M("local unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_IF) {
            LOG_DEBUG_M("if unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_WHILE) {
            LOG_DEBUG_M("while unget and return");
            unget_token(token);
            return token;
        }
    }

    exit(ESYNTAX);
}

static token_t fun_ret_list(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == KEYWORD) {
        if (*token.keyword == KW_INTEGER) {
            LOG_DEBUG_M("integer diving");
            token = type(token, ctx);
            string_appendc(ctx->retval, 'i');
            token = fun_ret_list_1(token, ctx);
            return token;

        } else if (*token.keyword == KW_NUMBER) {
            LOG_DEBUG_M("number diving");
            token = type(token, ctx);
            string_appendc(ctx->retval, 'n');
            token = fun_ret_list_1(token, ctx);
            return token;

        } else if (*token.keyword == KW_STRING) {
            LOG_DEBUG_M("string diving");
            token = type(token, ctx);
            string_appendc(ctx->retval, 's');
            token = fun_ret_list_1(token, ctx);
            return token;
        }
    }

    exit(ESYNTAX);
}

static token_t fun_ret(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == TYPE_SPEC) {
        LOG_DEBUG_M(": ok");
        token = get_next_token(ctx);
        token = fun_ret_list(token, ctx);
        return token;

    } else if (token.type == END) {
        LOG_DEBUG_M("$ unget and return");
        unget_token(token);
        return token;

    } else if (token.type == IDENTIFIER) {
        LOG_DEBUG_M("id unget and return");
        unget_token(token);
        return token;

    } else if (token.type == KEYWORD) {
        if (*token.keyword == KW_GLOBAL) {
            LOG_DEBUG_M("global unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_FUNCTION) {
            LOG_DEBUG_M("function unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_RETURN) {
            LOG_DEBUG_M("return unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_END) {
            LOG_DEBUG_M("end unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_LOCAL) {
            LOG_DEBUG_M("local unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_IF) {
            LOG_DEBUG_M("if unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_WHILE) {
            LOG_DEBUG_M("while unget and return");
            unget_token(token);
            return token;
        }
    }

    exit(ESYNTAX);
}

static token_t e_1(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == COMMA) {
        LOG_DEBUG_M(", ok");
        // gets first token in expr TODO
        // this will not be used when really switching
        token = get_next_token(ctx);
        debug_token(token);
        // current token is first in expression
        // TODO switch to bottom up
        // simulating expression evaluation, skipping `e`
        token = get_next_token(ctx);
        LOG_DEBUG_M("simulated next expr");

        token = e_1(token, ctx);

        return token;
    } else if (token.type == IDENTIFIER) {
        LOG_DEBUG_M("id unget and return");
        // TODO these ungets will be obsolete by bottom up, right?
        // or we just get next after calling bottom up to get the next
        // and unget that, if bottom up doesn't return token
        unget_token(token);
        return token;

    } else if (token.type == KEYWORD) {
        if (*token.keyword == KW_RETURN) {
            LOG_DEBUG_M("return unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_END) {
            LOG_DEBUG_M("end unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_LOCAL) {
            LOG_DEBUG_M("local unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_IF) {
            LOG_DEBUG_M("if unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_ELSE) {
            LOG_DEBUG_M("else unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_WHILE) {
            LOG_DEBUG_M("while unget and return");
            unget_token(token);
            return token;
        }
    }

    exit(ESYNTAX);
}

static token_t e_list(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);
#if 1
    // TODO switch to bottom up
    // get_next_token()?
    // it returns after analyzing expr
    // all checks are done in bottom up?

    // simulates expression eval by bottom up
    LOG_DEBUG_M("simulated first expr");
    token = get_next_token(ctx);
#endif

    // TODO
    //enum variable_type expr_type;
    //expr_type = expr_parser_start(ctx);

    token = e_1(token, ctx);

    return token;

    exit(ESYNTAX);
}

static token_t ret_e_list(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    // here saved id is the function we are in
    // can be used to retrive `retval` for checking return types
    LOG_DEBUG("ret_e_list() with saved_id: %s", ctx->saved_id->name);

    // TODO
    // switch and get an expression
    // here, token can be `id` and I don't know how to recognize
    // if to switch to bottom up or it is start of ID_SEQ or CALL
    //
    // ID_SEQ will be handled by bottom up, because it's just
    // simple expressions divided by commas
    // so we switch every RHS except call, which can be recognized from type

    // get token for e_list()
    token = get_next_token(ctx);

    token = e_list(token, ctx);

    // bottom-up will unget first not-expression token so it is available here

    debug_token(token);
    return token;

    exit(ESYNTAX);
}

static token_t ret_stmt(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == KEYWORD) {
        if (*token.keyword == KW_RETURN) {
            LOG_DEBUG_M("return ok");

            token = ret_e_list(token, ctx);

            return token;
        }
    }

    exit(ESYNTAX);
}

static token_t id_seq_1(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == COMMA) {
        LOG_DEBUG_M(", ok");
        token = get_next_token(ctx);

        if (token.type == IDENTIFIER) {
            if (token.identifier->type != VARIABLE) {
                LOG_ERROR("%s is not an existing variable", token.identifier->name);
                exit(EDEF);
            }
            LOG_DEBUG_M("id ok");
            debug_token(token);
            string_appendc(ctx->param, token.identifier->var.type);
            token = get_next_token(ctx);
            token = id_seq_1(token, ctx);
            return token;
        }
    } else if (token.type == ASSIGNMENT) {
        LOG_DEBUG_M("= unget and return");
        unget_token(token);
        return token;
    }
    exit(ESYNTAX);
}

static token_t id_seq(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == IDENTIFIER) {
        if (token.identifier->type != VARIABLE) {
            LOG_ERROR("%s is not an existing variable", token.identifier->name);
            exit(EDEF);
        }
        LOG_DEBUG_M("id ok");
        debug_token(token);
        // TODO
        string_appendc(ctx->param, token.identifier->var.type);
        token = get_next_token(ctx);
        token = id_seq_1(token, ctx);
        return token;
    }

    exit(ESYNTAX);
}

static token_t stmt(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == IDENTIFIER) {
        LOG_DEBUG_M("id diving");

        token = id_seq(token, ctx);

        LOG_DEBUG("id_seq() returned LSH: '%s'\n",
                string_expose(ctx->param));
        token = get_next_token(ctx);

        if (token.type != ASSIGNMENT)
            exit(ESYNTAX);
        LOG_DEBUG_M("= ok");

        token = get_next_token(ctx);

        token = e_list(token, ctx); // TODO where to switch?

        return token;
    }

    exit(ESYNTAX);
}

static token_t var_assign(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == ASSIGNMENT) {
        // TODO switch to bottom up, expecting single expression
        // simulating expression, skip `e`
        token = get_next_token(ctx);
        LOG_DEBUG_M("simulating expression");

        LOG_DEBUG_M("expr ok");
        debug_token(token);
        return token;
    } else if (token.type == IDENTIFIER) {
            LOG_DEBUG_M("id unget and return");
            unget_token(token);
            return token;

    } else if (token.type == KEYWORD) {
        if (*token.keyword == KW_RETURN) {
            LOG_DEBUG_M("return unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_END) {
            LOG_DEBUG_M("end unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_LOCAL) {
            LOG_DEBUG_M("local unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_IF) {
            LOG_DEBUG_M("if unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_ELSE) {
            LOG_DEBUG_M("else unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_WHILE) {
            LOG_DEBUG_M("while unget and return");
            unget_token(token);
            return token;
        }
    }

    exit(ESYNTAX);
}

static token_t var_dec(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    symtable_t *symtable = symstack_global_symtable(ctx->symstack);
    identifier_t *backup = ctx->saved_id;

    if (token.type == KEYWORD) {
        if (*token.keyword == KW_LOCAL) {
            LOG_DEBUG_M("local ok");
            token = get_next_token(ctx);
            if (token.type == IDENTIFIER) {
                ctx->saved_id = token.identifier;

                if (symtable_find(symtable, ctx->saved_id->name)) {
                    LOG_ERROR("%s is already a function", ctx->saved_id->name);
                    exit(EDEF);
                }
                symstack_most_local(ctx->symstack);
                symtable = symstack_get(ctx->symstack);
                if (symtable_find(symtable, ctx->saved_id->name) && ctx->saved_id->type == VARIABLE) {
                    LOG_ERROR("%s is already a variable", ctx->saved_id->name);
                    exit(EDEF);
                }
                ctx->saved_id->type = VARIABLE;

                LOG_DEBUG_M("id ok");
                debug_token(token);
                token = get_next_token(ctx);
                if (token.type == TYPE_SPEC) {
                    LOG_DEBUG_M(": ok");
                    token = get_next_token(ctx);
                    token = type(token, ctx);
                    debug_identifier(ctx->saved_id);
                    ctx->saved_id = backup;
                    return token;
                }
            }
        }
    }

    exit(ESYNTAX);
}

static token_t var_dec_def(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == KEYWORD) {
        if (*token.keyword == KW_LOCAL) {
            LOG_DEBUG_M("local diving");
            token = var_dec(token, ctx);
            token = var_assign(token, ctx);
            return token;
        }
    }

    exit(ESYNTAX);
}

// forward declaration
static token_t if_(token_t token, context_t *ctx);
static token_t while_(token_t token, context_t *ctx);

static token_t body_1(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == IDENTIFIER) {
        LOG_DEBUG_M("id diving");
        token = stmt(token, ctx);
        LOG_DEBUG_M("returning from stmt");
        debug_token(token);
        return token;

    } else if (token.type == KEYWORD) {
        if (*token.keyword == KW_RETURN) {
            LOG_DEBUG_M("return diving");
            token = ret_stmt(token, ctx);
            return token;

        } else if (*token.keyword == KW_LOCAL) {
            LOG_DEBUG_M("local diving");
            token = var_dec_def(token, ctx);
            return token;

        } else if (*token.keyword == KW_IF) {
            LOG_DEBUG_M("if diving");
            token = if_(token, ctx);
            return token;

        } else if (*token.keyword == KW_WHILE) {
            LOG_DEBUG_M("while diving");
            token = while_(token, ctx);
            return token;
        }
    }

    exit(ESYNTAX);
}

static token_t body_2(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == IDENTIFIER) {
        LOG_DEBUG_M("id diving");
        token = body_1(token, ctx);
        token = get_next_token(ctx);
        token = body_2(token, ctx);
        return token;

    } else if (token.type == KEYWORD) {
        if (*token.keyword == KW_RETURN) {
            LOG_DEBUG_M("return diving");
            token = body_1(token, ctx);
            token = get_next_token(ctx);
            token = body_2(token, ctx);
            return token;

        } else if (*token.keyword ==  KW_LOCAL) {
            LOG_DEBUG_M("local diving");
            token = body_1(token, ctx);
            token = get_next_token(ctx);
            token = body_2(token, ctx);
            return token;

        } else if (*token.keyword == KW_IF) {
            LOG_DEBUG_M("if diving");
            token = body_1(token, ctx);
            token = get_next_token(ctx);
            token = body_2(token, ctx);
            return token;

        } else if (*token.keyword == KW_WHILE) {
            LOG_DEBUG_M("while diving");
            token = body_1(token, ctx);
            token = get_next_token(ctx);
            token = body_2(token, ctx);
            return token;

        } else if (*token.keyword == KW_END) {
            LOG_DEBUG_M("end unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_ELSE) {
            LOG_DEBUG_M("else unget and return");
            unget_token(token);
            return token;
        }
    }

    exit(ESYNTAX);
}

static token_t body(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == IDENTIFIER) {
        LOG_DEBUG_M("identifier body diving");
        token = body_1(token, ctx);
        token = get_next_token(ctx);
        token = body_2(token, ctx);
        return token;

    } else if (token.type == KEYWORD) {
        if (*token.keyword == KW_RETURN) {
            LOG_DEBUG_M("return body diving");
            token = body_1(token, ctx);
            token = get_next_token(ctx);
            token = body_2(token, ctx);
            return token;

        } else if (*token.keyword == KW_LOCAL) {
            LOG_DEBUG_M("local body diving");
            token = body_1(token, ctx);
            token = get_next_token(ctx);
            token = body_2(token, ctx);
            return token;

        } else if (*token.keyword == KW_IF) {
            LOG_DEBUG_M("if body diving");
            token = body_1(token, ctx);
            token = get_next_token(ctx);
            token = body_2(token, ctx);
            return token;

        } else if (*token.keyword == KW_WHILE) {
            LOG_DEBUG_M("while body diving");
            token = body_1(token, ctx);
            token = get_next_token(ctx);
            token = body_2(token, ctx);
            return token;

        } else if (*token.keyword == KW_ELSE) {
            LOG_DEBUG_M("else unget and return");
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_END) {
            LOG_DEBUG_M("end unget and return");
            unget_token(token);
            return token;
        }
    }
    exit(ESYNTAX);
}

static token_t if_(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    symtable_t *then_symtable = symtable_create();
    symtable_t *else_symtable = symtable_create();
    symtable_t *popped;

    if (token.type == KEYWORD) {
        if (*token.keyword == KW_IF) {
            LOG_DEBUG_M("if ok");
            // TODO switch for expression
            // simulate expression, skip `e`
            token = get_next_token(ctx);
            debug_token(token);

            token = get_next_token(ctx);
            if (token.type != KEYWORD)
                exit(ESYNTAX);
            if (*token.keyword != KW_THEN)
                exit(ESYNTAX);

            LOG_DEBUG_M("then ok");

            // local symtable for new block
            symstack_push(ctx->symstack, then_symtable);

            token = get_next_token(ctx);

            token = body(token, ctx);

            token = get_next_token(ctx);

            popped = symstack_pop(ctx->symstack);
            if (popped != then_symtable) {
                LOG_ERROR_M("Wrong popped symtable at `then` block end.");
                exit(EINTERNAL);
            }
            symtable_destroy(then_symtable);

            if (token.type != KEYWORD)
                exit(ESYNTAX);
            if (*token.keyword != KW_ELSE)
                exit(ESYNTAX);

            LOG_DEBUG_M("else ok");

            // local symtable for new block
            symstack_push(ctx->symstack, else_symtable);

            token = get_next_token(ctx);

            token = body(token, ctx);

            token = get_next_token(ctx);

            popped = symstack_pop(ctx->symstack);
            if (popped != else_symtable) {
                LOG_ERROR_M("Wrong popped symtable at `else` block end");
                exit(EINTERNAL);
            }
            symtable_destroy(else_symtable);

            if (token.type != KEYWORD)
                exit(ESYNTAX);
            if (*token.keyword != KW_END)
                exit(ESYNTAX);

            LOG_DEBUG_M("end ok");

            return token;
        }
    }

    exit(ESYNTAX);
}

static token_t while_(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    symtable_t *local_symtable = symtable_create();
    symtable_t *popped;

    if (token.type == KEYWORD) {
        if (*token.keyword == KW_WHILE) {
            LOG_DEBUG_M("while ok");
            // TODO switch for expression
            // simulate expression evaluation, skip `e`
            token = get_next_token(ctx);

            token = get_next_token(ctx);
            if (token.type != KEYWORD)
                exit(ESYNTAX);
            if (*token.keyword != KW_DO)
                exit(ESYNTAX);
            LOG_DEBUG_M("do ok");

            // local symtable for new block
            symstack_push(ctx->symstack, local_symtable);

            token = get_next_token(ctx);

            token = body(token, ctx);

            token = get_next_token(ctx);

            popped = symstack_pop(ctx->symstack);
            if (popped != local_symtable) {
                LOG_ERROR_M("Wrong popped symtable at `while` block end.");
                exit(EINTERNAL);
            }
            symtable_destroy(local_symtable);

            if (token.type != KEYWORD)
                exit(ESYNTAX);
            if (*token.keyword != KW_END)
                exit(ESYNTAX);
            LOG_DEBUG_M("end ok");

            return token;
        }
    }

    exit(ESYNTAX);
}

static token_t param(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    symtable_t *global = symstack_global_symtable(ctx->symstack);

    if (token.type == IDENTIFIER) {
        // TODO check id is not already in global symtable
        // not saving because we would overwrite function name from fun_def()
        if (symtable_find(global, token.identifier->name)) {
            LOG_ERROR("%s is already a function", token.identifier->name);
            exit(EDEF);
        }

        LOG_DEBUG_M("id ok");
        debug_token(token);

        token.identifier->type = VARIABLE;

        token = get_next_token(ctx);

        if (token.type == TYPE_SPEC) {
            LOG_DEBUG_M(": ok");
            token = get_next_token(ctx);
            
            if (token.type == KEYWORD) {
                if (*token.keyword == KW_INTEGER) {
                    LOG_DEBUG_M("integer diving");
                    token = type(token, ctx);
                    string_appendc(ctx->param, 'i');
                    return token;
                } else if (*token.keyword == KW_NUMBER) {
                    LOG_DEBUG_M("number diving");
                    token = type(token, ctx);
                    string_appendc(ctx->param, 'n');
                    return token;
                } else if (*token.keyword == KW_STRING) {
                    LOG_DEBUG_M("string diving");
                    token = type(token, ctx);
                    string_appendc(ctx->param, 's');
                    return token;
                }
            }
        }
    }

    exit(ESYNTAX);
}

static token_t param_1(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == COMMA) {
        LOG_DEBUG_M(", ok");
        token = get_next_token(ctx);
        // here token is id, added to local symtable
        token = param(token, ctx);
        token = param_1(token, ctx);
        return token;

    } else if (token.type == RIGHT_PAR) {
        LOG_DEBUG_M(") unget and return");
        unget_token(token);
        return token;
    }

    exit(ESYNTAX);
}

static token_t param_list(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == IDENTIFIER) {
        LOG_DEBUG_M("id diving");
        token = param(token, ctx);
        token = param_1(token, ctx);
        return token;

    } else if (token.type == RIGHT_PAR) {
        LOG_DEBUG_M(") unget and return");
        unget_token(token);
        return token;
    }

    exit(ESYNTAX);
}

static token_t fun_def(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    identifier_t *function_id;

    if (token.type == KEYWORD) {
        if (*token.keyword == KW_FUNCTION) {
            LOG_DEBUG_M("function ok");
            token = get_next_token(ctx);

            if (token.type == IDENTIFIER) {
                debug_token(token);
                LOG_DEBUG_M("id ok");

                ctx->saved_id = token.identifier;
                function_id = token.identifier;

                debug_identifier(ctx->saved_id);

                token = get_next_token(ctx);

                if (token.type != LEFT_PAR)
                    exit(ESYNTAX);
                LOG_DEBUG_M("( ok");

                symtable_t *local_symtable = symtable_create();
                symstack_push(ctx->symstack, local_symtable);

                token = get_next_token(ctx);

                token = param_list(token, ctx);
                LOG_DEBUG("param from param_list() '%s'", string_expose(ctx->param));

                token = get_next_token(ctx);

                if (token.type != RIGHT_PAR)
                    exit(ESYNTAX);
                LOG_DEBUG_M(") ok");

                token = get_next_token(ctx);

                token = fun_ret(token, ctx);
                LOG_DEBUG("retval from fun_ret() '%s'", string_expose(ctx->retval));

                // export, check and set bot param and retval
                // before we dive into body
                if (ctx->saved_id->type == FUNCTION) {
                    if (ctx->saved_id->fun.defined) {
                        LOG_ERROR("%s() redefinition", ctx->saved_id->name);
                        exit(EDEF);
                    }
                    if (strcmp(ctx->saved_id->fun.param, string_expose(ctx->param))) {
                        LOG_ERROR("%s() parameters do not match declaration", ctx->saved_id->name);
                        exit(EDEF);
                    }
                    if (strcmp(ctx->saved_id->fun.retval, string_expose(ctx->retval))) {
                        LOG_ERROR("%s() return values do not match declaration", ctx->saved_id->name);
                        exit(EDEF);
                    }
                    // function matches its declaration
                    LOG_DEBUG("%s() matches declaration", ctx->saved_id->name);
                } else if (ctx->saved_id->type == VARIABLE) {
                    LOG_ERROR("%s() is already a variable", ctx->saved_id->name);
                    exit(EDEF);
                } else {
                    // no declaration preceded, set id to function
                    ctx->saved_id->type = FUNCTION;
                    LOG_DEBUG("%s() set as function", ctx->saved_id->name);
                }
                ctx->saved_id->fun.param = string_export(ctx->param);
                ctx->saved_id->fun.retval = string_export(ctx->retval);
                string_clear(ctx->param);
                string_clear(ctx->retval);

                token = get_next_token(ctx);

                // TODO
                ctx->saved_id = function_id;
                // body changes ctx->saved_id !
                token = body(token, ctx);

                token = get_next_token(ctx);

                if (token.type == KEYWORD) {
                    if (*token.keyword == KW_END) {
                        LOG_DEBUG_M("end returning");

                        symtable_t *popped = symstack_pop(ctx->symstack);
                        if (popped != local_symtable) {
                            LOG_ERROR_M("Wrong popped symtable at `function` block end");
                            exit(EINTERNAL);
                        }
                        symtable_destroy(local_symtable);

                        // saved function identifer because ctx->saved_id is rewritten by body
                        function_id->fun.defined = 1; 
                        debug_identifier(function_id);

                        return token;
                    }
                }
            }
        }
    }

    exit(ESYNTAX);
}

static token_t fun_signature(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == KEYWORD) {
        if (*token.keyword == KW_FUNCTION) {

            ctx->saved_id->type = FUNCTION;

            LOG_DEBUG_M("function ok");

            token = get_next_token(ctx);

            if (token.type != LEFT_PAR)
                exit(ESYNTAX);
            LOG_DEBUG_M("( ok");

            token = get_next_token(ctx);

            token = type_list(token, ctx);

            token = get_next_token(ctx);

            if (token.type != RIGHT_PAR)
                exit(ESYNTAX);
            LOG_DEBUG_M(") ok");

            token = get_next_token(ctx);

            token = fun_ret(token, ctx);

            ctx->saved_id->fun.param = string_export(ctx->param);
            ctx->saved_id->fun.retval = string_export(ctx->retval);
            string_clear(ctx->param);
            string_clear(ctx->retval);

            return token;
        }
    }

    exit(ESYNTAX);
}

static token_t fun_dec(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == KEYWORD) {
        if (*token.keyword == KW_GLOBAL) {
            LOG_DEBUG_M("global ok");
            token = get_next_token(ctx);
            debug_token(token);

            if (token.type == IDENTIFIER) {
                LOG_DEBUG_M("id  ok");

                ctx->saved_id = token.identifier;

                symtable_t *global_symtable = symstack_global_symtable(ctx->symstack);
                if (symtable_find(global_symtable, ctx->saved_id->name) && ctx->saved_id->type == FUNCTION) {
                    LOG_DEBUG("%s() is already a function", ctx->saved_id->name);
                    exit(EDEF);
                }

                token = get_next_token(ctx);

                if (token.type == TYPE_SPEC) {
                    LOG_DEBUG_M(":  ok");
                    token = get_next_token(ctx);
                    token = fun_signature(token, ctx);
                    LOG_DEBUG_M("returning from fun_dec with");
                    debug_token(token);
                    debug_identifier(ctx->saved_id);
                    return token;
                }
            }
        }
    }

    exit(ESYNTAX);
}

static token_t code_1(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    symtable_t *global = symstack_global_symtable(ctx->symstack);

    if (token.type == KEYWORD) {
        if (*token.keyword == KW_GLOBAL) {
            LOG_DEBUG_M("global diving");
            token = fun_dec(token, ctx);
            return token;

        } else if (*token.keyword == KW_FUNCTION) {
            LOG_DEBUG_M("function diving");
            token = fun_def(token, ctx);
            return token;
        }
    } else if (token.type == IDENTIFIER) {
        // TODO check it is a defined function
        if (!symtable_find(global, token.identifier->name)) {
            LOG_ERROR("%s() is not defined/declared function, cannot be called",
                    token.identifier->name);
            exit(EDEF);
        }
        LOG_DEBUG_M("id diving");
        token = call(token, ctx);
        return token;
    }

    exit(ESYNTAX);
}

static token_t code_2(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == KEYWORD) {
        if (*token.keyword == KW_GLOBAL) {
            LOG_DEBUG_M("global diving");
            token = code_1(token, ctx);
            token = get_next_token(ctx);
            token = code_2(token, ctx);
            return token;

        } else if (*token.keyword == KW_FUNCTION) {
            LOG_DEBUG_M("function diving");
            token = code_1(token, ctx);
            token = get_next_token(ctx);
            token = code_2(token, ctx);
            return token;
        }
    } else if (token.type == IDENTIFIER) {
        LOG_DEBUG_M("id diving");
        token = code_1(token, ctx);
        token = get_next_token(ctx);
        token = code_2(token, ctx);
        return token;

    } else if (token.type == END) {
        LOG_DEBUG_M("$ returning");
        return token;
    }

    exit(ESYNTAX);

}

static token_t code(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == KEYWORD) {
        if (*token.keyword == KW_GLOBAL) {
            LOG_DEBUG_M("global diving");
            token = code_1(token, ctx);
            token = get_next_token(ctx);
            token = code_2(token, ctx);
            return token;

        } else if (*token.keyword == KW_FUNCTION) {
            LOG_DEBUG_M("function diving");
            token = code_1(token, ctx);
            token = get_next_token(ctx);
            token = code_2(token, ctx);
            return token;
        }
    } else if (token.type == IDENTIFIER) {
        LOG_DEBUG_M("id diving");
        token = code_1(token, ctx);
        token = get_next_token(ctx);
        token = code_2(token, ctx);
        return token;

    } else if (token.type == END) {
        LOG_DEBUG_M("$ returning");
        return token;
    }

    exit(ESYNTAX);
}

static token_t require(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    if (token.type == KEYWORD) {
        if (*token.keyword == KW_REQUIRE) {
            LOG_DEBUG_M("require ok");
            token = get_next_token(ctx);
            if (token.type == STRING) {
                if(!strcmp(token.string, "ifj21")) {
                    LOG_DEBUG_M("\"ifj21\" ok");
                    free(token.string);
                    token = get_next_token(ctx);
                    return token;
                }
            }
        }
    }

    exit(ESYNTAX);
}

static int prog(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    token = require(token, ctx);

    if (token.type == END) {
        LOG_DEBUG_M("$ diving");
        token = code(token, ctx);
        return 0;

    } else if (token.type == KEYWORD) {
        if (*token.keyword == KW_GLOBAL) {
            LOG_DEBUG_M("global diving");
            token = code(token, ctx);
            return 0;

        } else if (*token.keyword == KW_FUNCTION) {
            LOG_DEBUG_M("function diving");
            token = code(token, ctx);
            return 0;
        }
    } else if (token.type == IDENTIFIER) {
        LOG_DEBUG_M("id diving");
        token = code(token, ctx);
        return 0;
    }

    exit(ESYNTAX);
}

void parser_start(context_t *ctx)
{
    token_t token;

    token = get_next_token(ctx);

    // starting non-terminal of recursive descent
    prog(token, ctx);

    LOG_DEBUG_M("SUCCESSFUL SYNTAX ANALYSIS");
}


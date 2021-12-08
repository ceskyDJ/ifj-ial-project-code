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
#include "kwtable.h"
#define LOG_LEVEL DEBUG
#include "logger.h"
#include "scanner.h"
#include "string_factory.h"
#include "symtable.h"
#include "symstack.h"
#include "token.h"

#include "expr_parser.h"
#include "generator.h"
#include "symqueue.h"

#include <assert.h>
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
    if (!id)
        return;
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

// TODO stolen from expr_parser.c
static bool is_valid_variable(context_t *ctx, token_t *token)
{
    symtable_t *symtable;
    identifier_t *tmp_id;

    // Go through all tables of symbols and try to find checked identifier with type (declared variable)
    symstack_most_local(ctx->symstack);
    while (symstack_is_active(ctx->symstack)) {
        symtable = symstack_get(ctx->symstack);
        tmp_id = symtable_find(symtable, token->identifier->name);

        if (tmp_id && tmp_id->type == VARIABLE) {
            // It's a valid variable, so we are done here
            token->identifier = tmp_id;
            return true;
        }

        symstack_next(ctx->symstack);
    }

    return false;
}

static token_t term(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);
    debug_identifier(ctx->saved_id);
    int param_index;
    bool implicit_conv = false;
    enum variable_type converted;

    if (token.type == IDENTIFIER) {
        if (!is_valid_variable(ctx, &token)) {
            LOG_ERROR("%s is not an existing variable", token.identifier->name);
            exit(EDEF);
        }

        converted = token.identifier->var.type;

        if (token.identifier->var.type == VAR_INTEGER) {
            param_index = strlen(string_expose(ctx->param));
            if (ctx->saved_id->fun.param[param_index] == VAR_NUMBER) {
                LOG_DEBUG("will do implicit conv for variable '%s'",
                        token.identifier->name);
                implicit_conv = true;
                // TODO do not rewrite it like this because we would generate
                // number when really the value is integer
                //token.identifier->var.type = VAR_NUMBER;
                converted = VAR_NUMBER;
            }
        }

        LOG_DEBUG_M("id ok");
        debug_identifier(token.identifier);
        string_appendc(ctx->param, converted);

        if (!strcmp(ctx->saved_id->fun.param, "write")) {
            gen_write_identifier(token.identifier);
        } else {
            gen_call_param(&token, implicit_conv);
        }


        token = get_next_token(ctx);
        return token;

    } else if (token.type == INTEGER) {
        LOG_DEBUG_M("TYPE integer ok");
        // TODO implicit conv to number

        param_index = strlen(string_expose(ctx->param));
        if (ctx->saved_id->fun.param[param_index] == VAR_NUMBER) {
                LOG_DEBUG("will do implicit conv for integer term '%d'",
                        token.integer);
                implicit_conv = true;
                //token.type = NUMBER;
                string_appendc(ctx->param, VAR_NUMBER);
        } else {
            string_appendc(ctx->param, VAR_INTEGER);
        }

        if (!strcmp(ctx->saved_id->fun.param, "write")) {
            gen_write_integer(token.integer);
        } else {
            gen_call_param(&token, implicit_conv);
        }

        token = get_next_token(ctx);
        return token;

    } else if (token.type == NUMBER) {
        LOG_DEBUG_M("TYPE number ok");
        string_appendc(ctx->param, VAR_NUMBER);

        if (!strcmp(ctx->saved_id->fun.param, "write")) {
            gen_write_number(token.number);
        } else {
            gen_call_param(&token, false);
        }

        token = get_next_token(ctx);
        return token;

    } else if (token.type == STRING) {
        LOG_DEBUG_M("TYPE string ok");
        string_appendc(ctx->param, VAR_STRING);

        if (!strcmp(ctx->saved_id->fun.param, "write")) {
            gen_write_string(token.string);
        } else {
            gen_call_param(&token, false);
        }
        // is string not used after this?
        // code using it is generated, so I think
        // we can free it
        free(token.string);

        token = get_next_token(ctx);
        return token;

    } else if (token.type == KEYWORD) {
        if (*token.keyword == KW_NIL) {
            // TODO nil as term in function call other than write()?
            if (!strcmp(ctx->saved_id->fun.param, "write")) {
                gen_write_nil();
            }
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

    identifier_t *fun_id;

    if (token.type == IDENTIFIER) {
        if (token.identifier->type == FUNCTION) {
            LOG_DEBUG_M("id function ok");

            fun_id = token.identifier;

            gen_create_frame();

            token = get_next_token(ctx);

            if (token.type != LEFT_PAR)
                exit(ESYNTAX);

            LOG_DEBUG_M("( ok");

            token = get_next_token(ctx);
            debug_token(token);

            // TODO again implicit conv integer->number
            token = term_seq(token, ctx);

            if (!strcmp(fun_id->fun.param, "write")) {
                LOG_DEBUG_M("Function write does not need signature check");
            } else if (strcmp(string_expose(ctx->param), fun_id->fun.param)) {
                LOG_ERROR("Parameters '%s' do not match what %s() takes: '%s'",
                        string_expose(ctx->param),
                        fun_id->name,
                        fun_id->fun.param);
                exit(EFUNCALL);
            }
            LOG_DEBUG("after term_seq(): param '%s'",
                    string_expose(ctx->param));

            string_clear(ctx->param);

            token = get_next_token(ctx);

            if (token.type != RIGHT_PAR)
                exit(ESYNTAX);

            LOG_DEBUG_M(") ok");

            // write() is generated by calls for each term,
            // so don't call it as a whole
            if (strcmp(fun_id->fun.param, "write")) {
                gen_call(fun_id);
            }

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
                LOG_DEBUG("var '%s' set to integer", ctx->saved_id->name);
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
                string_appendc(ctx->retval, VAR_INTEGER);
                // TODO do not gen in dec
                if (ctx->saved_id->type == FUNCTION)
                    gen_var_retval();
                token = fun_ret_list_1(token, ctx);
                return token;

            } else if (*token.keyword == KW_NUMBER) {
                LOG_DEBUG_M("number diving");
                token = type(token, ctx);
                string_appendc(ctx->retval, VAR_NUMBER);
                if (ctx->saved_id->type == FUNCTION)
                    gen_var_retval();
                token = fun_ret_list_1(token, ctx);
                return token;

            } else if (*token.keyword == KW_STRING) {
                LOG_DEBUG_M("string diving");
                token = type(token, ctx);
                string_appendc(ctx->retval, VAR_STRING);
                if (ctx->saved_id->type == FUNCTION)
                    gen_var_retval();
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
            string_appendc(ctx->retval, VAR_INTEGER);
            // TODO don't generate if we are in declaration
            LOG_DEBUG("fun_ret_list() integer: saved_id->type: %d", ctx->saved_id->type);
            debug_identifier(ctx->saved_id);
            if (ctx->saved_id->type == FUNCTION)
                gen_var_retval();
            token = fun_ret_list_1(token, ctx);
            return token;

        } else if (*token.keyword == KW_NUMBER) {
            LOG_DEBUG_M("number diving");
            token = type(token, ctx);
            string_appendc(ctx->retval, VAR_NUMBER);
            if (ctx->saved_id->type == FUNCTION)
                gen_var_retval();
            token = fun_ret_list_1(token, ctx);
            return token;

        } else if (*token.keyword == KW_STRING) {
            LOG_DEBUG_M("string diving");
            token = type(token, ctx);
            string_appendc(ctx->retval, VAR_STRING);
            if (ctx->saved_id->type == FUNCTION)
                gen_var_retval();
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
    enum variable_type expr_type;
    int retval_index;

    if (token.type == COMMA) {
        LOG_DEBUG_M(", ok");

        LOG_DEBUG_M("e_1() saved_id is");
        debug_identifier(ctx->saved_id);

        expr_type = expr_parser_start(ctx);
        LOG_DEBUG("expr is '%c'", expr_type);

        // TODO the implicit conv MUST work for both functions
        // and expression lists

        retval_index = strlen(string_expose(ctx->retval));
        LOG_DEBUG("e_1() retval_index: %d, expr: %c, retval: '%s'",
                retval_index,
                expr_type,
                string_expose(ctx->retval));

        if (ctx->saved_id->fun.retval[retval_index] == VAR_NUMBER &&
                expr_type == VAR_INTEGER) {
            LOG_DEBUG_M("will do implicit conv for expr result");
            gen_conv_to_number_top();
            expr_type = VAR_NUMBER;
        }

        string_appendc(ctx->retval, expr_type);

        gen_var_active_assign(ctx->symqueue, true);

        token = get_next_token(ctx);

        token = e_1(token, ctx);

        return token;
    } else if (token.type == IDENTIFIER) {
        LOG_DEBUG_M("id unget and return");
        // TODO these ungets will be obsolete by bottom up, right?
        // it doesn't seem so, but look at it later
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
    enum variable_type expr_type;
    int retval_index;

    token = get_next_token(ctx);
    if (token.type == KEYWORD) {
        LOG_DEBUG_M("empty return statement");
        unget_token(token);
        return token;
    }
    unget_token(token);

    expr_type = expr_parser_start(ctx);
    LOG_DEBUG("expr is '%c'", expr_type);

    retval_index = strlen(string_expose(ctx->retval));
    LOG_DEBUG("e_list() after first expr, retval '%s' len: %d",
            string_expose(ctx->retval),
            retval_index);

    // conv if corresponding LHS type is number and we are integer
    if (ctx->saved_id->fun.retval[retval_index] == VAR_NUMBER &&
            expr_type == VAR_INTEGER) {
        LOG_DEBUG_M("will do implicit conv for expr result");
        gen_conv_to_number_top();
        expr_type = VAR_NUMBER;
    }


    string_appendc(ctx->retval, expr_type);
    LOG_DEBUG_M("e_list: saved_id:");
    debug_identifier(ctx->saved_id);

    // TODO  bottom up prepared value on stack
    // I believe there must be the same number of expressions
    // as there are variables on LHS
    gen_var_active_assign(ctx->symqueue, true);

    token = get_next_token(ctx);

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

    int retval_len = (int)strlen(ctx->saved_id->fun.retval);
    // this arrays is local, used for creating "fake" identifiers
    // to put '%retval_n' to symqueue
    // all of them should be popped and used in e_list()
    // thus it should be safe to do it like this
    char retval_names[retval_len][20];

    identifier_t retval_ids[retval_len];
    for (int i = 0; i < retval_len; i++) {
        retval_ids[i].line = 0;
        retval_ids[i].character = 0;
    }

    for (int i = 0; i < retval_len; i++) {
        snprintf(retval_names[i], 20, "%%retval_%d", i+1);
        retval_ids[i].name = retval_names[i];
        symqueue_add(ctx->symqueue, &retval_ids[i]);
        LOG_DEBUG("added '%s' to symqueue", retval_names[i]);
    }

    token = e_list(token, ctx);

    // TODO implicit conv if returning integer when number expected
    LOG_DEBUG("ret_e_list: retval '%s'", string_expose(ctx->retval));
    debug_identifier(ctx->saved_id);

    if (strcmp(string_expose(ctx->retval), ctx->saved_id->fun.retval)) {
        LOG_ERROR("Return statement(s) '%s' do not match function return values '%s'",
                string_expose(ctx->retval), ctx->saved_id->fun.retval);
        exit(EFUNCALL);
    }

    string_clear(ctx->retval);

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
            gen_var_set_active(token.identifier, ctx->symqueue);
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
        if (!is_valid_variable(ctx, &token)) {
            LOG_ERROR("%s is not an existing variable", token.identifier->name);
            exit(EDEF);
        }

        LOG_DEBUG_M("id ok");
        debug_token(token);

        gen_var_set_active(token.identifier, ctx->symqueue);
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
    char *saved_LHS;
    identifier_t *backup;

    if (token.type == IDENTIFIER) {
        LOG_DEBUG_M("id diving");

        token = id_seq(token, ctx);

        LOG_DEBUG("id_seq() returned LSH: '%s'\n",
                string_expose(ctx->param));
        saved_LHS = string_export(ctx->param);

        int saved_LHS_len = strlen(saved_LHS);
        bool conv_returned[saved_LHS_len];
        for (int i = 0; i < saved_LHS_len; i++)
            conv_returned[i] = false;

        token = get_next_token(ctx);

        if (token.type != ASSIGNMENT)
            exit(ESYNTAX);
        LOG_DEBUG_M("= ok");

        token = get_next_token(ctx);
        debug_token(token);

        if (token.type == IDENTIFIER) {
            string_clear(ctx->param);
            identifier_t *fun_id = symtable_find(symstack_global_symtable(ctx->symstack),
                                                token.identifier->name);
            if (fun_id) {
                if ((int)strlen(fun_id->fun.retval) < saved_LHS_len) {
                    LOG_ERROR("%s():'%s' does not return enough values to fill LHS '%s'",
                            fun_id->name,
                            fun_id->fun.retval,
                            saved_LHS);
                    exit(EFUNCALL);
                }
                // check LHS (saved in ctx->param) is prefix of function's retval
                LOG_DEBUG("LHS '%s' (ctx->param '%s') %s() retval '%s'",
                        saved_LHS, string_expose(ctx->param), fun_id->name, fun_id->fun.retval);

                for (int i = 0; i < saved_LHS_len; i++) {
                    if (saved_LHS[i] == VAR_NUMBER && fun_id->fun.retval[i] == VAR_INTEGER) {
                        conv_returned[i] = true;
                        LOG_DEBUG("will apply implicit conv at index %d", i);
                    } else if (saved_LHS[i] == fun_id->fun.retval[i]) {
                        continue;
                    } else {
                        LOG_ERROR("LHS '%s' is not compatible with %s() return values '%s'",
                                saved_LHS,
                                fun_id->name,
                                fun_id->fun.retval);
                        exit(EFUNCALL);
                    }
                }
                LOG_DEBUG("ok, we can call %s()", fun_id->name);

                token.identifier = fun_id;
                backup = ctx->saved_id;
                ctx->saved_id = fun_id;
                token = call(token, ctx);
                ctx->saved_id = backup;

                for (int i = 0; i < saved_LHS_len; i++) {
                    gen_returned_assign(ctx->symqueue, conv_returned[i]);
                }

                assert(symqueue_is_empty(ctx->symqueue));
                free(saved_LHS);

                return token;
            }
        }

        // RHS is expression list
        unget_token(token);

        // make fake identifier and save it in context for e_list()
        // so it thinks we are in a function that returns our LHS
        backup = ctx->saved_id;
        identifier_t fake_fun = {
            .type = FUNCTION,
            .name = "$fake_fun_LHS$",
            .fun = {
                .retval = saved_LHS,
            },
        };
        ctx->saved_id = &fake_fun;

        token = e_list(token, ctx);
        ctx->saved_id = backup;

        LOG_DEBUG("stmt analyzed: '%s' = '%s'",
                saved_LHS,
                string_expose(ctx->retval));

        if (strcmp(saved_LHS, string_expose(ctx->retval))) {
            LOG_ERROR("LHS types '%s' do not match RHS types '%s'",
                    saved_LHS,
                    string_expose(ctx->retval));
            exit(EASSIGN); // TODO correct exit code?
        }

        assert(symqueue_is_empty(ctx->symqueue));
        string_clear(ctx->param);
        string_clear(ctx->retval);

        if (saved_LHS)
            free(saved_LHS);

        return token;
    }

    exit(ESYNTAX);
}

static token_t var_assign(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);
    enum variable_type expr_type;
    char expr_type_string[2];
    char *saved_LHS;

    if (token.type == ASSIGNMENT) {
        saved_LHS = string_export(ctx->param);
        token = get_next_token(ctx);

        if (token.type == IDENTIFIER) {
            string_clear(ctx->param);
            identifier_t *fun_id = symtable_find(symstack_global_symtable(ctx->symstack),
                                                token.identifier->name);
            if (fun_id) {
                if (strncmp(saved_LHS, fun_id->fun.retval, strlen(saved_LHS))) {
                    LOG_ERROR("Variable type '%s' is not compatible with %s(), which returns '%s'",
                            saved_LHS,
                            fun_id->name,
                            fun_id->fun.retval);
                    exit(EFUNCALL);
                }
                LOG_DEBUG("ok, we can call %s()", fun_id->name);
                token.identifier = fun_id;
                token = call(token, ctx);

                // TODO no implicit conv here, right?
                gen_returned_assign(ctx->symqueue, false);
                assert(symqueue_is_empty(ctx->symqueue));
                free(saved_LHS);

                return token;
            }
        }
        // RHS is single expression
        // TODO saved_LHS doesn't work

        unget_token(token);
        LOG_DEBUG_M("...expression code");
        expr_type = expr_parser_start(ctx);

        LOG_DEBUG("var_assing after expr: LHS '%s' expr_type '%c'",
                saved_LHS, expr_type);
        if (saved_LHS[0] == 'n' && expr_type == 'i') {
            gen_conv_to_number_top();
            expr_type = 'n';
        }

        snprintf(expr_type_string, 2, "%c", expr_type);
        if (strcmp(saved_LHS, expr_type_string)) {
            LOG_ERROR("expr type '%s' doesn't match variable type '%s'",
                    expr_type_string, saved_LHS);
            exit(EASSIGN);
        }
        // generate
        gen_var_dec_assign(ctx->symqueue, true);
        free(saved_LHS);

        return token;

    } else if (token.type == IDENTIFIER) {
            LOG_DEBUG_M("id unget and return");
            gen_var_dec_assign(ctx->symqueue, false);
            unget_token(token);
            return token;

    } else if (token.type == KEYWORD) {
        if (*token.keyword == KW_RETURN) {
            LOG_DEBUG_M("return unget and return");
            gen_var_dec_assign(ctx->symqueue, false);
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_END) {
            LOG_DEBUG_M("end unget and return");
            gen_var_dec_assign(ctx->symqueue, false);
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_LOCAL) {
            LOG_DEBUG_M("local unget and return");
            gen_var_dec_assign(ctx->symqueue, false);
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_IF) {
            LOG_DEBUG_M("if unget and return");
            gen_var_dec_assign(ctx->symqueue, false);
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_ELSE) {
            LOG_DEBUG_M("else unget and return");
            gen_var_dec_assign(ctx->symqueue, false);
            unget_token(token);
            return token;

        } else if (*token.keyword == KW_WHILE) {
            LOG_DEBUG_M("while unget and return");
            gen_var_dec_assign(ctx->symqueue, false);
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

                // generate
                gen_var_dec(ctx->saved_id, ctx->symqueue);

                LOG_DEBUG_M("id ok");
                debug_token(token);
                token = get_next_token(ctx);
                if (token.type == TYPE_SPEC) {
                    LOG_DEBUG_M(": ok");
                    token = get_next_token(ctx);
                    token = type(token, ctx);
                    debug_identifier(ctx->saved_id);
                    string_appendc(ctx->param, ctx->saved_id->var.type);
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
            LOG_DEBUG("param after var_dec(): '%s'",
                    string_expose(ctx->param));
            token = var_assign(token, ctx);
            string_clear(ctx->param);
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
    identifier_t *body_id;

    if (token.type == IDENTIFIER) {
        LOG_DEBUG_M("id diving");
        identifier_t *fun_id = symtable_find(symstack_global_symtable(ctx->symstack),
                                            token.identifier->name);
        if (fun_id) {
            // write() handled by swapping saved_id
            // call, but there is no LHS
            token.identifier = fun_id;
            body_id = ctx->saved_id;
            ctx->saved_id = fun_id;

            token = call(token, ctx);

            ctx->saved_id = body_id;
            debug_identifier(ctx->saved_id);
        } else {
            token = stmt(token, ctx);
            LOG_DEBUG_M("returning from stmt");
            debug_token(token);
        }
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

    unsigned int if_cnt;
    symtable_t *then_symtable = symtable_create();
    symtable_t *else_symtable = symtable_create();
    symtable_t *popped;

    if (token.type == KEYWORD) {
        if (*token.keyword == KW_IF) {
            LOG_DEBUG_M("if ok");
            // generates code for expression evaluation
            expr_parser_start(ctx);

            if_cnt = gen_if_start();

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

            gen_if_else(if_cnt);

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

            gen_if_end(if_cnt);

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

            gen_while_start_before_expr();

            // generates code for expression at top of the stack
            expr_parser_start(ctx);

            gen_while_start_after_expr();

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

            gen_while_end();

            return token;
        }
    }

    exit(ESYNTAX);
}

static token_t param(token_t token, context_t *ctx)
{
    LOG_DEBUG_M();
    debug_token(token);

    identifier_t *saved_id;

    symtable_t *global = symstack_global_symtable(ctx->symstack);

    if (token.type == IDENTIFIER) {
        if (symtable_find(global, token.identifier->name)) {
            LOG_ERROR("%s is already a function", token.identifier->name);
            exit(EDEF);
        }

        LOG_DEBUG_M("id ok");
        debug_token(token);

        token.identifier->type = VARIABLE;
        saved_id = token.identifier;

        gen_fun_param(token.identifier);

        token = get_next_token(ctx);

        if (token.type == TYPE_SPEC) {
            LOG_DEBUG_M(": ok");
            token = get_next_token(ctx);
            
            if (token.type == KEYWORD) {
                if (*token.keyword == KW_INTEGER) {
                    LOG_DEBUG_M("integer diving");
                    token = type(token, ctx);
                    string_appendc(ctx->param, VAR_INTEGER);
                    saved_id->var.type = VAR_INTEGER;
                    LOG_DEBUG("var '%s' set to integer", saved_id->name);
                    return token;
                } else if (*token.keyword == KW_NUMBER) {
                    LOG_DEBUG_M("number diving");
                    token = type(token, ctx);
                    string_appendc(ctx->param, VAR_NUMBER);
                    saved_id->var.type = VAR_NUMBER;
                    LOG_DEBUG("var '%s' set to number", saved_id->name);
                    return token;
                } else if (*token.keyword == KW_STRING) {
                    LOG_DEBUG_M("string diving");
                    token = type(token, ctx);
                    string_appendc(ctx->param, VAR_STRING);
                    saved_id->var.type = VAR_STRING;
                    LOG_DEBUG("var '%s' set to string", saved_id->name);
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

                // generate
                gen_fun_start(function_id);

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

                LOG_DEBUG_M("saved before fun_ret()");
                debug_identifier(ctx->saved_id);
                // TODO hack
                if (ctx->saved_id->type != FUNCTION) {
                    ctx->saved_id->type = FUNCTION;
                    token = fun_ret(token, ctx);
                    ctx->saved_id->type = 0;
                } else {
                    token = fun_ret(token, ctx);
                }
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

                // TODO don't remember what I wanted here...
                ctx->saved_id = function_id;

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

                        // generate
                        gen_fun_end(function_id);

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

            // TODO commented this because fun dec generated code for retvals
            //ctx->saved_id->type = FUNCTION;

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
                    LOG_DEBUG("fun_dec() before signature: saved_id->type: %d", ctx->saved_id->type);
                    token = fun_signature(token, ctx);
                    LOG_DEBUG_M("returning from fun_dec with");
                    ctx->saved_id->type = FUNCTION;
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
        // really? it can be only declared and definition might follow, right?
        if (!symtable_find(global, token.identifier->name)) {
            LOG_ERROR("%s() is not defined/declared function, cannot be called",
                    token.identifier->name);
            exit(EDEF);
        }
        LOG_DEBUG_M("id diving");
        // this saves the id of called function
        // so we know we are calling write()
        // and it doesn't rewrite anything important, because
        // we are not in body
        ctx->saved_id = token.identifier;
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

                    // generate
                    gen_ifjcode21();

                    gen_builtins();

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


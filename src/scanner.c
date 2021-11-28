/**
 * @file scanner.c
 * Scanner (lexical analyzer)
 *
 * It loads code from stdin and parses it into tokens for syntactic analyzer
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Michal Šmahel (xsmahe01)
 */

#include "scanner.h"
#include "kwtable.h"
#include "string_factory.h"
#include "exit_codes.h"
#include "logger.h"
#include "symstack.h"
#include "context.h"
#include "token.h"

#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>

/**
 * Internal global variable for storing saved tokens
 */
token_t saved_token = {0,};
/**
 * Is saved_token empty, so another token could be saved in there?
 */
bool token_cache_empty = true;

/**
 * Finite state machine's states
 */
enum fsm_states {
    S_INIT,
    S_SYMBOL,
    S_INT, S_DEC, S_NUM, S_EXP, S_EXP_SIGN, S_NUM_EXP,
    S_DIV, S_INT_DIV,
    S_PLUS,
    S_MUL,
    S_GT, S_GE,
    S_LT, S_LE,
    S_ASSIGN, S_EQ,
    S_TYPE_SPEC,
    S_COMMA,
    S_STRLEN,
    S_LEFT_PAR,
    S_RIGHT_PAR,
    S_MINUS, S_DECIDE_COMM, S_LINE_COMM, S_ML_COMM_1, S_ML_COMM_2, S_ML_COMM_3,
    S_STR_IN, S_STR_OUT, S_ESCAPE, S_ESCAPE_D, S_ESCAPE_DD,
    S_CAT_1, S_CAT_2,
    S_NE_1, S_NE_2
};

static void append_with_normalization(string_t *string, char c)
{
    char tmp_char[5];

    if ((c >= 0 && c <= 32) || c == 35 || c == 92) {
        // Normalize character into \ddd format (ddd = ASCII position)
        sprintf(tmp_char, "\\%03d", c);

        for (int i = 0; i < 4; i++) {
            APPEND_C(string, tmp_char[i]);
        }

        return;
    }

    // Just append the char
    APPEND_C(string, c);
}

token_t get_next_token(context_t *context)
{
    int c; // Loaded char from stdin
    enum fsm_states state = S_INIT;
    keyword_t *tmp_keyword;
    identifier_t *tmp_identifier;
    symtable_t *tmp_symtable;
    token_t token;
    static int row = 1; // X coord of input
    static int column = 1; // Y coord of input

    // Prepare string factory
    string_clear(context->string);

    // Just return saved token, when there is one
    if (!token_cache_empty) {
        token_cache_empty = true;

        return saved_token;
    }

    // Construct token from data on standard input
    c = getchar();
    while (c != EOF) {
        // We're using states from enum, default isn't needed
        switch (state) { // NOLINT(hicpp-multiway-paths-covered)
            case S_INIT:
                if (isspace(c)) {
                    if (c == '\n') {
                        NEW_ROW(row, column);
                    }

                    state = S_INIT;
                }
                else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
                    APPEND_C(context->string, c);
                    state = S_SYMBOL;
                } else if (isdigit(c)) {
                    APPEND_C(context->string, c);
                    state = S_INT;
                } else if (c == '/')
                    state = S_DIV;
                else if (c == '+')
                    state = S_PLUS;
                else if (c == '*')
                    state = S_MUL;
                else if (c == '>')
                    state = S_GT;
                else if (c == '<')
                    state = S_LT;
                else if (c == '=')
                    state = S_ASSIGN;
                else if (c == ':')
                    state = S_TYPE_SPEC;
                else if (c == ',')
                    state = S_COMMA;
                else if (c == '#')
                    state = S_STRLEN;
                else if (c == '(')
                    state = S_LEFT_PAR;
                else if (c == ')')
                    state = S_RIGHT_PAR;
                else if (c == '-')
                    state = S_MINUS;
                else if (c == '"')
                    state = S_STR_IN;
                else if (c == '.')
                    state = S_CAT_1;
                else if (c == '~')
                    state = S_NE_1;
                else
                    exit(ELEX);
                break;
            case S_SYMBOL:
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || isdigit(c)) {
                    APPEND_C(context->string, c);
                    state = S_SYMBOL;
                } else {
                    // We have complete symbol, but we don't know what is it - keyword or identifier?
                    tmp_keyword = kwtable_find(context->kwtable, string_expose(context->string));
                    if (tmp_keyword) {
                        token.type = KEYWORD;
                        token.keyword = tmp_keyword;
                    } else {
                        // Try to find identifier in tables of symbols
                        symstack_most_local(context->symstack);
                        tmp_identifier = symtable_find(symstack_get(context->symstack), string_expose(context->string));
                        if (!tmp_identifier) {
                            // Create new identifier in the most-local symtable
                            symstack_most_local(context->symstack);
                            tmp_symtable = symstack_get(context->symstack);
                            tmp_identifier = symtable_add(tmp_symtable, string_expose(context->string));

                            // Setup new identifier
                            tmp_identifier->line = row;
                            tmp_identifier->character = column - strlen(string_expose(context->string));
                        }

                        token.type = IDENTIFIER;
                        token.identifier = tmp_identifier;
                    }

                    UNGET_CHAR(c);
                    return token;
                }
                break;
            case S_INT:
                APPEND_C(context->string, c);

                if (isdigit(c))
                    state = S_INT;
                else if (c == '.')
                    state = S_DEC;
                else if (c == 'e' || c == 'E')
                    state = S_EXP;
                else {
                    token.type = INTEGER;
                    token.integer = (int)strtol(string_expose(context->string), NULL, 10);

                    UNGET_CHAR(c);
                    return token;
                }
                break;
            case S_DEC:
                if (isdigit(c)) {
                    APPEND_C(context->string, c);
                    state = S_NUM;
                } else
                    exit(ELEX);
                break;
            case S_NUM:
                if (isdigit(c)) {
                    APPEND_C(context->string, c);
                    state = S_NUM;
                } else if (c == 'e' || c == 'E') {
                    APPEND_C(context->string, c);
                    state = S_EXP;
                } else {
                    token.type = NUMBER;
                    token.number = strtod(string_expose(context->string), NULL);

                    UNGET_CHAR(c);
                    return token;
                }
                break;
            case S_EXP:
                if (c == '+' || c == '-') {
                    APPEND_C(context->string, c);
                    state = S_EXP_SIGN;
                } else if (isdigit(c)) {
                    APPEND_C(context->string, c);
                    state = S_NUM_EXP;
                } else
                    exit(ELEX);
                break;
            case S_EXP_SIGN:
                if (isdigit(c)) {
                    APPEND_C(context->string, c);
                    state = S_NUM_EXP;
                } else
                    exit(ELEX);
                break;
            case S_NUM_EXP:
                if (isdigit(c)) {
                    APPEND_C(context->string, c);
                    state = S_NUM_EXP;
                } else {
                    token.type = NUMBER;
                    token.number = strtod(string_expose(context->string), NULL);

                    UNGET_CHAR(c);
                    return token;
                }
                break;
            case S_DIV:
                if (c == '/')
                    state = S_INT_DIV;
                else {
                    UNGET_CHAR(c);
                    return INIT_TOKEN(token, DIVISION);
                }
                break;
            case S_INT_DIV:
                UNGET_CHAR(c);
                return INIT_TOKEN(token, INT_DIVISION);
            case S_PLUS:
                UNGET_CHAR(c);
                return INIT_TOKEN(token, ADDITION);
            case S_MUL:
                UNGET_CHAR(c);
                return INIT_TOKEN(token, MULTIPLICATION);
            case S_GT:
                if (c == '=')
                    state = S_GE;
                else {
                    UNGET_CHAR(c);
                    return INIT_TOKEN(token, GT);
                }
                break;
            case S_GE:
                UNGET_CHAR(c);
                return INIT_TOKEN(token, GEQ);
            case S_LT:
                if (c == '=')
                    state = S_LE;
                else {
                    UNGET_CHAR(c);
                    return INIT_TOKEN(token, LT);
                }
                break;
            case S_LE:
                UNGET_CHAR(c);
                return INIT_TOKEN(token, LEQ);
            case S_ASSIGN:
                if (c == '=')
                    state = S_EQ;
                else {
                    UNGET_CHAR(c);
                    return INIT_TOKEN(token, ASSIGNMENT);
                }
                break;
            case S_EQ:
                UNGET_CHAR(c);
                return INIT_TOKEN(token, EQ);
            case S_TYPE_SPEC:
                UNGET_CHAR(c);
                return INIT_TOKEN(token, TYPE_SPEC);
            case S_COMMA:
                UNGET_CHAR(c);
                return INIT_TOKEN(token, COMMA);
            case S_STRLEN:
                UNGET_CHAR(c);
                return INIT_TOKEN(token, STRLEN);
            case S_LEFT_PAR:
                UNGET_CHAR(c);
                return INIT_TOKEN(token, LEFT_PAR);
            case S_RIGHT_PAR:
                UNGET_CHAR(c);
                return INIT_TOKEN(token, RIGHT_PAR);
            case S_MINUS:
                if (c == '-')
                    state = S_DECIDE_COMM;
                else {
                    UNGET_CHAR(c);
                    return INIT_TOKEN(token, SUBTRACTION);
                }
                break;
            case S_DECIDE_COMM:
                if (c == '[')
                    state = S_ML_COMM_1;
                else
                    state = S_LINE_COMM;
                break;
            case S_LINE_COMM:
                if (c == '\n') {
                    state = S_INIT;

                    NEW_ROW(row, column);
                }
                else
                    state = S_LINE_COMM;
                break;
            case S_ML_COMM_1:
                if (c == '[')
                    state = S_ML_COMM_2;
                else
                    state = S_LINE_COMM;
                break;
            case S_ML_COMM_2:
                if (c == ']')
                    state = S_ML_COMM_3;
                else {
                    if (c == '\n')
                        NEW_ROW(row, column);

                    state = S_ML_COMM_2;
                }
                break;
            case S_ML_COMM_3:
                if (c == ']')
                    state = S_INIT;
                else
                    state = S_ML_COMM_2;
                break;
            case S_STR_IN:
                if (c == '"')
                    state = S_STR_OUT;
                else if (c == '\\')
                    state = S_ESCAPE;
                else {
                    append_with_normalization(context->string, (char) c);
                    state = S_STR_IN;
                }
                break;
            case S_STR_OUT:
                token.type = STRING;
                token.string = string_export(context->string);

                UNGET_CHAR(c);
                return token;
            case S_ESCAPE:
                if (isdigit(c)) {
                    // Backslash is ignored in other cases, so it needs to be explicitly appended here
                    APPEND_C(context->string, '\\');
                    APPEND_C(context->string, c);
                    state = S_ESCAPE_D;
                } else if (c == '"' || c == '\\' || c == 'n' || c == 't') {
                    if (c == 'n')
                        append_with_normalization(context->string, '\n');
                    else if (c == 't')
                        append_with_normalization(context->string, '\t');
                    else
                        append_with_normalization(context->string, (char) c);

                    state = S_STR_IN;
                } else
                    exit(ELEX);
                break;
            case S_ESCAPE_D:
                if (isdigit(c)) {
                    APPEND_C(context->string, c);
                    state = S_ESCAPE_DD;
                } else
                    exit(ELEX);
                break;
            case S_ESCAPE_DD:
                if (isdigit(c)) {
                    APPEND_C(context->string, c);
                    state = S_STR_IN;
                } else
                    exit(ELEX);
                break;
            case S_CAT_1:
                if (c == '.')
                    state = S_CAT_2;
                else
                    exit(ELEX);
                break;
            case S_CAT_2:
                UNGET_CHAR(c);
                return INIT_TOKEN(token, CONCAT);
            case S_NE_1:
                if (c == '=')
                    state = S_NE_2;
                else
                    exit(ELEX);
                break;
            case S_NE_2:
                UNGET_CHAR(c);
                return INIT_TOKEN(token, NEQ);
        }

        // Load next character
        c = getchar();
        column++;
    }

    // c == EOF --> we're at the end of input stream
    token.type = END;

    return token;
}

void unget_token(token_t token)
{
    if (!token_cache_empty) {
        // Token cache is one-item only, we can't store more tokens
        LOG_ERROR_M("Tried to unget second token to scanner. This operation is forbidden.");
        return;
    }

    saved_token = token;
    token_cache_empty = false;
}

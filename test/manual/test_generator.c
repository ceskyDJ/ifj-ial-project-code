#include "../../src/generator.h"
#include "../../src/token.h"
#include "../../src/symtable.h"
#include "../../src/symqueue.h"
#include "../../src/kwtable.h"

#include <stdbool.h>

int main() {
    token_t token;
    identifier_t id;
    identifier_t tmp;
    symqueue_t *queue= symqueue_create();
    kwtable_t *kw = kwtable_create();

    printf("#-----GEN_IFJCODE21-----\n");
    gen_ifjcode21();

    printf("\n#-----GEN_FUN_START-----\n");
    id.name = "foo";
    id.fun.param = "ssin";
    gen_fun_start(&id);

    printf("\n#-----GEN_FUN_PARAM-----\n");
    id.name = "x";
    id.line = 1;
    id.character = 1;
    gen_fun_param(&id);

    printf("\n");

    id.name = "y";
    id.line = 12;
    id.character = 13;
    gen_fun_param(&id);


    printf("\n#-----GEN_CREATE_FRAME-----\n");
    gen_create_frame();

    printf("\n#-----GEN_CALL_PARAM-----\n");
    token.type = IDENTIFIER;
    token.identifier = &id;
    gen_call_param(&token);

    token.type = INTEGER;
    token.integer = 13;
    gen_call_param(&token);

    token.type = STRING;
    token.string = "call_param_string";
    gen_call_param(&token);

    printf("\n#-----GEN_VAR_DEC-----\n");
    id.name = "z";
    id.line = 27;
    id.character = 22;
    gen_var_dec(&id, queue);
    printf("[symqueue %s\n", symqueue_is_empty(queue) ? "is empty]" : "is NOT empty]");

    printf("\n#-----GEN_VAR_DEC_ASSIGN-----\n");
    gen_var_dec_assign(queue, true);
    printf("[symqueue %s\n", symqueue_is_empty(queue) ? "is empty]" : "is NOT empty]");

    printf("\n#-----GEN_VAR_SET_ACTIVE-----\n");
    id.name = "trie";
    id.line = 43;
    id.character = 1;
    gen_var_set_active(&id, queue);
    printf("[symqueue %s\n", symqueue_is_empty(queue) ? "is empty]" : "is NOT empty]");

    tmp.name = "%retval_1";
    tmp.line = 0;
    tmp.character = 0;
    gen_var_set_active(&tmp, queue);
    printf("[symqueue %s\n", symqueue_is_empty(queue) ? "is empty]" : "is NOT empty]");

    printf("\n#-----GEN_VAR_ACTIVE_ASSIGN-----\n");
    gen_var_active_assign(queue, true);
    printf("[symqueue %s\n", symqueue_is_empty(queue) ? "is empty]" : "is NOT empty]");

    printf("\n");

    gen_var_active_assign(queue, true);
    printf("[symqueue %s\n", symqueue_is_empty(queue) ? "is empty]" : "is NOT empty]");

    gen_var_active_assign(queue, false);
    printf("[symqueue %s\n", symqueue_is_empty(queue) ? "is empty]" : "is NOT empty]");

    printf("\n#-----GEN_RETURNED_ASSIGN-----\n");
    id.name = "trie";
    id.line = 43;
    id.character = 1;
    gen_var_set_active(&id, queue);
    tmp.name = "foo";
    tmp.line = 2;
    tmp.character = 3;
    gen_var_set_active(&tmp, queue);

    gen_returned_assign(queue);
    gen_returned_assign(queue);


    printf("\n#-----GEN_VAR_RETVAL-----\n");
    gen_var_retval();

    printf("\n");

    gen_var_retval();

    printf("\n#-----GEN_FUN_END-----\n");
    id.name = "foo";
    gen_fun_end(&id);

    printf("\n#-----GEN_PUSH_TERM-----\n");
    token.type = INTEGER;
    token.integer = 42;
    gen_push_term(&token);

    printf("\n");

    token.type = NUMBER;
    token.number = 5.55;
    gen_push_term(&token);

    printf("\n");
    
    token.type = STRING;
    token.string = "hello\\032there\\032this\\032is\\032string";
    gen_push_term(&token);

    printf("\n");

    id.name = "mr_token";
    id.line = 99;
    id.character = 11;
    token.type = IDENTIFIER;
    token.identifier = &id;
    gen_push_term(&token);

    printf("\n");

    token.type = KEYWORD;
    token.keyword = kwtable_find(kw, "nil");
    gen_push_term(&token);

    printf("\n#-----GEN_OPERATION-----\n");
    printf("skipped the simple ones...\n");
    gen_operation(LEQ);
    printf("\n");
    gen_operation(GEQ);
    printf("\n");
    gen_operation(STRLEN);
    printf("\n");
    gen_operation(CONCAT);
    printf("\n");

    printf("\n#-----GEN_IF_START-----\n");
    unsigned int if_cnt;
    if_cnt = gen_if_start();

    printf("\n#-----GEN_IF_ELSE-----\n");
    gen_if_else(if_cnt);

    printf("\n#-----GEN_IF_END-----\n");
    gen_if_end(if_cnt);

    printf("\n#-----GEN_WHILE_START_BEFORE_EXPR-----\n");
    gen_while_start_before_expr();

    printf("\n#-----GEN_WHILE_START_AFTER_EXPR-----\n");
    gen_while_start_after_expr();

    printf("\n#-----GEN_WHILE_END-----\n");
    gen_while_end();

    printf("\n#-----GEN_NIL_CHECK_TOP-----\n");
    gen_nil_check_top();

    printf("\n#-----GEN_NIL_CHECK_SUBTOP-----\n");
    gen_nil_check_subtop();

    printf("\n#-----GEN_ZERO_DIV_CHECK-----\n");
    gen_zero_div_check();

    printf("\n#-----GEN_CONV_TO_NUMBER_TOP-----\n");
    gen_conv_to_number_top(); 

    printf("\n#-----GEN_CONV_TO_NUMBER_SUBTOP-----\n");
    gen_conv_to_number_subtop();

    return 0;
}

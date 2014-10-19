#include "scenario.h"

bool basic_xform_scenario(char** msg) {
    setup(2);

    ot_op* opa = ot_new_op();
    ot_insert(opa, "abc");
    ot_client_apply(clients[0], &opa);
    flush_client(0);
    ASSERT_OP_SNAPSHOT(clients[0]->doc->composed, "abc", msg);

    ot_op* opb = ot_new_op();
    ot_insert(opb, "def");
    ot_client_apply(clients[1], &opb);
    flush_client(1);
    ASSERT_OP_SNAPSHOT(clients[1]->doc->composed, "def", msg);

    flush_server();

    ASSERT_CONVERGENCE("abcdef", msg);

    teardown();
    return true;
}

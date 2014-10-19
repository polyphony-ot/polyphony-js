#include "scenario.h"

bool xform_anticipated_scenario(char** msg) {
    setup(2);

    ot_op* opa = ot_new_op();
    ot_insert(opa, "a");
    ot_client_apply(clients[0], &opa);
    ASSERT_OP_SNAPSHOT(clients[0]->doc->composed, "a", msg);

    ot_op* opb = ot_new_op();
    ot_insert(opb, "b");
    ot_client_apply(clients[1], &opb);
    ASSERT_OP_SNAPSHOT(clients[1]->doc->composed, "b", msg);

    flush_clients();

    ot_op* opc = ot_new_op();
    ot_skip(opc, 1);
    ot_insert(opc, "b");
    ot_client_apply(clients[1], &opc);
    ASSERT_OP_SNAPSHOT(clients[1]->doc->composed, "bb", msg);

    ot_op* opd = ot_new_op();
    ot_skip(opd, 1);
    ot_insert(opd, "a");
    ot_client_apply(clients[0], &opd);
    ASSERT_OP_SNAPSHOT(clients[0]->doc->composed, "aa", msg);

    flush_server();
    flush_client(0);
    flush_server();
    flush_client(1);
    flush_server();

    ASSERT_CONVERGENCE("abab", msg);

    teardown();

    return true;
}

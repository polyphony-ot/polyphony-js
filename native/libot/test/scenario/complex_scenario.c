#include "scenario.h"

// In this scenario, a complex case of transformation, buffering, and bridging
// is tested. It should fully exercise all the components needed for real-time
// text editing.
bool complex_scenario(char** msg) {
    setup(2);

    ot_op* opc = ot_new_op();
    ot_insert(opc, "ABC");
    ot_client_apply(clients[1], &opc);

    flush_client(1);
    ASSERT_OP_SNAPSHOT(server->doc->composed, "ABC", msg);
    ASSERT_OP_SNAPSHOT(clients[1]->doc->composed, "ABC", msg);

    ot_op* opa = ot_new_op();
    ot_insert(opa, "abc");
    ot_client_apply(clients[0], &opa);
    ASSERT_OP_SNAPSHOT(clients[0]->doc->composed, "abc", msg);

    ot_op* opb = ot_new_op();
    ot_skip(opb, 3);
    ot_insert(opb, "def");
    ot_client_apply(clients[0], &opb);
    ASSERT_OP_SNAPSHOT(clients[0]->doc->composed, "abcdef", msg);

    flush_server();
    ASSERT_OP_SNAPSHOT(server->doc->composed, "ABC", msg);
    ASSERT_OP_SNAPSHOT(clients[0]->doc->composed, "ABCabcdef", msg);
    ASSERT_OP_SNAPSHOT(clients[1]->doc->composed, "ABC", msg);

    ot_op* ope = ot_new_op();
    ot_skip(ope, 9);
    ot_insert(ope, "ghi");
    ot_client_apply(clients[0], &ope);
    ASSERT_OP_SNAPSHOT(clients[0]->doc->composed, "ABCabcdefghi", msg);

    ot_op* opd = ot_new_op();
    ot_skip(opd, 3);
    ot_insert(opd, "DEF");
    ot_client_apply(clients[1], &opd);

    flush_client(1);
    ASSERT_OP_SNAPSHOT(server->doc->composed, "ABCDEF", msg);
    ASSERT_OP_SNAPSHOT(clients[1]->doc->composed, "ABCDEF", msg);

    flush_server();
    ASSERT_OP_SNAPSHOT(clients[0]->doc->composed, "ABCDEFabcdefghi", msg);
    ASSERT_OP_SNAPSHOT(clients[1]->doc->composed, "ABCDEF", msg);

    flush_client(0);
    flush_server();
    ASSERT_OP_SNAPSHOT(server->doc->composed, "ABCDEFabc", msg);
    ASSERT_OP_SNAPSHOT(clients[0]->doc->composed, "ABCDEFabcdefghi", msg);
    ASSERT_OP_SNAPSHOT(clients[1]->doc->composed, "ABCDEFabc", msg);

    flush_client(0);
    flush_server();
    ASSERT_CONVERGENCE("ABCDEFabcdefghi", msg);

    teardown();

    return true;
}

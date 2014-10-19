#include "scenario.h"

// In this scenario, two clients send an initial op at the same time, requiring
// that the server transform them. Before the server replies with the
// transformed ops, one client buffers additional changes. The buffered changes
// are then sent after the server's response is received.
bool xform_buffer_scenario(char** msg) {
    setup(2);

    ot_op* opc = ot_new_op();
    ot_insert(opc, "ABC");
    ot_client_apply(clients[1], &opc);
    flush_clients();
    ASSERT_OP_SNAPSHOT(clients[1]->doc->composed, "ABC", msg);

    ot_op* opa = ot_new_op();
    ot_insert(opa, "abc");
    ot_client_apply(clients[0], &opa);
    flush_clients();
    ASSERT_OP_SNAPSHOT(clients[0]->doc->composed, "abc", msg);

    ot_op* opb = ot_new_op();
    ot_skip(opb, 3);
    ot_insert(opb, "def");
    ot_client_apply(clients[0], &opb);
    ASSERT_OP_SNAPSHOT(clients[0]->doc->composed, "abcdef", msg);

    flush_server();

    ASSERT_OP_SNAPSHOT(server->doc->composed, "ABCabc", msg);
    ASSERT_OP_SNAPSHOT(clients[0]->doc->composed, "ABCabcdef", msg);
    ASSERT_OP_SNAPSHOT(clients[1]->doc->composed, "ABCabc", msg);

    flush_clients();
    flush_server();

    ASSERT_CONVERGENCE("ABCabcdef", msg);

    teardown();

    return true;
}

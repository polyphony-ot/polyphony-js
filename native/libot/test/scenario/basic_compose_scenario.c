#include "scenario.h"

// In this scenario, two clients make non-concurrent, composable changes so that
// transformation is never needed.
bool basic_compose_scenario(char** msg) {
    setup(2);

    ot_op* opa = ot_new_op();
    ot_insert(opa, "a");
    ot_client_apply(clients[0], &opa);

    flush_clients();
    flush_server();
    ASSERT_CONVERGENCE("a", msg);

    ot_op* opb = ot_new_op();
    ot_skip(opb, 1);
    ot_insert(opb, "b");
    ot_client_apply(clients[1], &opb);

    flush_clients();
    flush_server();
    ASSERT_CONVERGENCE("ab", msg);

    ot_op* opc = ot_new_op();
    ot_skip(opc, 1);
    ot_delete(opc, 1);
    ot_client_apply(clients[0], &opc);

    flush_clients();
    flush_server();
    ASSERT_CONVERGENCE("a", msg);

    teardown();

    return true;
}

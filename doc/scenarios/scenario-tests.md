Scenario Tests
==============

Scenario tests are high-level integration tests that can be used to test client-server interactions. They use the scenario testing framework which is defined in `test/scenario/scenario.h`. This framework makes it easy to test virtually any OT scenario by giving you the ability to control communication between clients and a server.

The entire scenario testing framework and reference documentation can be found in [scenario.h](../../test/scenario/scenario.h).

How Scenario Tests Work
-----------------------

The scenario testing framework provides test scenarios with a single server instance and an arbitrary number of clients. The test can then use the server and clients just as a real application would - with one difference. Scenarios are provided with queues that can buffer client or server operations. These queues allow you to effectively simulate network latency or concurrent operations in a reproducible way.

![Diagram showing how the scenario queues work](../images/scenario-diagram.png)

As the diagram above shows, operations are not immediately received when they are sent by a client or the server. Instead, they are put into a queue that must be flushed at a later time. Operations will only reach their receipient after the queue they reside in is flushed.

Creating a Scenario
-------------------

Creating new scenario tests is best explained with an example. This section walks step-by-step through creating `basic_xform_scenario.c`. This scenario tests that all of the clients converge on the same document after applying two different operations concurrently. That is, client 1 applies "abc", client 2 applies "def", the server transforms them and we end up with "abcdef".

### The Basic Test Function

Scenarios look like any other automated test. They are defined by a single function with a specific signature. Scenarios **must** call the `setup()` and `teardown()` functions that are defined by the framework. The `setup()` function creates everything that your test will need to run, including clients, a server, and callbacks that forward operations between them. The `teardown()` function does the opposite. It cleans up everything so that the scenario doesn't leave behind any memory leaks.

```c
// Include the framework header.
#include "scenario.h"

// Every test must return a bool to indicate if the test passed or failed. It
// should also take a single parameter, msg, that allows you to output a
// helpful error message if the test failed. The name of the function can be
// whatever you want.
bool basic_xform_scenario(char** msg) {
    // Call the setup function telling the framework that our scenario will
    // need 2 clients.
    setup(2);

    // Code for running the scenario.

    // Remember to teardown the test to avoid any memory leaks.
    teardown();

    // Return true if the scenario passed.
    return true;
}
```

### Performing the Test

Once your test function is in place, the next step is to run code that actually carries out the scenario you want to test. Let's look at the first chunk of code from `basic_xform_scenario.c`.

First we create a new operation, called opa, that inserts the text "abc".

```c
ot_op* opa = ot_new_op();
ot_insert(opa, "abc");
```

Next, we apply opa to one of the clients. The framework creates an array of clients that contains the number of clients we requested when we called setup (which in this case is 2). Here we are applying opa to the first client (keep in mind that the first client is index 0).

```c
ot_client_apply(clients[0], &opa);
```

When we applied opa it was not automatically send to the server, which it would've been if this were a real application. Instead, the operation was placed into a queue. We must call `flush_clients()` or `flush_clients(0)` in order for the server to actually receive opa.

```c
flush_clients(0);
```

Now that we've applied and sent opa, our first assertion verifies that the snapshot of the client's document is actually equal to "abc". `ASSERT_OP_SNAPSHOT` is a macro provided by the framework which will automatically return false and fail the test if an operation's snapshot isn't equal to an expected string.

```c
ASSERT_OP_SNAPSHOT(clients[0]->doc->composed, "abc", msg);
```

Now we do the same thing again, but instead apply the text "def" to the other client.

```c
ot_op* opb = ot_new_op();
ot_insert(opb, "def");
ot_client_apply(clients[1], &opb);
flush_clients(1);
ASSERT_OP_SNAPSHOT(clients[1]->doc->composed, "def", msg);
```

Next, call `flush_server()` so that all of the clients receive operations relayed by the server.

```c
flush_server();
```

Finally, we assert that all of the clients and the server have converged on the same document. `ASSERT_CONVERGENCE` is a macro provided by the framework that checks for convergence on an expected string.

```c
ASSERT_CONVERGENCE("abcdef", msg);
```

The last step is to add our test to `scenario/main.c` so that it is actually run.

```c
int main() {
    // ...
    
    RUN_SCENARIO(basic_xform_scenario);

    // ...
}
```

### Testing Concurrency

An important detail to realize is that we waited to call `flush_server()` until we applied opa and opb to their respective clients. Remember that operations are not received until they're flushed from their queue. After the server received opa from client 1, it attempted to relay opa it to all of the clients. However, it was intercepted by the testing framework and placed into the server's queue. Had we allowed opa to be immediately relayed, client 2 would've received it and automatically applied it to its own document.

To make this clearer, let's start by looking at what would've happened if we didn't have a queue:

1. We call `ot_client_apply(clients[0], &opa)`.
2. opa is applied to the client 1's document.
3. Client 1 sends opa to the server.
4. The server applies opa to its own document.
5. The server broadcasts a confirmation of opa to all of the clients.
6. Client 2 receives opa.
7. Client 2 applies opa to its own document.

All we did was apply opa to client 1, but this kicked off a series of calls that actually ended up applying opa to the server and _all_ of the clients! By the time we try to apply opb to client 2, it's too late - client 2 has already been updated with opa and in fact, applying opb would fail since opb isn't composable with opa. This shows how without queuing, it is impossible to test concurrent operations.

By queuing the responses from the server, we can give ourselves an opportunity to modify other clients before they're updated. This is what's happening behind the scenes with queuing in our test:

1. We call `ot_client_apply(clients[0], &opa)` and then flush client 1.
2. The server receives and relays opa to all of the clients.
3. opa is intercepted and placed into the server's queue.
4. We call `ot_client_apply(clients[1], &opb)` and then flush client 2.
5. The server must transform opa and opb to get opb' which it broadcasts to all of the clients.
6. opb' is intercepted placed into the server's queue.
7. We call `flush_server()`.
8. Client 2 receives and applies opa.
9. Client 1 receives and applies opb'.

By delaying the responses from the server, we were able to modify two clients concurrently, allowing us to test that the server could transform opb and opa successfully.

Running Scenario Tests
----------------------

Scenario tests are automatically run along with the rest of libot's tests. For example:

```bash
$ make clean test
...
bin/debug/scenario
Running basic_xform_scenario... passed.
Running scenario2... passed.
Running scenario3... passed.

3 tests passed.
0 tests failed.
3 tests total.
```

When to Create Scenario Tests
-----------------------------

Keep in mind that scenario tests are high-level integration tests. They're extremely useful for making sure that clients and servers communicate properly and that operations get transformed and composed correctly in real-world scenarios.

This also means that scenario tests are more difficult to debug. If a scenario fails, it's usually necessary to step through the all of the client and server code to figure out when and where something went wrong. Scenarios are also inherently more complex, making them less maintainable than unit tests.

Keep these things in mind when considering writing a scenario test. For example, if you want to test that two operations can be transformed correctly, writing a scenario would probably be overkill. Instead, a unit test would be much more appropriate. However, if you want to test that three clients can insert, delete, and format the same text concurrently, then a scenario is much more useful.

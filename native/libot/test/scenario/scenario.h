/*
    This header contains the scenario testing framework. For a guide on how to
    create and run scenario tests, see doc/scenario/scenario-tests.md.
*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../../server.h"
#include "../../client.h"
#include "../common.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"

// CONSTANTS
//
// These values can be increased if a scenario test needs more clients or queue
// space. Keep in mind that memory usage will grow fast since each scenario test
// has its own queues.

// The maximum number of clients that can be used in a scenario.
#define MAX_CLIENTS 16

// The maximum number of operations that can be queued by a client.
#define MAX_CLIENT_QUEUE 16

// The maximum number of operations that can be queued by the server.
#define MAX_SERVER_QUEUE 16

// PRIVATE FRAMEWORK DECLARATIONS
//
// These are declarations for private framework variables and functions that
// aren't intended to be referenced directly. They are defined at the bottom of
// this file.

static size_t client_queue_lens[MAX_CLIENT_QUEUE];
static char* client_queues[MAX_CLIENTS][MAX_CLIENT_QUEUE];
static size_t clients_len;
static size_t server_queue_len;
static char* server_queue[MAX_SERVER_QUEUE];
static int server_send(const char* op);
static int server_event(ot_event_type t, ot_op* op);
static int client_send(const char* op);
static int client_event(ot_event_type t, ot_op* op);

// ASSERTIONS
//
// Assertion macros for validating common conditions in scenario tests. If an
// assertion fails, it will return false to fail the test and output a helpful
// error message.

// Asserts that an operation's snapshot is equal to an expected string. If it
// isn't, the test will fail and msg will be set to an error message.
#define ASSERT_OP_SNAPSHOT(op, expected, msg)                                  \
    if (!_assert_op_snapshot(op, expected, "Line #" STRLINE                    \
                                           ": Unexpected operation snapshot.", \
                             msg)) {                                           \
        return false;                                                          \
    }

// Asserts that all of the clients and the server have converged on the same
// document. If some of them have not converged, the test will fail and msg will
// be set to an error message.
#define ASSERT_CONVERGENCE(expected, msg)                                      \
    if (!_assert_convergence(expected, "Line #" STRLINE, msg)) {               \
        return false;                                                          \
    }

// SERVER AND CLIENTS
//
// These variables are set by the framework when setup() is called. They are
// used in tests to carry out a scenario. The clients will have IDs
// corresponding to their location in the array. For example,
// clients[0]->client_id == 0, clients[1]->client_id == 1, and so on.

// The single server used in a scenario.
static ot_server* server;

// The array of clients used in a scenario. It will contain the number of
// clients specified when setup() is called.
static ot_client* clients[MAX_CLIENTS];

// TEST FUNCTIONS
//
// The following functions are used in tests to flush queues, as well as setup
// and teardown the environment.

// Flushes all operations from a specific client's queue.
static void flush_client(size_t id) {
    assert(client_queue_lens[id] > 0);

    for (size_t i = 0; i < client_queue_lens[id]; ++i) {
        char* data = client_queues[id][i];
        ot_server_receive(server, data);
        free(data);
    }

    client_queue_lens[id] = 0;
}

// Flushes all operations from every client's queue.
static void flush_clients() {
    bool flushed = false;

    for (size_t i = 0; i < clients_len; ++i) {
        if (client_queue_lens[i] > 0) {
            flushed = true;
            flush_client(i);
        }
    }

    assert(flushed);
}

// Flushes all operations from the server's queue.
static void flush_server() {
    assert(server_queue_len > 0);

    for (size_t i = 0; i < server_queue_len; ++i) {
        for (size_t j = 0; j < clients_len; ++j) {
            ot_client_receive(clients[j], server_queue[i]);
        }
        free(server_queue[i]);
    }

    server_queue_len = 0;
}

// This function should be called at the beginning of every scenario. It
// initializes all of the clients and the server. num_clients should be the
// number of clients used in the scenario.
static void setup(size_t num_clients) {
    clients_len = num_clients;
    server = ot_new_server(server_send, server_event);

    for (size_t i = 0; i < num_clients; ++i) {
        clients[i] = ot_new_client(client_send, client_event);
        clients[i]->client_id = i;
    }
}

// This function should be called at the end of every scenario. It cleans up all
// the clients and the server.
static void teardown() {
    for (size_t i = 0; i < clients_len; ++i) {
        ot_free_client(clients[i]);
    }

    ot_free_server(server);
}

// PRIVATE FRAMEWORK CODE
//
// The remaining code is used internally by the framework. You shouldn't need to
// reference it directly.

// The actual number of clients being used in a scenario. This is determined by
// the number given to setup() when it is called.
static size_t clients_len = 0;

// The number of operations currently in server_queue.
static size_t server_queue_len = 0;

// The array of queued server operations.
static char* server_queue[MAX_SERVER_QUEUE];

// An array containing the lengths of each client queue.
static size_t client_queue_lens[MAX_CLIENTS] = { 0 };

// The 2D array of client queues.
static char* client_queues[MAX_CLIENTS][MAX_CLIENT_QUEUE];

// Server send callback that stores op in the staging area until it is flushed.
static int server_send(const char* op) {
    assert(server_queue_len != MAX_SERVER_QUEUE);

    size_t len = strlen(op) + 1;
    char* copy = malloc(len);
    memcpy(copy, op, len);
    server_queue[server_queue_len] = copy;
    server_queue_len++;
    return 0;
}

// Client send callback that stores op in the staging area until it is flushed.
static int client_send(const char* op) {
    ot_op* dec = ot_new_op();
    ot_decode(dec, op);
    size_t id = dec->client_id;
    ot_free_op(dec);

    assert(client_queue_lens[id] != MAX_CLIENT_QUEUE);

    size_t len = strlen(op) + 1;
    char* copy = malloc(len);
    memcpy(copy, op, len);
    client_queues[id][client_queue_lens[id]] = copy;
    client_queue_lens[id]++;
    return 0;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

// Server event callback function that simply asserts that an event type was
// provided.
static int server_event(ot_event_type t, ot_op* op) {
    assert(t);
    return 0;
}

// Client event callback function used by all of the clients that simply asserts
// that an event type was provided.
static int client_event(ot_event_type t, ot_op* op) {
    assert(t);
    return 0;
}

#pragma clang diagnostic pop

// Asserts that two operations are equal. If they aren't, msg will be set to an
// error message.
static bool assert_ops_equal(ot_op* op1, ot_op* op2, char** msg) {
    bool equal = ot_equal(op1, op2);
    if (!equal) {
        char* op1_enc = ot_encode(op1);
        char* op2_enc = ot_encode(op2);
        write_msg(msg, "Operations aren't equal.\n"
                       "\tOperation1: %s\n"
                       "\tOperation2: %s",
                  op1_enc, op2_enc);
        free(op1_enc);
        free(op2_enc);
    }

    return equal;
}

// Asserts that an operation's snapshot is equal to an expected string. If it
// isn't, msg will be set to an error message. A prefix may be provided which
// will be prepended to the message. This function is typically not used
// directly, see the ASSERT_OP_SNAPSHOT macro instead.
static bool _assert_op_snapshot(ot_op* op, char* const expected, char* prefix,
                                char** msg) {

    char* actual = ot_snapshot(op);
    int cmp = strcmp(expected, actual);
    if (cmp != 0) {
        char* op_enc = ot_encode(op);
        write_msg(msg, "%s\n"
                       "\tOperation: %s\n"
                       "\tActual Snapshot: %s\n"
                       "\tExpected Snapshot: %s",
                  prefix, op_enc, actual, expected);
        free(actual);
        free(op_enc);
        return false;
    }

    free(actual);
    return true;
}

// Asserts that all of the clients and the server have converged on the same
// document. If some of them have not converged, msg will be set to an error
// message. loc is the location in the source file where this assert was called.
// This function is typically not used directly, see the ASSERT_CONVERGENCE
// macro instead.
static bool _assert_convergence(char* const expected, char* loc, char** msg) {
    for (size_t i = 0; i < clients_len; ++i) {
        char* prefix = NULL;
        write_msg(&prefix, "%s: Client %zu didn't converge.", loc, i);
        ot_op* client_op = clients[i]->doc->composed;
        bool passed = _assert_op_snapshot(client_op, expected, prefix, msg);
        free(prefix);
        if (!passed) {
            return false;
        }
    }

    char* prefix = NULL;
    write_msg(&prefix, "%s: The server didn't converge.", loc);
    ot_op* server_op = server->doc->composed;
    bool passed = _assert_op_snapshot(server_op, expected, prefix, msg);
    free(prefix);
    if (!passed) {
        return false;
    }

    return true;
}

#pragma clang diagnostic pop

libot
=====

A cross-platform library for real-time text editing.

libot is the core of the [polyphony project](https://github.com/polyphony-ot). It uses operational transformation (OT) to allow for real-time text editing between users. Since it's written in C, it can be used on virtually any platform, meaning that the complexities of OT only need to be handled in a single codebase. The other polyphony libraries simply wrap libot in order to provide a more native experience for their own platform.

Getting Started
---------------

First, [check to see](https://github.com/polyphony-ot) if there's a polyphony library available for your platform. Using a library for your platform will be easier and will provide a more idiomatic API.

If there isn't a library for your specific platform (or if you want to make one), take a look at the [Getting Started](doc/getting-started.md) guide for an explanation of how to use libot's API.

### Example

Here's a quick snippet that demonstrates what using libot looks like. The following code shows how a client would insert the text "abc" into a document:

```c
// This function will be called by libot whenever a message needs to be sent to
// the server.
static int send(const char* msg) {
    // Send msg to the server
}

// This function will be called by libot whenever the UI or editor need to be
// updated with new text.
static int event(ot_event_type t, ot_op* op) {
    // Update the UI/editor with the changes in op.
}

// Create a new client and provide it our send and event functions.
ot_client* client = ot_new_client(send, event);

// Create and apply a new op that inserts the text "abc" into the document.
ot_op* op = ot_new_op();
ot_insert("abc");
ot_client_apply(client, op);
```

That's it! The libot client handles sending our change to the server, notifying our editor that there's new text, and resolving any conflicts with OT.

Building
--------

To build libot, simply run `make`. This will create debug and release archives of libot which can be found in `bin/debug/libot.a` and `bin/release/libot.a`.

libot builds using clang by default, but it should also be compatible with gcc.

### Testing

If you're making changes to libot or running it on a new platform, you'll also want to run tests with `make clean test`. This will run libot's entire test suite.

Code coverage can also be enabled by setting the `COVERAGE` variable with `make -e COVERAGE=1 clean test`. Note that code coverage requires that [lcov](http://ltp.sourceforge.net/coverage/lcov.php) be installed and in your PATH. The generated report can be found in `bin/coverage/index.html`.

You can also test for memory leaks by using the `TESTRUNNER` variable. This variable allows you to specify an arbitrary wrapper command for running tests. For example, to run the tests with valgrind you'd do `make -e TESTRUNNER=valgrind clean test`.

Documentation
-------------

The [doc](doc) directory contains various guides and explanations for the different parts of libot. The [Getting Started](doc/getting-started.md) guide in particular has a good general introduction to working with libot. You can also find reference documentation for specific types and functions in the header files.

Contributing
------------

Polyphony is looking for contributors! If you're interested in learning how a real-time text editor works, contributing new features, or porting libot to a new platform, please see the [contributing guide](CONTRIBUTING.md).

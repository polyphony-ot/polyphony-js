Getting Started
===============

This guide walks through the basics of how to use libot.

1. [Overview](#overview)
2. [Quickstart](#quickstart)
3. [Operational Transformation](#operational-transformation)
4. [Reference Documentation](#reference-documentation)

Overview
--------

### What is libot?

The purpose of libot is to provide applications with the ability to perform
real-time text editing. It is _not_ a complete real-time text editor, but it
can be used to easily create one.

#### How is it different from other real-time editors/libraries?

Real-time collaboration is most useful when it can be done from any application
or any platform. Traditionally, real-time text editors have focused on
JavaScript and running in the browser, making them difficult or impossible to
use on mobile devices. Some services, such as Google's Realtime API, require
that user data be stored on someone else's servers.

libot aims to solve these problems by being cross-platform and by being free for
anyone to use or modify. It's written entirely in standards-compliant C, it's
open source under Apache2, and it's extensively documented and tested.

### What is Polyphony?

Polyphony is the name of a group of projects, of which libot is the core. Using
a C library from another language usually isn't very easy, so there are other
Polyphony wrapper libraries that target a specific platform. These projects
provide an API that's idiomatic to their language, but still rely on libot under
the hood to handle all of the complex OT logic.

Before using libot directly, you should check to see if there's a Polyphony
library for the language you're using.

Quickstart
----------

### Creating a Client

A client requires two callback functions - a send function and an event
function.

The send function will be called by the client when it needs a message sent to
the server.

```c
int my_send_func(const char* msg) {
    // This function should send "msg" to the server.
}
```

The event function will be called by the client when something happens that your
editor needs to know about.

```c
int my_event_func(ot_event_type t, ot_op* op) {
    // This function should handle any events sent from the client.
}
```

With these two functions, we have all that we need to create a new `ot_client`.

```c
ot_client* client = ot_new_client(my_send_func, my_event_func);
```

The last step is assigning the client a unique ID. There are many ways to do
this, the easiest of which is by having your server increment a number for each
new client.

```c
// Generate a unique ID somehow and assign it to the client.
uint32_t unique_id = ...;
client->client_id = unique_id;
```

#### Connecting to a Server

Since libot doesn't handle networking, it's completely up to your application
(or a wrapper library) to decide how to establish a server connection. A common
choice is WebSockets.

Regardless of how your application decides to connect to the server, it will
need to invoke the `ot_client_receive` function whenever a message is received
from the server.

```c
// Pretend that this function gets called whenever a message arrives from the
// server.
void message_received(char* msg) {
    ot_client_receive(client, msg);
}
```

#### Hooking Up to an Editor

In order for libot to work correctly, the editor and libot client must stay in
sync at all times. This generally means that the interaction between the two
should be synchronous. For some UI frameworks it means that interaction with the
client should be done on the UI thread.

##### Handling User Input

When a user makes a change in the editor, that change must be passed to libot so
that it can handle updating the document. This is referred to as applying an
operation.

For example, most editors have some sort of "text changed" event. Inside that
event, we can create an operation that reflects the user's change and apply it
to the client.

```c
// Pretend that this function gets called whenever our editor's contents
// change.
void text_changed() {
    // Create a new operation that will contain the changes the user just made.
    ot_op* op = ot_new_op();

    // Use ot_insert, ot_delete and ot_skip to build the operation. How this
    // is done depends on how the editor's "text changed" event works.
    ...

    // Apply the operation to the client.
    ot_client_apply(client, op);
}
```

##### Handling Changes Sent by the Server

When the client processes a change sent by the server, it will fire two events:
`OT_OP_INCOMING` and `OT_OP_APPLIED`.

When `OT_OP_INCOMING` is received, the editor must stop what it's doing and
prepare for a received operation to be applied. This means it must:

* disallow any additional changes until the operation is applied.
* flush any pending user changes and apply them with `ot_client_apply` (see the
  previous section).

Shortly after, an `OT_OP_APPLIED` will be received. At this point the editor
must update itself with the changes contained in the `op` parameter of the event
function. After it updates, it can resume allowing user input.

#### Running a Client

Those are all the steps you need to create a fully functioning, real-time
client. If things don't work or you're encountering errors, double check that
your editor is correctly syncing changes with the client. Bugs most commonly
occur when the editor doesn't send changes to libot correctly, or it doesn't
update itself correctly after receiving changes from libot.

### Creating a Server

Similar to a client, a server also requires two callback functions - a send
function and an event function.

The send function will be called by the server when it needs a message sent to
all connected clients.

```c
int my_send_func(const char* msg) {
    // This function should send "msg" to ALL connected clients.
}
```

The event function will be called when things happen that you may want to know
about.

```c
int my_event_func(ot_event_type t, ot_op* op) {
    // This function should optionally handle any events created by the server.
}
```

To create a new `ot_server` we simply provide those two functions to
`ot_new_server`.

```c
ot_server* server = ot_new_server(my_send_func, my_event_func);
```

#### Connecting to Clients

Again, libot doesn't handle networking, so it's up to your application to decide
how to establish connections with clients.

When a message is received from a client, is should be passed to the server with
the `ot_server_receive` function.

```c
// Pretend that this function gets called whenever a message arrives from a
// client.
void message_received(char* msg) {
    ot_server_receive(server, msg);
}
```

#### Running a Server

That is generally all that's needed to create a fully functional server. Note
that a server can only have a single document open at a time. If your
application needs to support editing multiple documents simultaneously, simply
create a new server for each document.

Operational Transformation
--------------------------

The heart of what makes libot work is a technology called operational
transformation (or OT for short). Google Docs, Etherpad, Share.js, and others
all use operational transformation for real-time collaboration. It's also worth
noting that OT isn't limited to text. It can also be used to do real-time
drawing, for example.

To see how libot's OT implementation handles merging concurrent changes, see the
[documentation on OT](ot.md).

Reference Documentation
-----------------------

Reference documentation for all of libot's types and functions can be found in
the header files.

* [array.h](../array.h) - contains types and functions that implement a
  dynamically resizing array.
* [client.h](../client.h) - contains code related to the client-side portion of
  OT.
* [compose.h](../compose.h) - contains functions for performing operation
  composition.
* [decode.h](../decode.h) - contains functions for decoding operations and
  documents from JSON.
* [doc.h](../doc.h) - contains types and functions for manipulating OT
  documents.
* [encode.h](../encode.h) - contains functions for encoding operations and
  documents to JSON.
* [hex.h](../hex.h) - contains functions for converting byte arrays to hex-
  encoded strings and vice versa.
* [ot.h](../ot.h) - contains the basic types and functions for OT, including
  operations, components, iterators, etc.
* [server.h](../server.h) - contains code related to the server-side portion of
  OT.
* [sha1.h](../sha1.h) - contains code for generating SHA1 hashes.
* [xform.h](../xform.h) - contains functions for performing operational
  transforms.

#ifndef LIBOT_SERVER_H
#define LIBOT_SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "doc.h"
#include "xform.h"
#include "compose.h"
#include "encode.h"
#include "decode.h"

typedef struct {
    send_func send;
    ot_event_func event;
    ot_doc* doc;
} ot_server;

ot_server* ot_new_server(send_func send, ot_event_func event);

void ot_free_server(ot_server* server);

void ot_server_open(ot_server* server, ot_doc* doc);

void ot_server_receive(ot_server* server, const char* op);

#endif

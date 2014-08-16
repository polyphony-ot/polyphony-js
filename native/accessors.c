#include "libot/doc.h"
#include "libot/server.h"

ot_doc* ot_server_get_doc(ot_server* server) {
	return server->doc;
}

ot_op* ot_doc_get_composed(ot_doc* doc) {
	return doc->composed;
}

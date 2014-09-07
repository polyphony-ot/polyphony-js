"use strict";

/**
 * Creates an OT server using a given SocketServer.
 * @param {SocketServer} socketServer - The WebSocket to listen on.
 * @constructor
 */
polyphony.Server = function(socketServer) {
    this.socketServer = socketServer;

    var sendFunc = Runtime.addFunction(function(stringPointer) {
        var string = Module.Pointer_stringify(stringPointer);
        socketServer.broadcast(string);
    });

    var nativeServer = cFuncs.otNewServer(sendFunc, null);
    var nativeDoc = cFuncs.otNewDoc();
    cFuncs.otServerOpen(nativeServer, nativeDoc);

    var lastId = 0;
    socketServer.onConnection = function(socket) {
        socket.onMessage = function(message) {
            cFuncs.otServerReceive(nativeServer, message);
        };

        lastId++;
        var message = {
            clientId: lastId
        };

        var nativeDocPtr = cFuncs.otServerGetDoc(nativeServer);
        if (nativeDocPtr !== 0) {
            var nativeOpPtr = cFuncs.otDocGetComposed(nativeDocPtr);
            if (nativeOpPtr !== 0) {
                var lastOpJSON = cFuncs.otEncode(nativeOpPtr);

                if (lastOpJSON) {
                    message.lastOp = lastOpJSON;
                }
            }
        }

        console.log("[INFO] Acknowledging new client.\n\tJSON:", message);
        socket.send(JSON.stringify(message));
    };

    this.nativeServer = nativeServer;
};

polyphony.Server.prototype = {
    set maxSize(max) {
        var nativeDocPtr = cFuncs.otServerGetDoc(this.nativeServer);
        cFuncs.otDocSetMaxSize(nativeDocPtr, max);
    },
    close: function() {
        this.socketServer.close();
    }
};

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

    var lastId = 0;
    socketServer.onConnection = function(socket) {
        socket.onMessage = function(message) {
            cFuncs.otServerReceive(nativeServer, message);
        };

        lastId++;
        var message = {
            clientId: lastId
        };

        var nativeDocPtr = Module.getValue(nativeServer + 8, "*");
        if (nativeDocPtr !== 0) {
            var nativeOpPtr = Module.getValue(nativeDocPtr + 16, "*");
            var lastOpJSON = cFuncs.otEncode(nativeOpPtr);

            if (lastOpJSON) {
                message.lastOp = lastOpJSON;
            }
        }

        console.log("Acknowledging new client: %O", message);
        socket.send(JSON.stringify(message));
    };
};

polyphony.Server.prototype = {
    close: function() {
        this.socketServer.close();
    }
};

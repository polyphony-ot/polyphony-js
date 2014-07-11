"use strict";

libot.Server = function(ws) {
    var WebSocketServer = ws.Server;
    this._socketSever = new WebSocketServer({
        port: 8080
    });

    this._socketSever.broadcast = function(data) {
        for (var i in this.clients) {
            this.clients[i].send(data);
        }
    };

    var sendFunc = Runtime.addFunction(function(stringPointer) {
        var string = Module.Pointer_stringify(stringPointer);
        this._socketSever.broadcast(string);
    }.bind(this));

    this._otServer = cFuncs.otNewServer(sendFunc, null);

    var lastId = 0;
    this._socketSever.on("connection", function(ws) {
        ws.on("message", function(message) {
            cFuncs.otServerReceive(this._otServer, message);
        }.bind(this));
        lastId++;
        console.log("Assigning client ID: " + lastId);
        var msg = { clientId: lastId };
        ws.send(JSON.stringify(msg));
    }.bind(this));
};

libot.Server.prototype = {
    close: function() {
        this._socketSever.close();
    }
};

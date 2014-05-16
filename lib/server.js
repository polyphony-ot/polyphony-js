"use strict";

libot.Server = function(ws) {
    var WebSocketServer = ws.Server;
    this._socketSever = new WebSocketServer({
        port: 8080
    });
    this._socketSever.on("connection", function(ws) {
        ws.on("message", function(message) {
            console.log("Server received: %s", message);
        });
    });
};

libot.Server.prototype = {
    close: function() {
        this._socketSever.close();
    }
};

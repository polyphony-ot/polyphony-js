/* eslint-env node, browser */

"use strict";

libot.Client = function(serverAddress) {
    this._socket = new WebSocket(serverAddress);
    this._socket.onopen = function() {
        console.log("Connection open!");
    };
    this._socket.onmessage = function(e) {
        console.log(e.data);
    };
};

libot.Client.prototype = {
    send: function(op) {
    	var json = libot.encode(op);
        console.log("Sending an operation for: " + json);
        this._socket.send(json);
    },
    close: function() {
        this._socket.close();
    }
};
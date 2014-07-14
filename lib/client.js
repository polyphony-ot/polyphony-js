/* eslint-env node, browser */

"use strict";

libot.Client = function(serverAddress) {
    this.onEvent = null;
    this._socket = new WebSocket(serverAddress);
    this._socket.onopen = function() {
        console.log("WebSocket connection open.");
    };

    var sendFunc = Runtime.addFunction(function(stringPointer) {
        var string = Module.Pointer_stringify(stringPointer);
        this._socket.send(string);
    }.bind(this));

    var eventFunc = Runtime.addFunction(function(type, nativeOp) {
        if (this.onEvent) {
            this.onEvent(type, new libot.Op(nativeOp));
        }
    }.bind(this));

    this._socket.onmessage = function(e) {
        if (!this._otClient) {
            var obj = JSON.parse(e.data);
            this._otClient = cFuncs.otNewClient(sendFunc, eventFunc, obj.clientId);
            cFuncs.otClientReceive(this._otClient, obj.lastOp);
            return;
        }
        cFuncs.otClientReceive(this._otClient, e.data);
    }.bind(this);
};

libot.Client.prototype = {
    apply: function(op) {
        var ptr = Module._malloc(4);
        Module.setValue(ptr, op.nativeOp, "*");
        console.time("native");
        cFuncs.otClientApply(this._otClient, ptr);
        console.timeEnd("native");
    },
    close: function() {
        this._socket.close();
    }
};

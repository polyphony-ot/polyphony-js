/* eslint-env node, browser */

"use strict";

polyphony.Client = function(serverAddress) {
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
            this.onEvent(type, new polyphony.Op(nativeOp));
        }
    }.bind(this));

    this._socket.onmessage = function(e) {
        if (!this._otClient) {
            var obj = JSON.parse(e.data);

            console.log("[INFO] Creating new client.\n\tClient ID: %d",
                obj.clientId);

            this._otClient = cFuncs.otNewClient(sendFunc, eventFunc);
            cFuncs.otClientSetId(this._otClient, obj.clientId);
            if (obj.lastOp) {
                cFuncs.otClientReceive(this._otClient, obj.lastOp);
            }
            return;
        }
        cFuncs.otClientReceive(this._otClient, e.data);
    }.bind(this);
};

polyphony.Client.prototype = {
    apply: function(op) {
        var ptr = Module._malloc(4);
        Module.setValue(ptr, op.nativeOp, "*");
        console.time("native");
        var error = cFuncs.otClientApply(this._otClient, ptr);
        console.timeEnd("native");

        return (error === 0);
    },
    close: function() {
        this._socket.close();
    }
};

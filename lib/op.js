"use strict";

/**
 * Creates an instance of Op.
 *
 * @param {number} clientId The ID of the client that's creating the operation.
 * @param {string} parent   The hash of the parent operation from which this
 *                          operation is derived.
 * @constructor
 */
libot.Op = function Op(clientId, parent) {
    this._otOp = cFuncs.otNewOp(clientId, parent);
};

libot.Op.prototype = {
    skip: function(count) {
        cFuncs.otSkip(this._otOp, count);
    },
    insert: function(text) {
        cFuncs.otInsert(this._otOp, text);
    },
    delete: function(count) {
        cFuncs.otDelete(this._otOp, count);
    },
    snapshot: function() {
        return cFuncs.otSnapshot(this._otOp);
    }
};

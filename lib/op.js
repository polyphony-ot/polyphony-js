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
    snapshot: function() {
        return cFuncs.otSnapshot(this._otOp);
    }
};

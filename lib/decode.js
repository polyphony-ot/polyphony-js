"use strict";

/**
 * Decodes a JSON string into an Op.
 *
 * @param {string} json The JSON string to decode.
 * @return {libot.Op} The decoded operation.
 */
libot.Decode = function(json) {
    var op = new libot.Op();
    var error = cFuncs.otDecode(op.nativeOp, json);
    if (error !== 0) {
        throw new Error("Couldn't decode JSON into a valid operation. Error " +
            "code " + error + ".");
    }

    return op;
};

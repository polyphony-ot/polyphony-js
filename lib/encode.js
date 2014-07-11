"use strict";

libot.encode = function(op) {
    return cFuncs.otEncode(op.nativeOp);
};

"use strict";

/**
 * Creates an instance of Op.
 * @param {NativeOp} [nativeOp] - A pointer to an existing native operation.
 * @constructor
 */
polyphony.Op = function Op(nativeOp) {
    if (!nativeOp) {
        nativeOp = cFuncs.otNewOp();
    }

    Object.defineProperty(this, "nativeOp", {
        get: function() {
            return nativeOp;
        }
    });
};

polyphony.Op.prototype = {
    get nativeOp() {
        return this.nativeOp;
    },
    apply: function(skipFunc, insertFunc, deleteFunc) {
        var json = cFuncs.otEncode(this.nativeOp);
        var jsonObject = JSON.parse(json);
        var index = 0;
        for (var i = 0; i < jsonObject.components.length; i++) {
            var component = jsonObject.components[i];
            switch (component.type) {
                case "skip":
                    if (skipFunc) {
                        skipFunc(index, component.count);
                    }
                    index += component.count;
                    break;
                case "insert":
                    if (insertFunc) {
                        insertFunc(index, component.text);
                    }
                    break;
                case "delete":
                    if (deleteFunc) {
                        deleteFunc(index, component.count);
                    }
                    index += component.count;
                    break;
            }
        }
    },
    skip: function(count) {
        cFuncs.otSkip(this.nativeOp, count);
    },
    insert: function(text) {
        cFuncs.otInsert(this.nativeOp, text);
    },
    delete: function(count) {
        cFuncs.otDelete(this.nativeOp, count);
    },
    snapshot: function() {
        return cFuncs.otSnapshot(this.nativeOp);
    }
};

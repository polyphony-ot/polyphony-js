"use strict";

libot.TextareaEditor = function(textarea, client, jsDiff) {
    this.jsDiff = jsDiff;
    var old = textarea.value;

    textarea.oninput = function() {
        var op = this.diffOp(old, textarea.value);
        client.send(op);
        old = textarea.value;
    }.bind(this);
};

libot.TextareaEditor.prototype = {
    diffOp: function(oldContents, newContents) {
        var changes = this.jsDiff.diffChars(oldContents, newContents);
        var op = new libot.Op(0, "");
        for (var i = 0; i < changes.length; i++) {
            var change = changes[i];
            if (change.added) {
                op.insert(change.value);
            } else if (change.removed) {
                op.delete(change.value.length);
            } else {
                op.skip(change.value.length);
            }
        }

        return op;
    }
};

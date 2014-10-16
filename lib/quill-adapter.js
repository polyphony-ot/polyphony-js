"use strict";

polyphony.QuillAdapter = function(editor, client) {
    this.editor = editor;

    var previousDelta;
    editor.on("text-change", function(delta, source) {
        if (client.maxSize > editor.getLength()) {
            previousDelta = editor.getContents();
        }

        if (source === "api") {
            return;
        }

        if (client.maxSize <= editor.getLength()) {
            editor.setContents(previousDelta.invert(delta));
            previousDelta = editor.getContents();

            if (this.onError) {
                this.onError("The maximum size of the document has been " +
                    "reached.");
            }
            return;
        }

        var op = this.deltaToOperation(delta);
        client.apply(op);
    }.bind(this));

    client.onEvent = function(type, op) {
        if (type === 3) {
            editor.editor.checkUpdate();
            return;
        }
        var delta = this.operationToDelta(op);
        editor.updateContents(delta);
    }.bind(this);
};

polyphony.QuillAdapter.prototype = {
    deltaToOperation: function(delta) {
        var op = new polyphony.Op();
        var position = 0;

        function fillSkips(index) {
            if (index > position) {
                var count = index - position;
                op.skip(count);
                position = index;
            }
        }

        delta.apply(
            function(index, text) {
                fillSkips(index);
                op.insert(text);
                position += text.length;
            },
            function(index, length) {
                fillSkips(index);
                op.delete(length);
            }
        );

        fillSkips(delta.endLength);

        return op;
    },
    operationToDelta: function(op) {
        var deltaOps = [];
        op.apply(
            function skipFunc(index, count) {
                var retain = {
                    start: index,
                    end: index + count
                };
                deltaOps.push(retain);
            },
            function insertFunc(index, text) {
                var insert = {
                    value: text
                };
                deltaOps.push(insert);
            }
        );

        return {
            startLength: this.editor.getLength(),
            ops: deltaOps
        };
    }
};

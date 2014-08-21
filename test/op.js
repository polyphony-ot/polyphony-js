/* eslint-env node, mocha */
/* eslint no-new: 0 */

"use strict";

var polyphony = require("../bin/debug/polyphony");
var assert = require("assert");

describe("Op", function() {
    describe("constructor", function() {
        it("should create a new native op if one isn't provided", function() {
            var op = new polyphony.Op();
            assert(op.nativeOp);
        });
    });

    describe("snapshot", function() {
        it("should return the correct document contents", function() {
            var op = polyphony.Decode('{ "clientId": 0, "parent": "0", "hash": "00", "components": [ { "type": "insert", "text": "abc" } ] }');
            var snapshot = op.snapshot();

            assert.strictEqual(snapshot, "abc");
        });

        it("should return an empty string for an empty op", function() {
            var op = polyphony.Decode('{ "clientId": 0, "parent": "0", "hash": "00", "components": [ ] }');
            var snapshot = op.snapshot();

            assert.strictEqual(snapshot, "");
        });
    });
});

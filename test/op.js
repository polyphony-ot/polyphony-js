/* eslint-env node, mocha */
/* eslint no-new: 0 */

"use strict";

var libot = require("../bin/debug/libot");
var assert = require("assert");

describe("Op", function() {
    describe("constructor", function() {
        it("shouldn't throw an exception with valid parameters", function() {
            var op = new libot.Op(0, "");
            assert(op);
        });
    });

    describe("snapshot", function() {
        it("should return the correct document contents", function() {
            var op = libot.Decode('{ "clientId": 0, "parent": "0", "components": [ { "type": "insert", "text": "abc" } ] }');
            var snapshot = op.snapshot();

            assert.strictEqual(snapshot, "abc");
        });

        it("should return an empty string for an empty op", function() {
            var op = libot.Decode('{ "clientId": 0, "parent": "0", "components": [ ] }');
            var snapshot = op.snapshot();

            assert.strictEqual(snapshot, "");
        });
    });
});

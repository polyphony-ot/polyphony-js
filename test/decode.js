/* eslint-env node, mocha */

"use strict";

var polyphony = require("../bin/debug/polyphony");
var assert = require("assert");

describe("Decode", function() {
    it("shouldn't throw an exception for a valid JSON operation", function() {
        var op = polyphony.Decode('{ "clientId": 0, "parent": "0", "components": [ { "type": "insert", "text": "abc" } ] }');
        assert(op);
    });

    it("should throw an exception for JSON operations without a clientId field", function() {
        assert.throws(function() {
            polyphony.Decode('{ "parent": "0", "components": [ { "type": "insert", "text": "abc" } ] }');
        });
    });

    it("should throw an exception for JSON operations without a parent field", function() {
        assert.throws(function() {
            polyphony.Decode('{ "clientId": 0, "components": [ { "type": "insert", "text": "abc" } ] }');
        });
    });

    it("should throw an exception for JSON operations without a components field", function() {
        assert.throws(function() {
            polyphony.Decode('{ "clientId": 0, "parent": "0" }');
        });
    });

    it("should throw an exception for JSON components without a type field", function() {
        assert.throws(function() {
            polyphony.Decode('{ "clientId": 0, "parent": "0", "components": [ { "text": "abc" } ] }');
        });
    });
});

/* eslint no-unused-vars:0 */

"use strict";

/**
 * An interface that defines methods for communicating with a single client.
 * @constructor
 */
polyphony.Socket = function() {};

polyphony.Socket.prototype = {
    /**
     * Called whenever a message is received.
     * @abstract
     * @param  {String} message - The received message.
     */
    onMessage: function(message) {},

    /**
     * Sends a message.
     * @abstract
     * @param  {String} message - The message to send.
     */
    send: function(message) {}
};

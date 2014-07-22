/* eslint no-unused-vars:0 */

"use strict";

/**
 * An interface that defines methods for communicating with clients via sockets.
 * @constructor
 */
polyphony.SocketServer = function() {};

polyphony.SocketServer.prototype = {
    /**
     * Broadcasts a message to all connected clients.
     * @abstract
     * @param {String} message - The message to broadcast.
     */
    broadcast: function(message) {},

    /**
     * Called whenever a new client connection occurs.
     * @abstract
     * @param {Socket} socket - A socket for the new connection.
     */
    onConnection: function(socket) {},

    /**
     * Closes the server and all of its connections.
     * @abstract
     */
    close: function() {}
};

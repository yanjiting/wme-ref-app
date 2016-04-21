// http://ejohn.org/blog/ecmascript-5-strict-mode-json-and-more/
"use strict";

// Optional. You will see this name in eg. 'ps' or 'top' command
process.title = 'call.io';

// Port where we'll run the websocket server
var webSocketsServerPort = 8080;

process.argv.forEach(function(val,index,array){
    if(val=="--port"){
        var inputServerPort = array[index+1];
        if(inputServerPort!=null && inputServerPort>0)
            webSocketsServerPort = inputServerPort;
    }
});

// websocket and http servers
var webSocketServer = require('websocket').server;
var http = require('http');

/**
 * Global variables
 */
var connections = {};

Array.prototype.remove = function() {
    var what, a = arguments, L = a.length, ax;
    while (L && this.length) {
        what = a[--L];
        while ((ax = this.indexOf(what)) !== -1) {
            this.splice(ax, 1);
        }
    }
    return this;
};

/**
 * HTTP server
 */
var server = http.createServer(function(request, response) {
    // Not important for us. We're writing WebSocket server, not HTTP server
});
server.listen(webSocketsServerPort, function() {
    console.log((new Date()) + " Server is listening on port " + webSocketsServerPort);
});

/**
 * WebSocket server
 */
var wsServer = new webSocketServer({
    // WebSocket server is tied to a HTTP server. WebSocket request is just
    // an enhanced HTTP request. For more info http://tools.ietf.org/html/rfc6455#page-6
    httpServer: server
});

function sendToRoom(roomId, data, self) {
    var rooms = connections[roomId].peers;
    var i, c;
    for(i = 0; i < rooms.length; i++)
    {
        c = rooms[i];
        if(c != self)
            c.sendUTF(data);
    }
}

// This callback function is called every time someone
// tries to connect to the WebSocket server
wsServer.on('request', function(request) {
    console.log((new Date()) + ' Connection from origin ' + request.origin + '.');

    // accept connection - you should check 'request.origin' to make sure that
    // client is connecting from your website
    // (http://en.wikipedia.org/wiki/Same_origin_policy)
    var connection = request.accept(null, request.origin); 
    
    var roomId = request.resourceURL.query['r'];
    connection.id = roomId;
    if(! connections[roomId]){
        console.log("no room is found , new array");
        connections[roomId] = {peers:[]};
    }
    connections[roomId].peers.push(connection);
    //request.resourceURL.pathname
    console.log((new Date()) + ' Connection accepted. roomId=' + roomId + ", head count=" + connections[roomId].peers.length);
    connection.sendUTF(JSON.stringify({type: "join", count: connections[roomId].peers.length}));
    if(connections[roomId].venueUrl && connections[roomId].venueUrl.length > 0)
    {
        console.log("Send to: --" + connections[roomId].venueUrl);
        connection.sendUTF(connections[roomId].venueUrl);
    }

    // user sent some message
    connection.on('message', function(message) {
        var textData;
        if(message.type === 'utf8') {
            textData = message.utf8Data;
        }
        else if (message.type === 'binary') { // accept only text
            textData = new Buffer(message.binaryData, 'binary').toString();
        }
        var cmd = JSON.parse(textData);
        if(cmd.type === "echo")
        {
            connection.sendUTF(JSON.stringify({type: "echo", count: connections[connection.id].peers.length}));
        }
        else
        {
            console.log(cmd);
            if(cmd.type === "venue")
            {
                connections[connection.id].venueUrl = JSON.stringify(cmd);
            }
            sendToRoom(connection.id, JSON.stringify(cmd), connection);
        }
    });

    // user disconnected
    connection.on('close', function(cn) {
        if(!connections[connection.id]){
            console.log(">>>>> close a socket but no room has found for it.");
            return;
        }
        connections[connection.id].peers.remove(connection);
        console.log("connection closed, roomId=" + connection.id + ", room size=" + connections[connection.id].peers.length);
        if(connections[connection.id].peers.length == 0){
            delete connections[connection.id];
        }
    });
});

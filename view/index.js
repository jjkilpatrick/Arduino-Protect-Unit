var app = require('express')(),           
	server = require('http').createServer(app).listen(8081),    
	io = require('socket.io').listen(server),
	Parse = require('parse').Parse,
	connected = false;

// Init Parse
Parse.initialize("kvJX1AbxaIA0s1GAoMFvyY7PXumIZrE0n6fT9dUa", "ZDnB3E5qO3SPQ4aliT7odkK1uzjO8haNWmttTiNG");

console.log("Listening for new clients on port 8081");
 
// respond to web GET requests with the index.html page:
app.get('/', function (request, response) {
	response.sendfile(__dirname + '/index.html');
});

// listen for new socket.io connections
// we don't start reading anyting until the browser has connected
io.sockets.on('connection', function (socket) {

	if (!connected) {
		console.log('browser connected');
		connected = true;
	}
	
});
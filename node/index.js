var serialport = require("serialport"),
	SerialPort  = serialport.SerialPort,    
	app = require('express')(),           
	server = require('http').createServer(app).listen(8080),    
	io = require('socket.io').listen(server),
	Parse = require('parse').Parse;

// open the USB serial port
var myPort = new SerialPort("/dev/tty.usbmodem1421", { 
	// look for return and newline at the end of each data packet
	parser: serialport.parsers.readline("\r\n")
});

// Init Parse
Parse.initialize("kvJX1AbxaIA0s1GAoMFvyY7PXumIZrE0n6fT9dUa", "ZDnB3E5qO3SPQ4aliT7odkK1uzjO8haNWmttTiNG");

console.log("Listening for new clients on port 8080");
var connected = false;
 
// respond to web GET requests with the index.html page:
app.get('/', function (request, response) {
	response.sendfile(__dirname + '/index.html');
});

// listen for new socket.io connections
// we don't start reading anyting until the browser has connected
io.sockets.on('connection', function (socket) {

	if (!connected) {
		// clear out any old data from the serial bufffer:
		myPort.flush(function(){});
		// send a byte to the serial port to ask for data:
		myPort.write('c');
		console.log('browser connected');
		connected = true;
	}

	// if the client disconnects:
	socket.on('disconnect', function () {
		myPort.write('x');
		console.log('browser disconnected');
		connected = false;
	});

	// listen for new serial data:  
	myPort.on('data', function (data) {
		// Convert the string received into a JSON object:
		var serialData = tryParseJSON(data);
		// send a serial event to the web client with the data:
		socket.emit('serialEvent', serialData);

		var Data = Parse.Object.extend("Data");
		var data = new Data();

		data.set("temperature", serialData.temperature);
		data.set("pressure", serialData.pressure);
		data.set("altitude", serialData.altitude);
		data.set("motion", serialData.motion);
		data.set("air", serialData.air);
		data.set("light", serialData.light);
		data.set("rain", serialData.rain);

		data.save(null, {
			success: function(data) {
				// Execute any logic that should take place after the object is saved.
				console.log('New object created with objectId: ' + data.id);
			},
			error: function(data, error) {
				// Execute any logic that should take place if the save fails.
				// error is a Parse.Error with an error code and message.
				console.log('Failed to create new object, with error code: ' + error.message);
			}
		});

	});
});

function tryParseJSON (jsonString){
	try {
		var o = JSON.parse(jsonString);
		if (o && typeof o === "object" && o !== null) {
			return o;
		}
	}
	catch (e) { }
	return false;
};
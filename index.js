var gamepad = require("gamepad");
var SerialPort = require('serialport');

// Inicia a conexão
var port = new SerialPort('/dev/ttyUSB2');

// Initialize the library
gamepad.init();

// List the state of all currently attached devices
for (var i = 0, l = gamepad.numDevices(); i < l; i++) {
    console.log(i, gamepad.deviceAtIndex());
}

// Create a game loop and poll for events
setInterval(gamepad.processEvents, 16);
// Scan for new gamepads as a slower rate
setInterval(gamepad.detectDevices, 500);

// Axis movement and accelerator
var movement_axis = 0;
var accelerator_axis = 0;

function pad(num, size) {
    var s = "000000000" + Math.abs(num);
    s = s.substr(s.length - size);
    if (num >= 0) {
        return s;
    }
    return '-' + s.substr(1);
}

// Send commands on each interval
function sendCommands() {
    /*

    left/right,acceleration
    
    left is negative
    right is positive
    acceleration is always positive 

    e.g:
        100, 100
        0, 100
        0, 80
        -100, 20
        -50, 80
    */
    var comando = pad(Math.ceil(99 * movement_axis), 3) + ',' + pad(Math.ceil(100 * accelerator_axis), 3);
    console.log(comando);

    // TODO SEND COMMAND OVER SERIAL
    // Check if serial is available and exists
    port.write(comando, function(err) {
        if (err) {
            return console.log('Error on write: ', err.message);
        }
        console.log('message written');
    });
}

// Read data from arduino
if (ALLOW_DEBUG) {
    port.on('readable', function() {
        console.log('Data:', port.read().toString('ascii'));
    });
}

// Open errors will be emitted as an error event 
port.on('error', function(err) {
    console.log('Error: ', err.message);
})

// Send commands for controller in each 50 ms
setInterval(sendCommands, 250);

var ALLOW_DEBUG = false;

// Listen for move events on all gamepads
gamepad.on("move", function(id, axis, value) {
    // Ignore Other Axis
    if (axis !== 0 && axis !== 5) return;

    // Debug messages
    if (ALLOW_DEBUG) {
        console.log("move", {
            id: id,
            axis: axis,
            value: value,
        });
    }

    // Store movement or accelerator
    if (axis == 0) {
        // Store movement (left/right) value   
        movement_axis = value;
        if (ALLOW_DEBUG) {
            console.log("new movement", movement_axis);
        }
    } else if (axis == 5) {
        // Store acceleration value
        if (value < 0) {
            accelerator_axis = (value + 1) * 0.5;
        } else {
            accelerator_axis = 0.5 + (value * 0.5);
        }
        if (ALLOW_DEBUG) {
            console.log("new acceleration", accelerator_axis);
        }
    }
});

// Listen for button up events on all gamepads
gamepad.on("up", function(id, num) {
    if (ALLOW_DEBUG) {
        console.log("up", {
            id: id,
            num: num,
        });
    }
});

// Listen for button down events on all gamepads
gamepad.on("down", function(id, num) {
    if (ALLOW_DEBUG) {
        console.log("down", {
            id: id,
            num: num,
        });
    }
});
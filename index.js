var gamepad = require("gamepad");
var SerialPort = require('serialport');

// Inicia a conexão
var port = new SerialPort('/dev/ttyUSB0');

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

// Communication state change if receiver stop send '#' or xbee is disconnected
var communication_state = false;

// Last message time
var lastMessageReceived = new Date().getTime();

// Add zeros before
function pad(num, size) {
    //var s = "000000000" + Math.abs(num);
    var s = "         " + Math.abs(num);
    s = s.substr(s.length - size);
    if (num >= 0) {
        return s;
    }
    return '-' + s.substr(1);
}

// Reconnection timer
var reconnectTime = null;
var enableReconnect = true;

// Try Reconect
function reconnect() {
    if (enableReconnect) {
        enableReconnect = false;
        reconnectTime = setInterval(function() {
            if (communication_state) {
                clearInterval(reconnectTime);
                return;
            }
            console.log("reconnecting!");
            port.write('*', function(err) {
                if (err) {
                    return console.log('Error on write reconnect : ', err.message);
                }
                console.log('message recconect written');
            });
        }, 500);
    }
}

// Send commands on each interval
function sendCommands() {
    /*

        left/right,acceleration
    
        left is negative
        right is positive
        left and right are a single variable (side)
        acceleration is always positive 
        message always have 9 chars
        the first char is always '>'
        the last one are always '\n'

        e.g:
            >100,100\n
            >000,100\n
            >000,080\n
            >001,100\n
            >-01,020\n
            >-50,080\n
        */
    if (!communication_state) {
        console.log("Communication Stopped");
        reconnect();
        return;
    }

    clearInterval(reconnectTime);

    var comando = '>' + pad(Math.ceil(99 * movement_axis), 3) + ',' + pad(Math.ceil(100 * accelerator_axis), 3) + '\n';
    //var comando = Math.ceil(99 * movement_axis) + ',' + Math.ceil(100 * accelerator_axis) + '\n';
    console.log(comando);

    // TODO SEND COMMAND OVER SERIAL
    // Check if serial is available and exists
    port.write(comando, function(err) {
        if (err) {
            return console.log('Error on write command : ', err.message);
        }
        console.log('command written');
    });
}

// Check if arduino is still alive
function keepAlive() {
    // Disable communication state till we receive a new confirmation message
    var newTime = new Date().getTime();
    if (communication_state !== false && newTime - lastMessageReceived >= 1000) {
        lastMessageReceived = newTime;
        communication_state = false;
        enableReconnect = true;
        console.log("DEAD -- X(");
    }
}

// Send commands for controller in each 100 ms
setInterval(sendCommands, 100);

// Check if arduino is still alive
setInterval(keepAlive, 1000);

// Enable or disable debug
var ALLOW_DEBUG = false;

// Read data from arduino to check if it's still alive
port.on('readable', function() {
    if (ALLOW_DEBUG) {
        console.log("Received");
        console.log('Data:', port.read().toString('ascii'));
    } else {
        var msg_read = port.read();
    }

    clearInterval(reconnectTime);
    communication_state = true;
    lastMessageReceived = new Date().getTime();
});

// Open errors will be emitted as an error event 
port.on('error', function(err) {
    if (ALLOW_DEBUG) {
        console.log('Error: ', err.message);
    }

    console.log('Xbee disconnected');
    // FERROU :X
});

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
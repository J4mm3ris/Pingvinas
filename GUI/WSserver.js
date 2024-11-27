const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');

const WebSocket = require('ws');
const wss = new WebSocket.Server({ port: 8080 });


wss.on('connection', (ws) => {
  console.log('WebSocket Client Connected');

  // Receive messages from React and forward to Arduino
  ws.on('message', (message) => {
    console.log(`Received from React: ${message}`);
    sendToArduino(message);
  });

  // Send Arduino data to React
  parser.on('data', (data) => {
    ws.send(data);
  });
});

// Replace with your Arduino's port
const port = new SerialPort({
  path: 'COM4', // Replace with your port (e.g., COM3 for Windows)
  baudRate: 115200,       // Match your Arduino sketch
});

// Create a parser to read serial data
const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));

// Open the port and handle events
port.on('open', () => {
  console.log('Serial Port Open');
});

parser.on('data', (data) => {
  console.log(`Arduino says: ${data}`);
});


const sendToArduino = (message) => {
  port.write(message + '\n', (err) => {
    if (err) {
      return console.error('Error on write:', err.message);
    }
    console.log(`Sent to Arduino: ${message}`);
  });
};

// Example: Send a message after 2 seconds
setTimeout(() => {
  sendToArduino('Hello Arduino!');
}, 2000);
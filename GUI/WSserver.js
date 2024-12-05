const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');

const WebSocket = require('ws');
const wss = new WebSocket.Server({ port: 8080 });


wss.on('connection', (ws) => {

  ws.on('message', (message) => {
    console.log(`Received from React: ${message}`);
    sendToArduino(message);
  });

  parser.on('data', (data) => {
    ws.send(data);
  });
});


const port = new SerialPort({
  path: 'COM7', 
  baudRate: 115200,      
});

const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));


port.on('open', () => {
  console.log('Serial Port Open');
});

parser.on('data', (data) => {
  console.log(`Arduino says: ${data}`);
});


const sendToArduino = (message) => {
  port.write(message + '\n', (err) => {
    if (err) {
      return console.error('Error :', err.message);
    }
    console.log(`Sent : ${message}`);
  });
};


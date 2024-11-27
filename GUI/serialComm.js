

// Replace with your Arduino's port


// Open Port
port.on('open', () => {
  console.log('Serial Port Open');
});

// Read data from Arduino
parser.on('data', (data) => {
  console.log(`Arduino says: ${data}`);
});

// Function to send data to Arduino
const sendToArduino = (message) => {
  port.write(message + '\n');
};

// Example of sending a message
setTimeout(() => {
  sendToArduino('Hello Arduino!');
}, 2000);
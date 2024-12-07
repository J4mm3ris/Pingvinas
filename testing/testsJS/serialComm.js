const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const readline = require('readline');

const port = new SerialPort({
  path: 'COM3',   
  baudRate: 112500  
});


const parser = port.pipe(new ReadlineParser({ delimiter: '\r\n' }));


const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
  prompt: '> '
});


rl.prompt();


parser.on('data', (data) => {
  console.log(`\n< Received: ${data}`);
  rl.prompt();
});


rl.on('line', (line) => {
  if (line.trim()) {
    port.write(line, (err) => {
      if (err) {
        console.error('Error on write:', err.message);
      } else {
        console.log(`> Sent: ${line}`);
      }
      rl.prompt();
    });
  } else {
    rl.prompt();
  }
});


port.on('error', (err) => {
  console.error('Serial port error:', err.message);
});

port.on('open', () => {
  rl.prompt();
});


port.on('close', () => {
  process.exit(0);
});

rl.on('SIGINT', () => {
  port.close();
});

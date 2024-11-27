"use client"; 

import React, { useEffect, useState } from 'react';

function GamepadController() {
  
  const [controllerIndex, setControllerIndex] = useState(null);
  const [buttons, setButtons] = useState([]);
  const [axes, setAxes] = useState([]);

  const [message, setMessage] = useState('');
  const [arduinoData, setArduinoData] = useState('');

  useEffect(() => {

    const handleGamepadConnected = (event) => {
      setControllerIndex(event.gamepad.index);
      console.log(`Gamepad connected: ${event.gamepad.id}`);
    };

    const handleGamepadDisconnected = (event) => {
      setControllerIndex(null);
      console.log(`Gamepad disconnected: ${event.gamepad.id}`);
    };

    window.addEventListener('gamepadconnected', handleGamepadConnected);
    window.addEventListener('gamepaddisconnected', handleGamepadDisconnected);

    return () => {
      window.removeEventListener('gamepadconnected', handleGamepadConnected);
      window.removeEventListener('gamepaddisconnected', handleGamepadDisconnected);
    };
  }, []);

  useEffect(() => {

    if (controllerIndex === null) return;

    const updateControllerState = () => {
      const gamepad = navigator.getGamepads()[controllerIndex];
      if (gamepad) {
        setButtons(gamepad.buttons.map(button => button.pressed));
        setAxes(gamepad.axes);
      }
      requestAnimationFrame(updateControllerState);
    };

    updateControllerState();
  }, [controllerIndex]);

  useEffect(() => {
    const socket = new WebSocket('ws://localhost:8080');

    socket.onopen = () => {
      console.log('WebSocket Connected');
    };

    socket.onmessage = (event) => {
      setArduinoData(event.data);
    };

    socket.onclose = () => {
      console.log('WebSocket Disconnected');
    };

    const sendMessage = () => {
      socket.send(message);
      setMessage('');
    };
    return () => socket.close(); 
  }, [message]);


  return (
    <div>
      <div>
        <h1>Controller Status:</h1>
        {controllerIndex === null ? (
          <p>No controller connected</p>
        ) : (
          <div>
            <p>Controller Index: {controllerIndex}</p>
            <h2>Buttons</h2>
            {buttons.map((pressed, index) => (
              <div key={index}>
                Button {index}: {pressed ? 'Pressed' : 'Not Pressed'}
              </div>
            ))}
            <h2>Axes</h2>
            {axes.map((value, index) => (
              <div key={index}>
                Axis {index}: {value.toFixed(2)}
              </div>
            ))}
          </div>
        )}
      </div>
      <div>
        -------------------------
      </div>
      <div>
        <h1>React & Arduino</h1>
        <input className='text-black inline'
          type="text"
          value={message}
          onChange={(e) => setMessage(e.target.value)}
        />
        <p>
          <button className='text-red-600 hover:text-amber-300 ' onClick={() => sendMessage()}>Send to Arduino</button>
        </p>
        <p>Arduino says: {arduinoData}</p>
      </div>
    </div>
  );

  
}

export default GamepadController;

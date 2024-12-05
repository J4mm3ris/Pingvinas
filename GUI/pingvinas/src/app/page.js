"use client"; 
import React, { useEffect, useState } from 'react';
import { Button } from "@material-tailwind/react";
function GamepadController() {

  const niceBTN = "bg-transparent hover:bg-blue-500 text-blue-700 font-semibold hover:text-white py-2 px-4 border border-blue-500 hover:border-transparent rounded";
  const kil = "bg-red-900 hover:bg-red-700 text-white font-semibold py-5 px-9 rounded-full hover:uppercase border border-blue-500 hover:text-black";
  const setsp  ="bg-green-600 hover:bg-green-400 text-white font-semibold py-2 px-3 rounded-full "
  
  // lets pray it works :P
  let socket = new WebSocket('ws://localhost:8080');

  socket.onclose = () => {
    console.error('Oh shit');
    connectWebSocket();
  }
  const connectWebSocket = () => {
    socket = new WebSocket('ws://localhost:8080');
  }
  
  const [controllerIndex, setControllerIndex] = useState(null);
  const [buttons, setButtons] = useState([]);
  const [axes, setAxes] = useState([]);

  const [speed, setMessage] = useState('0');
  const [Kp, setKp] = useState('1');
  const [Ki, setKi] = useState('1');
  const [Kd, setKd] = useState('1');

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

  

  socket.onmessage = (event) => {
    setArduinoData(event.data);
  };

  const sendSpeed = (spd) => {
    socket.send("^;"+spd);
    console.log("Updated speed");
  };
  const SendYawPID = () => {
    socket.send("$YAW;"+Math.floor(parseFloat(Kp)*1000).toString()+";"+Math.floor(parseFloat(Ki)*1000).toString()+";"+Math.floor(parseFloat(Kd)*1000).toString());
  };
  const SendPRPID = () => {
    socket.send("$PR;"+Math.floor(parseFloat(Kp)*1000).toString()+";"+Math.floor(parseFloat(Ki)*1000).toString()+";"+Math.floor(parseFloat(Kd)*1000).toString());

  };

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
      <div className='flex space-x-5 width:5 w-full'>
        <h1><p>Kp:</p><input className='text-black inline  sm:w-10'
          type="text"
          value={Kp}
          onChange={(e) => setKp(e.target.value)}
        /></h1>
        <h1><p>Ki:</p><input className='text-black inline sm:w-10'
          type="text"
          value={Ki}
          onChange={(e) => setKi(e.target.value)}
        /></h1>
        <h1><p>Kd:</p><input className='text-black inline sm:w-10'
          type="text"
          value={Kd}
          onChange={(e) => setKd(e.target.value)}
        /></h1>
        
      </div>
      <div className='flex w-max gap-4'>
        <p>
          <Button className={niceBTN}  onClick={() => SendYawPID()}>Set Yaw PID</Button>
        </p>
        <p>
          <Button className={niceBTN} onClick={() => SendPRPID()}>Set Pitch/Roll PID</Button>
        </p>
      </div>
      <div>
        <h1>React & Arduino</h1>
        <input className='text-black inline'
          type="text"
          value={speed}
          onChange={(e) => setMessage(e.target.value)}
        />
        <div className='inline w-full'>
          <button className={setsp} onClick={() => sendSpeed(speed)}>SetSpeed</button>
          <p></p>
          <button className={kil} onClick={() => sendSpeed(0)}>Kill Motors</button>
        </div>

        <p>Beep Boop: {arduinoData}</p>
      </div>
    </div>
  );

  
}

export default GamepadController;

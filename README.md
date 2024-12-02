
![Pingvinas](https://github.com/J4mm3ris/Pingvinas/blob/main/graphics/text.png)

## Shit! here we go again.

It might fly, *it might not.*

## Serial Commands

### Remote
>#### Emergency
>>**`MAYDAY MAYDAY MAYDAY`** - kills power to motors, requires manual reset,
>#### Modify core values
>>**`$YAW;{Kp};{Ki};{Kd}`** - set Kp, Ki, Kd values (multiplied by 1000) for YAW axis,\
>>**`$PR;{Kp};{Ki};{Kd}`** - set Kp, Ki, Kd values (multiplied by 1000) for PITCH and ROLL axes,
>#### Simple flight
>>**`&;{MotorSpeed};{TargetAngle(pitch)};{TargetAngle(roll)};{TargetAngle(yaw)}`** - set motor speed (MIN - 1160, MAX - TBD), target angle in degrees,
>#### Hover
>>**`^;{Speed}`** - set hover speed (resets angles to 0),
>#### Debug
>>**`%D`** - dump most important variables;

### Local

>**`#D{1/0}`** - enable / disable debug messages;

## Design process

![GIF](https://github.com/J4mm3ris/Pingvinas/blob/main/graphics/dezigningProzces.gif)



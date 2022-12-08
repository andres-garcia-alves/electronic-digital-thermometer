# Digital Thermometer

Arduino code for a digital thermometer (temperature, humidity and heat index) with auto-OFF functionality.

&nbsp;

This project use the following electronic components:
- 1 x Arduino UNO / Nano v3
- 1 x DHT11 temperature/humidity sensor
- 1 x 3461BS-1 display (4 digit, 7 segment multiplexed display)
- 8 x 1k5 ohm resistors
- 1 x Transistor BC547 NPN
- 1 x Electrolitic capacitor 470uF 10v
- 1 x Push-button
- 1 x Batery 9v


The circuit:
- Pressing the push-button charges the capacitor and puts the transistor in conducting state.
- After the button is released, the capacitor continues to power the base of the transistor for around 20 seconds.
- The current through the transistor powers the Arduino (and rest of the thermometer).
- Arduino constrols the temperature+humidity measurement and the multiplexing process for the 4 digit display.

Notes:
- This thermometer automatically turns off approximately 20 seconds after activation.
- To increase this time, change the capacitor to one of higher capacitance.

&nbsp;

### Screenshots

| Diagram A                          | Diagram B                          |
|------------------------------------|------------------------------------|
| ![](Resources/Diagrams-01.jpg)  | ![](Resources/Diagrams-02.jpg)  |

| Prototype                          |  Prototype                         |
|------------------------------------|------------------------------------|
| ![](Resources/Prototype-01.jpg) | ![](Resources/05-Prototype-02.jpg) |

| Building                           |  Building                          |
|------------------------------------|------------------------------------|
| ![](Resources/Building-01.jpg)  | ![](Resources/Building-04.jpg)  |

| Building                           |  Building                          |
|------------------------------------|------------------------------------|
| ![](Resources/Building-08.jpg)  | ![](Resources/Building-09.jpg)  |

| Project Final                      | Project Final (YouTube video)      |
|------------------------------------|------------------------------------|
| ![](Resources/Building-11.jpg)  | [![](Resources/Proyect-Final.jpg)](https://youtu.be/) |

See 'Rescources' sub-folder for more pictures of the proyect.

&nbsp;

### Version History

v1.0 (2022.12.08) - Initial release.  

&nbsp;

This source code is licensed under GPL v3.0  
Please send me your feedback about this app: andres.garcia.alves@gmail.com

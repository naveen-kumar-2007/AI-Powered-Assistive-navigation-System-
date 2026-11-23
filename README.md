# AI-Powered Assistive Navigation System for Visually Impaired People

An ESP8266-based assistive navigation prototype that detects nearby obstacles using an ultrasonic sensor, alerts the user through a vibration motor, and uploads GPS coordinates to ThingSpeak for remote viewing on a web page.

## Features

- HC-SR04 ultrasonic obstacle detection
- Vibration alert when an obstacle is within the configured distance
- NEO-6M GPS latitude and longitude reading
- Wi-Fi upload to ThingSpeak
- Web page displaying the latest coordinates on Google Maps

## Repository structure

```text
Smart_Blind_Stick_GitHub/
├── firmware/
│   └── Smart_Blind_Stick.ino
├── website/
│   └── index.html
├── README.md
└── LICENSE
```

## Hardware

- ESP8266 NodeMCU development board
- HC-SR04 ultrasonic sensor
- NEO-6M GPS module
- Vibration motor
- NPN transistor (for example, BC547 or 2N2222) and 1 kΩ base resistor
- Flyback diode suitable for the motor
- HC-SR04 ECHO voltage divider (for example, 1 kΩ from ECHO to D2 and 2 kΩ from D2 to GND)
- Regulated power supply suitable for each module

## Example NodeMCU connections

| Module | Pin | NodeMCU |
|---|---|---|
| HC-SR04 | TRIG | D1 |
| HC-SR04 | ECHO | D2 through voltage divider |
| HC-SR04 | VCC | regulated 5 V |
| HC-SR04 | GND | GND |
| NEO-6M | TX | D6 (ESP8266 software-serial RX) |
| NEO-6M | GND | GND |
| NEO-6M | VCC | according to the exact GPS breakout board specification |
| Vibration driver | transistor base through 1 kΩ | D5 |
| Vibration driver | emitter | GND |
| Motor | positive | suitable motor supply |
| Motor | negative | transistor collector |

Connect all circuit grounds together. The GPS module's RX connection is not needed for basic location reading. If connected, ESP8266 D7 is its TX output.

**Electrical safety:** ESP8266 GPIO pins are 3.3 V only and are not 5 V tolerant. Never connect HC-SR04 ECHO directly to D2. Never connect a motor directly to a GPIO. Use a transistor driver and flyback diode. Check the voltage/current ratings of your exact modules and battery regulator before powering the circuit.

## Arduino IDE setup

1. Install ESP8266 board support in Arduino IDE.
2. Select **NodeMCU 1.0 (ESP-12E Module)**.
3. Install the **TinyGPSPlus** library.
4. Open `firmware/Smart_Blind_Stick.ino`.
5. Replace `YOUR_WIFI_NAME`, `YOUR_WIFI_PASSWORD`, and `YOUR_THINGSPEAK_WRITE_API_KEY`.
6. Verify the pin assignments against your actual assembled circuit.
7. Upload the sketch and open Serial Monitor at 115200 baud.

The sketch expects GPS NMEA output at 9600 baud.

## ThingSpeak setup

1. Create an account at https://thingspeak.mathworks.com/.
2. Create a channel named `Smart Blind Stick`.
3. Enable Field 1 (Obstacle Distance) and Field 2 (Vibration Status).
4. Enable channel location tracking if available in your channel settings.
5. Copy the channel Write API Key into the firmware.
6. Copy the Channel ID into `website/index.html`.
7. For the simple website in this repository, set the channel to **Public**. A public channel makes its data visible to anyone who has the channel ID. Do not publish sensitive location data without consent.

The firmware sends distance, vibration status, latitude and longitude to ThingSpeak every 20 seconds. A valid, recent GPS fix is required for an upload.

## Publish the website with GitHub Pages

1. Create a GitHub repository.
2. Upload the contents of the `website` folder to the repository's root, or configure Pages to publish the `website` folder if using a supported deployment workflow.
3. In repository **Settings → Pages**, select the deployment source and branch.
4. Open the published Pages URL.

For the simplest setup, put `index.html` in the repository root.

## Testing checklist

- Confirm the ESP8266 connects to Wi-Fi.
- Test the ultrasonic sensor and verify distance readings in Serial Monitor.
- Verify the vibration motor operates through its transistor driver.
- Test GPS outdoors with a clear view of the sky; first fix may take time.
- Confirm ThingSpeak receives a new entry.
- Open the website and confirm the map shows the latest coordinates.

## Limitations

- GPS may not work reliably indoors or under heavy obstruction.
- ESP8266 uploads only when it has Wi-Fi internet access. For tracking away from Wi-Fi, provide internet through a phone hotspot or add a suitable cellular data module.
- This prototype is not a certified safety device and should not be relied upon as the only mobility aid.
- ThingSpeak and the website show the latest uploaded position, not guaranteed continuous real-time tracking.

## License

See [LICENSE](LICENSE).

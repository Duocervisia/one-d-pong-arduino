# One D Pong

![One D Pong](one-d-pong.jpg?raw=true)

This project is an LED Pong game based on an Arduino, utilizing the FastLED library. Two players can compete against each other by pressing buttons to return a ball. The game features adjustable difficulty and displays the current score on an LED strip.

## Table of Contents

- [Features](#features)
- [Components](#components)
- [Setup](#setup)
- [License](#license)

## Features

- Playable Pong Implementation: Allows two players to compete against each other.
- LED Display: Shows score, ball movements, and animations.
- Difficulty Adjustment: A potentiometer to adjust game speed and difficulty.

## Components

- ESP8266: Microcontroller with WiFi capabilities.
- LED Strip: Based on WS2812/NeoPixel, controlled via the FastLED library.
- Buttons: Two buttons for player controls.
- Potentiometer: To adjust game difficulty.

## Setup

1. Wiring: Use the schematic file to connect all peripherals. If needed, the controllers can be 3D printed for the buttons. The diameter of the buttons should be 12mm and they should have a maximum depth of 18mm. The .ipt file for the controllers is created using Autodesk Inventor.
2. Install Libraries: Ensure the following libraries are installed in your Arduino IDE:
   - FastLED
   - ezButton

3. Upload Code: Upload the provided code to your ESP8266.

## License

This project is licensed under the MIT License.
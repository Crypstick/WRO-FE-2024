# SSTalwarts

This repository contains engineering materials of a self-driven vehicle's model participating in the WRO Future Engineers competition in the season 2024.

## Content

- `t-photos` contains 2 photos of the team (an official one and one funny photo with all team members)
  - `Formal.jpeg` contains a formal picture of the team
  - `Informal.jpeg` contains a funny picture of the team
- `v-photos` contains 6 photos of the vehicle (from every side, from top and bottom)
  - `Back View.jpeg` contains 1 photo of the back view of our bot
  - `Bottom View.jpeg` contains 1 photo of the bottom view of our bot
  - `Front View.jpeg` contains 1 photo of the front view of our bot
  - `Left View.jpeg` contains 1 photo of the left view of our bot
  - `Right View.jpeg` contains 1 photo of the right view of our bot
  - `Top View.jpeg` contains 1 photo of the top view of our bot
- `video` contains the video.md file with the link to a video where driving demonstration exists
- `schemes` contains one or several schematic diagrams in form of JPEG, PNG or PDF of the electromechanical components illustrating all the elements (electronic components and motors used in the vehicle and how they connect to each other.
  - `Overall System  Diagram.png` contains a photo of our block diagram
  - ` Microcontroller System Diagram.png` contains a photo of our microcontroller system diagram
- `src` contains code of control software for all components which were programmed to participate in the competition
  1. `wro_final/wro_final.ino` is our main source code to operate the bot.
  2. `Evoeditted.zip` is our library required to run our microcontroller.
- `models` is for the files for models used by 3D printers, laser cutting machines and CNC machines to produce the vehicle elements. If there is nothing to add to this location, the directory can be removed.
  - `VL53LOX lego mount.stl` contains a 3d model of a mount for our time of flight sensor
  - `tcs3472 colour sensor lego mount.stl` contains 3d model of a mount for our colour sensor
- `other` is for other files which can be used to understand how to prepare the vehicle for the competition. It may include documentation how to connect to a SBC/SBM and upload files there, datasets, hardware specifications, communication protocols descriptions etc. If there is nothing to add to this location, the directory can be removed.

## Introduction

### Usage


1.⁠ ⁠Install the libraries found in the `Libraries_To_Add.txt` by going to the library manager in the Arduino IDE and installing them by name. **ALL LIBRARIRES ARE REQUIRED FOR THE CODE TO RUN CORRECTLY.**
2.⁠ ⁠Install `src/EvoEditted.zip` and follow the following on the Arduino IDE: `Sketch --> Include Library --> Add .ZIP Library --> Select EvoEditted.zip`.
3.⁠ ⁠Open the file `src/wro_final/wro_final.ino` with the Arduino IDE.

## Reminders before running the bot

- Software
  - Ensure that the board is set to ESP32S3
  - Ensure serial bot rate is 115200
  - Run!
- Hardware
  - Wires have been connected properly
  - Components are working properly
  - Structural parts are connected properly
  - Spaces on the wheels are correct
- Placement of the bot
  - Make sure the bot is placed straight
  - Compass is strongly secured on the bot

## Components used

1. Microcontroller: EvolutionX1, ESP32S3 x1
2. Motors: EV3 Medium motors x3
3. Camera: HuskeyLens x1
4. Distance Sensor: Time of Flight, VL53L0X x2
5. Compass: IMU, MPU9250 x1
6. Colour Sensor: TCS3472 x1
7. Charger: BQ25887 x1
8. I2C Multiplexor: TCA9548A x1
9. IO Multiiplexor: SX1506 x1
10. Batteries: lithium-ion battery 2s x4










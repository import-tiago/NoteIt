# NoteIt

[![Status](https://img.shields.io/badge/status-WIP-red)](https://github.com/TiagoPaulaSilva/NoteIt)

**NoteIt** is a datalogger for devices based on UART protocols.

It is powered by rechargeable battery, has micro-SD memory card storage with [FatFs](http://elm-chan.org/fsw/ff/00index_e.html), compatibility with 5V and 3.3V levels, automatic baud rate detector/manual baud rate selection; and, optionally, you can add date, time and temperature to the recorded data.

All settings can be easily configured via the OLED display and rotary encoder.

The content stored on the memory card can be accessed by a computer using a USB type C cable connected directly to the NoteIt, without the needed for an external USB/SD-Card adapter.

| ![](https://github.com/TiagoPaulaSilva/NoteIt/blob/main/Assets/PCB_Preview_01.jpg) | ![](https://github.com/TiagoPaulaSilva/NoteIt/blob/main/Assets/PCB_Preview_03.jpg) | ![](https://github.com/TiagoPaulaSilva/NoteIt/blob/main/Assets/PCB_Preview_02.jpg) |
|--|--|--|

This device was developed to be used in the simplest, carefree and agile way possible: just connect to the target device and know that the data is being saved.

- Don't worry about the file name: the management of files on the SD-Card is done automatically.
- Don't worry about voltage levels: being UART, the data will be registered.
- Don't worry about the datalogger power supply: it has its own power supply and is ultra low consumption, you can use it for weeks without an external source. If you want, you can use an external 5V source via USB cable type C.

## Screens Prototype Preview
### Home Screen
<p align="left"><img src="https://github.com/TiagoPaulaSilva/NoteIt/blob/main/Assets/Home_Screen_Prototype.png" width="50%" height="50%"></p>

## Schematic Preview
![image](https://github.com/TiagoPaulaSilva/NoteIt/blob/main/Assets/Schematic_Preview.jpg)

### Contributing
0. Give this project a :star:
1. Create an issue and describe your idea.
2. [Fork it](https://github.com/TiagoPaulaSilva/NoteIt/fork).
3. Create your feature branch (`git checkout -b my-new-feature`).
4. Commit your changes (`git commit -a -m "Added feature title"`).
5. Publish the branch (`git push origin my-new-feature`).
6. Create a new pull request.
7. Done! :heavy_check_mark:

### License Information
<p xmlns:cc="http://creativecommons.org/ns#" xmlns:dct="http://purl.org/dc/terms/"><a property="dct:title" rel="cc:attributionURL" href="https://github.com/TiagoPaulaSilva/NoteIt">NoteIt</a> by <a rel="cc:attributionURL dct:creator" property="cc:attributionName" href="https://twitter.com/tiagopsilvaa">Tiago Silva</a> is licensed under <a href="http://creativecommons.org/licenses/by-nc/4.0/?ref=chooser-v1" target="_blank" rel="license noopener noreferrer" style="display:inline-block;">CC BY-NC 4.0 <img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/cc.svg?ref=chooser-v1"><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/by.svg?ref=chooser-v1"><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/nc.svg?ref=chooser-v1"></a></p>

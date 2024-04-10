### About

That is a simple Arduino project presenting example usage of the LightFiles library. The program allows to store notes in the memory. Note consists of one-word title and a content. You can add, read and delete notes.
Since access to build-in flash memory is quite complex and requires modifications EEPROM is used instead.
Program was tested on `Arduino Mega 2560`.

### How to run

1. Add source files by selecting `Sketch->Add File...` and selecting all files in the `source` folder (root directory) separately.
2. Connect Arduino.
3. Flash program.
4. Change variable `FIRST_USE` to `false`.
5. Start testing by typing commands in `Tools->Serial Monitor`.

### Usage

All commands can be shown after executing `help` command in the serial monitor.

### Program structure

TBD

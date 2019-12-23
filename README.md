# SCUMM-VM for ESP32 / Odroid-Go

This is my try to port the SCUMM-VM (See: https://www.scummvm.org/) to ESP32. Currently it works with a lot of restrictions:
- Only works with MonkeyIsland1 CD-Version
- Only available engine is the SCUMM-Engine
- No sound
- No animated cursor
- No load/save function
...

But it proofs that SCUMM can run on the ESP32!

You can find an installable FW here: https://github.com/johannesbehr/scumm/blob/master/release/Go-Scumm.fw

To try you need the files monkey1.000 and monkey1.001. 
Put them in the folder: roms/scummvm/monkey1/

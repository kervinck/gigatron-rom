# Hardware
Hardware projects involving some or all of the **_gtemuAT67_** suite of software.<br/>

## Arduino Nano
Kicad files for an Arduino Nano USB interface to any decent PC/Laptop to allow the **_gtemuAT67_**<br/>
suite of tools to communicate with and control real **_Gigatron TTL_** hardware.<br/>
- Able to reset hardware.<br/>
- Able to download and execute code to hardware.<br/>
- Able to control hardware with keyboard emulation of the controller.<br/>
- Able to control hardware with emulation of a PS2 keyboard, (WozMon, Tiny BASIC, etc).<br/>
- Provides a physical and electrical pass through for real controllers/joysticks.<br/>

## ROM Switcher
Kicad files for a ROM switcher that relies on electrically erasable PROM devices for efficient<br/>
switching between multiple ROMS for ROM debugging and development.<br/>
- Uses cheap and easily available EEPROMS that are simple and fast to program with a decent ROM programmer.<br/>
- Uses a banked design to allow for two separate 64Kx16 ROMS to be contained within two 128Kx8 devices.<br/>
- Simple tools allow you to split ROM's into their separate instruction and data counterparts, which can then be<br/>
  burnt easily into the EEPROMS.<br/>
- A pass-through socket allows the original EPROM to be part of the ROM switching, effectively allowing for up to three<br/>
  unique ROM's to be installed.<br/>
- A simple set of momentary contact switches allow for board reset and switching between the three ROM's, with three different<br/>
  coloured LED's providing ROM identification.<br/>
https://www.youtube.com/watch?v=gdF33IdJ85U&feature=youtu.be

## BabelFish
A bespoke version of BabelFish that allows the **_gtemuAT67_** suite of software to control real hardware.<br/>
- Currently works with all versions of known ROM's.<br/>
- Provides USB control of physical hardware's menu.<br/>
- Provides emulation of PS2 keyboard.<br/>
- Provides physical controller pass-through.<br/>

## TODO
- **_BabelFish_** needs to be updated to the latest version.<br/>

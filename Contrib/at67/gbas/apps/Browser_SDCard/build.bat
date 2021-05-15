gtbasic Browser_SDCard_%1.gbas
python3 ..\..\..\tools\gtHexDump.py Browser_SDCard_%1.gt1 > Browser_SDCard_%1.h
if /I "%~1" == "ROMv2_v1"  copy Browser_SDCard_%1.h ..\..\..\..\gigatron-rom\Contrib\at67\hw\SDCard_NO_ROM\BabelFish
if /I "%~1" == "ROMv5a_v1" copy Browser_SDCard_%1.h ..\..\..\..\gigatron-rom\Contrib\at67\hw\SDCard_ROM\BabelFish
copy Browser_SDCard_%1.gt1 ..\..\..\..\roms\build

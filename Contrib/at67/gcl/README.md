# Building GCL under Windows
- This assumes that you have already cloned or downloaded the gigatron-rom repo.
- Do NOT do any of this under the Windows PowerShell, it won't work and you will get
  nonsensical errors, use a normal command prompt.

## Python Installation
~~~
1) You must install a 2.7.x version of Python, any of the 3.x versions
   will not work and you will need to modify the build/compile Python source,
   (not recommended as it is constantly being updated).
~~~

## Build GCL at a Command Prompt
~~~
1) cd <path to gigatron repo>
2) py Core\compilegcl.py <path to GCL file>

D:\> cd D:\gigatron_rom
D:\> py Core\compilegcl.py D:\gcl\test.gcl
~~~

## Building GCL in the Emulator
~~~
1) To build and automatically upload "gcl" programs under the emulator, add the absolute 
   path for the gigatron-rom repo directory to the "loader_config.ini" file:

[Comms]                ; case sensitive
BaudRate    = 115200   ; arduino software stack doesn't like > 115200
ComPort     = 0        ; can be an index or a name, eg: ComPort = COM5
Timeout     = 5.0      ; maximum seconds to wait for Gigatron to respond
GclBuild    = D:/gigatron-rom ; must be an absolute path, can contain spaces
~~~

# Building GCL under Linux/MacOS
## TODO

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

## Directory Structure
~~~
1) Create a new directory named "gcl" within "gigatron-rom" and then a "build"
   directory within "gcl":

gigatron-rom/gcl/build

2) Copy "asm.py", "compilegcl.py" and "gcl0x.py" from "gigatron-rom/Core" into the
   gigatron-rom/gcl/build directory you just created.

3) Copy "interface.json" from "gigatron-rom" into the gigatron-rom/gcl/build directory
   that you just created.

4) Your new directory structure should look like this:

gigatron-rom/gcl/build/asm.py
gigatron-rom/gcl/build/compilegcl.py
gigatron-rom/gcl/build/gcl0x.py
gigatron-rom/gcl/build/interface.json
~~~
    
## Batch File
~~~
1) Create a batch file "make.bat", (call it whatever you like as long as it has the bat
   extension), inside the "gcl" directory and type/paste the following into the batch file:

py build/compilegcl.py %1 -s build/interface.json

2) You can now build any "gcl" file within the "gcl" directory by typing the following at
   a command prompt within the "gcl" directory:

make test.gcl

3) Your new directory structure should look like this:

gigatron-rom/gcl/make.bat
gigatron-rom/gcl/build/asm.py
gigatron-rom/gcl/build/compilegcl.py
gigatron-rom/gcl/build/gcl0x.py
gigatron-rom/gcl/build/interface.json
~~~

## Building GCL in the Emulator
~~~
1) To build and automatically upload "gcl" programs under the emulator, add the absolute 
   path to the build directory you created above to the "loader_config.ini" file:

GclBuild = D:/Projects/Gigatron TTL/buildx64/gcl/build  ; must be an absolute path, can contain spaces
~~~

# Building GCL under Linux/MacOS
## TODO

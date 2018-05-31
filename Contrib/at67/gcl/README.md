# Building GCL under Windows

This assumes that you have already cloned or downloaded the gigatron-rom repo.<br/>
~~~
1) You must install a 2.7 version of Python, (I used 2.7.13), any of the 3.x versions
   will not work and you will need to modify the build/compile Python source,
   (not recommended as it is constantly being updated).
    
2) Do NOT do any of this under the Windows PowerShell, it won't work and you will get
   nonsensical errors, (I don't know why and I am really not interested, this one had
   me stumped for a few hours), use a normal command prompt.
    
3) I created a new directory named "gcl" within "gigatron-rom" and then a "build"
   directory within "gcl"; the reason for this is that Python seems to go a little
   silly with the creation of temporary/random directories.
    
4) I copied "asm.py", "compilegcl.py" and "gcl0x.py" into the new "gcl\build" directory.

5) I then created the ROM Symbol Table, change directory to your main gigatron-rom
   installation and either "make theloop.2.rom" or run the following command line:
    
-  python Core/theloop.py Apps/Snake.gcl Apps/Racer.gcl Apps/Mandelbrot.gcl Apps/Pictures.gcl
   Apps/Credits.gcl Apps/Loader.gcl Apps/Screen.gcl Apps/Main.gcl Core/Reset.gcl

-  copy "theloop.sym" into "gcl\build"

-  "gcl\build" should now contain "asm.py", "compilegcl.py", "gcl0x.py" and "theloop.sym";
   if any of these .py files are modified and you want your GCL build system to stay up to
   date, then you need to re-copy the new .py files.
    
6) Create a batch file "make.bat", (call it whatever you like as long as it has the bat
   extension), inside the "gcl" directory and type/paste the following into the batch file:

-  py build/compilegcl.py %1 build/theloop.sym
~~~
You can now build any "gcl" file within the "gcl" directory by typing the following at a<br/>
command prompt within the "gcl" directory:<br/>

e.g. "make test.gcl" this will build "test.gt1" assuming no errors.<br/>
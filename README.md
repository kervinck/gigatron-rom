# Apple 1 "15 Puzzle"
    

Files in the repo:

* **puzz15.asm**
    * The 6502 assembly language source code
    * I used [dasm](https://dasm-assembler.github.io/) to assemble this, but I'm imagine that other assemblers should work without too much tweaking
    
* **puzz15.bin**
    * Assembled binary file
    * This is dasm's default output format, so the first two bytes are the (little-endian) origin address of the code, and not the actual code itself.
    
* **puzz15.js**
    * Javascript "tape file" format, compatible with Will Scullin's [Apple 1js emulator](https://www.scullinsteel.com/apple1/)
    * You can use this with a local copy of the emulator by putting the file in the `/tapes` directory and adding a reference to it in `apple1.htm`
    
* **puzz15.txt**
    * The binary data in manually-typed "Woz Monitor" format
    * Many emulators will let you copy/paste or otherwise load this in
    * In theory, you could also send this over to a real Apple 1 (or replica/clone) via serial communication

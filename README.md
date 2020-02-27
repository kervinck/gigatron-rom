# Apple 1 "15 Puzzle"
    
<img src="https://github.com/JeffJetton/apple1-15-puzzle/blob/master/img/screenshot1.png" width="400">
    
[(View full example run)](https://github.com/JeffJetton/apple1-15-puzzle/blob/master/img/combined_screenshots.png)

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


# Sample Run

# Tweaks/Hacks

**Always ask for instructions**

In situtations where the game might be getting different players (a demo display, for example), you might want to have the option to view instructions after *every* game, rather than just when the program is first run. From the Monitor, just change one byte:

```
0394: 0B
```
(This sets the destination of a `jmp` instruction to a different entry point.)
   
**Skip title line**

The above change essentially starts the game over every round, including the initial display of `15 Puzzle - By Jeff Jetton`. You can tell the game to always skip that line by entering this in the Monitor:

```
030C: 95
```
(This bumps up the spot at which the program begins looking for the "welcome" text data.)

**Custom difficulty levels**

Game difficulty is determined by the number of "shuffles" a fresh (solved) puzzle gets. The easiest setting (level 1) just does three random moves to mix the board up for you, while the hardest (level 5) does 255.

The number of shuffles that correspond to each difficulty level can be found in bytes `$0533` to `$0536`:

![monitor screenshot](https://github.com/JeffJetton/apple1-15-puzzle/blob/master/img/monitor.png)

So if, for example, you wanted to make level two slightly harder, just increase that second value by a couple of shuffles:

```
0534: 0B
```


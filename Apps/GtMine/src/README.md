# Minesweeper for the GIGATRON

The game Minesweeper should be familiar to most.
The Gigatron is a 1980's style computer consisting of just a handful of 7400 series TTL chips.
Everything about the Gigatron can be found here:

https://gigatron.io/

https://forum.gigatron.io/

There are two compilers for the Gigatron. A BASIC and a C compiler. With the gtBASIC I had already made a game. It can be found here:

https://github.com/hans61/gtsokoban

https://forum.gigatron.io/viewtopic.php?t=300&start=10

To play a bit with the glcc compiler I decided to use Minesweeper. You can find the glcc here:

https://github.com/lb3361/gigatron-lcc

## My Version

![gtmine for Gigatron](picture/gtmine.jpg)

Game controls:

- Move cursor with the corresponding key on the keyboard or controller
- Mark field with SPACE or key B on the controller
- Uncover field with ENTER or key A on the controller
- Restart the game N key or Start key on the controller
- Change game strength
  - Beginner: key B
  - Advanced: key A
  - Expert: key E
  - Rotated with start on controller

## use SYS_Sprite6_v3

glcc now supports SYS_Sprite6

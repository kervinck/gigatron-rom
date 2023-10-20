# Gigatron
A collection of software and tools I developed for the [Gigatron TTL microcomputer](https://gigatron.io).

## Software

+ [Video Poker](#video-poker)
+ [Craps](#craps)
+ [Fish Tank](#fish-tank)

### Video Poker

A 5-card poker game for Gigatron developed in [at67's gtBASIC](https://forum.gigatron.io/viewtopic.php?f=4&t=232). Requires Gigatron ROMv3 or higher.

![Screenshot](screenshots/videopoker-02.png?raw=true)

**Gamepad Controls**
- **Left, Right** - Card selection
- **Up, Down** - Increase / Decrease bet
- **A Button** - Hold / Unhold card
- **B Button** - Deal / Draw

**Keyboard Controls**
- **Left, Right** - Card selection
- **Up, Down** - Increase / Decrease bet
- **Backspace, Delete, End** - Hold / Unhold card
- **Home, PrtScn/Insert/SysReq** - Deal / Draw

**Gameplay**

During the bet round, use **Up** or **Down** to increase or decrease your bet. The minimum bet is 1 credit and the maximum is 5 credits. Press **B** to deal the cards. Select cards using **Left** or **Right**. Press **A** to toggle Hold on the selected card. Press **B** to exchange unheld cards for new ones from the deck. If a winning hand results, you will be awarded the amount of credits corresponding to the illuminated win on the payout board at the top of the screen. The bet can now be increased or decreased again before the next round of cards is dealt.

Winning hands and rewards are as follows:

**Royal Flush:** 250 x Bet

**Straight Flush:** 50 x Bet

**Four of a Kind:** 25 x Bet

**Full House:** 9 x Bet

**Flush:** 6 x Bet

**Straight:** 4 x Bet

**Three of a Kind:** 3 x Bet

**Two Pair:** 2 x Bet

**Pair of Jacks or Better:** 1 x Bet

### Craps

Craps dice game for Gigatron developed in [at67's gtBASIC](https://forum.gigatron.io/viewtopic.php?f=4&t=232). Requires Gigatron ROMv3 or higher.

![Screenshot](screenshots/craps-01.png?raw=true)

**Gamepad Controls**
- **Up, Down** - Increase / Decrease bet
- **A Button** - Roll the dice
- **B Button** - Roll the dice

**Keyboard Controls**
- **Up, Down** - Increase / Decrease bet
- **Backspace, Delete, End** - Roll the dice
- **Home, PrtScn/Insert/SysReq** - Roll the dice

**Gameplay**

Craps is a dice game where you bet on the outcomes of the roll of a pair of dice.

Each round has two phases, "come-out" and "point", which are described below:

**Come-Out Phase**

First, wager your bet with **Up/Down**. 

Next, roll the dice by pressing **A** or **B** on the gamepad.

A come-out roll of 2 ("Snake Eyes"), 3 ("Craps Three"), or 12 ("Boxcars") is called "craps" or "crapping out" and you lose the bet.

A come-out roll of 7 ("Lucky Seven") or 11 ("Yo-leven") is a "natural" and you win the bet.

All other rolls (4, 5, 6, 8, 9, and 10) establish the "point", and you move onto the second phase of the game described below.

**Point Phase**

During the point phase, press **A** or **B** on the gamepad to start rolling the dice.

The dice will continue to roll until you either hit your point and win, or roll a 7 ("Seven Out") and lose.

Unlike the come-out phase, it is important to note that your bet is unaffected by rolls of 2, 3, 11, or 12 at this time.

### Fish Tank

**Work-in-Progress**

A fish tank simulation for Gigatron developed in [at67's gtBASIC](https://forum.gigatron.io/viewtopic.php?f=4&t=232). Requires Gigatron ROMv3 or higher.

![Screenshot](screenshots/fishtank-alpha-03.png?raw=true)

**Gamepad Controls**
- **A Button** - Spawn random fish
- **B Button** - Change gravel mixture

**Keyboard Controls**
- **Backspace, Delete, End** - Spawn random fish
- **Home, PrtScn/Insert/SysReq** - Change gravel mixture

## Tools
### CustomFish
A personalized version of the BabelFish firmware which supports Arduino Mega as well as breadboard buttons to load software.

### sendFile.py
Modification of the original which adds support for the Arduino Mega. Sends GT1 or BASIC files to the Gigatron from a computer.

### gigatron-gimp-palette.gpl
Gigatron Color Palette for GIMP.

### minipro
Usage examples and udev rules for minipro.

## ROM
Custom Gigatron ROM files (SD and non-SD versions).
See [Makefile.custom](rom/Makefile.custom) for included apps.

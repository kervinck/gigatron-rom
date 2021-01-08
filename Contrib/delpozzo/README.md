# Gigatron
A collection of software and tools I developed for the [Gigatron TTL microcomputer](https://gigatron.io).

## Software
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

## Tools
### CustomFish
A personalized version of the BabelFish firmware which supports Arduino Mega as well as breadboard buttons to load software.

### sendFile.py
Modification of the original which adds support for the Arduino Mega. Sends GT1 or BASIC files to the Gigatron from a computer.

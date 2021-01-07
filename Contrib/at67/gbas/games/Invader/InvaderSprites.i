const Barrier = 0
load sprite, ../../../res/image/Invader/Barrier.tga, Barrier, NoFlip

const Saucer = 1
load sprite, ../../../res/image/Invader/Saucer.tga, Saucer + 0, NoFlip
load sprite, ../../../res/image/Invader/Saucer.tga, Saucer + 1, FlipX

const Player = 3
load sprite, ../../../res/image/Invader/Player.tga, Player + 0, NoFlip
load sprite, ../../../res/image/Invader/Player.tga, Player + 1, FlipX

const InvBigLt = 5
load sprite, ../../../res/image/Invader/InvBig_0.tga, InvBigLt + 0, NoFlip
load sprite, ../../../res/image/Invader/InvBig_1.tga, InvBigLt + 1, NoFlip

const InvBigRt = 7 'instance and hardware flip, (native code), in the X direction to save memory
load sprite, ../../../res/image/Invader/InvBig_0.tga, InvBigRt + 0, FlipX
load sprite, ../../../res/image/Invader/InvBig_1.tga, InvBigRt + 1, FlipX

const InvMedLt = 9
load sprite, ../../../res/image/Invader/InvMed_0.tga, InvMedLt + 0, NoFlip
load sprite, ../../../res/image/Invader/InvMed_1.tga, InvMedLt + 1, NoFlip

const InvMedRt = 11 'instance and hardware flip, (native code), in the X direction to save memory
load sprite, ../../../res/image/Invader/InvMed_0.tga, InvMedRt + 0, FlipX
load sprite, ../../../res/image/Invader/InvMed_1.tga, InvMedRt + 1, FlipX

const InvSmlLt = 13
load sprite, ../../../res/image/Invader/InvSml_0.tga, InvSmlLt + 0, NoFlip
load sprite, ../../../res/image/Invader/InvSml_1.tga, InvSmlLt + 1, NoFlip

const InvSmlRt = 15 'instance and hardware flip, (native code), in the X direction to save memory
load sprite, ../../../res/image/Invader/InvSml_0.tga, InvSmlRt + 0, FlipX
load sprite, ../../../res/image/Invader/InvSml_1.tga, InvSmlRt + 1, FlipX

const InvBlk = 17
load sprite, ../../../res/image/Invader/InvBlk.tga, InvBlk, NoFlip

const IExplode = 18
load sprite, ../../../res/image/Invader/IExplode.tga, IExplode + 0, NoFlip
load sprite, ../../../res/image/Invader/IExplode.tga, IExplode + 1, FlipX

const PExplode = 20
load sprite, ../../../res/image/Invader/PExplode_0.tga, PExplode + 0, NoFlip
load sprite, ../../../res/image/Invader/PExplode_1.tga, PExplode + 1, NoFlip

const BExplode = 22
load sprite, ../../../res/image/Invader/BExplode.tga, BExplode, NoFlip

const BulBlk = 23
load sprite, ../../../res/image/Invader/BulBlk.tga, BulBlk, NoFlip

const PLife = 24
load sprite, ../../../res/image/Invader/PLife.tga, PLife, NoFlip

const PBlk = 25
load sprite, ../../../res/image/Invader/PBlk.tga, PBlk, NoFlip

const Digit = 26
load sprite, ../../../res/image/Invader/Zero.tga,  Digit + 0, NoFlip, 0
load sprite, ../../../res/image/Invader/One.tga,   Digit + 1, NoFlip, 0
load sprite, ../../../res/image/Invader/Two.tga,   Digit + 2, NoFlip, 0
load sprite, ../../../res/image/Invader/Three.tga, Digit + 3, NoFlip, 0
load sprite, ../../../res/image/Invader/Four.tga,  Digit + 4, NoFlip, 0
load sprite, ../../../res/image/Invader/Five.tga,  Digit + 5, NoFlip, 0
load sprite, ../../../res/image/Invader/Six.tga,   Digit + 6, NoFlip, 0
load sprite, ../../../res/image/Invader/Seven.tga, Digit + 7, NoFlip, 0
load sprite, ../../../res/image/Invader/Eight.tga, Digit + 8, NoFlip, 0
load sprite, ../../../res/image/Invader/Nine.tga,  Digit + 9, NoFlip, 0

const Level = 36
load sprite, ../../../res/image/Invader/Level.tga, Level, NoFlip, 0

const SExplode = 37
load sprite, ../../../res/image/Invader/SExplode_0.tga, SExplode + 0, NoFlip, 0
load sprite, ../../../res/image/Invader/SExplode_1.tga, SExplode + 1, NoFlip, 0

const SauBlk = 39
load sprite, ../../../res/image/Invader/SauBlk.tga, SauBlk, NoFlip, 0

const DigitS = 40
load sprite, ../../../res/image/Invader/Zero_s.tga,  DigitS + 0, NoFlip, 0
load sprite, ../../../res/image/Invader/One_s.tga,   DigitS + 1, NoFlip, 0
load sprite, ../../../res/image/Invader/Zero_s.tga,  DigitS + 2, NoFlip, 0
load sprite, ../../../res/image/Invader/Three_s.tga, DigitS + 3, NoFlip, 0
load sprite, ../../../res/image/Invader/Zero_s.tga,  DigitS + 4, NoFlip, 0
load sprite, ../../../res/image/Invader/Five_s.tga,  DigitS + 5, NoFlip, 0
load sprite, ../../../res/image/Invader/Zero_s.tga,  DigitS + 6, NoFlip, 0
load sprite, ../../../res/image/Invader/Zero_s.tga,  DigitS + 7, NoFlip, 0
load sprite, ../../../res/image/Invader/Zero_s.tga,  DigitS + 8, NoFlip, 0
load sprite, ../../../res/image/Invader/Zero_s.tga,  DigitS + 9, NoFlip, 0

const SaucerStripLt = 50
load sprite, ../../../res/image/Invader/Saucer_strip0.tga, SaucerStripLt + 0, NoFLip
load sprite, ../../../res/image/Invader/Saucer_strip1.tga, SaucerStripLt + 1, NoFLip
load sprite, ../../../res/image/Invader/Saucer_strip2.tga, SaucerStripLt + 2, NoFLip

const SaucerStripRt = 53
load sprite, ../../../res/image/Invader/Saucer_strip0.tga, SaucerStripRt + 0, FLipX
load sprite, ../../../res/image/Invader/Saucer_strip1.tga, SaucerStripRt + 1, FLipX
load sprite, ../../../res/image/Invader/Saucer_strip2.tga, SaucerStripRt + 2, FLipX
#ifndef AUDIO_H
#define AUDIO_H


#define GIGA_NUM_CHANNELS    4
#define GIGA_CHANNELS_MASK   3
#define GIGA_CHANNEL_OFFSET  0x0100
#define GIGA_SOUND_TIMER     0x002C

#define GIGA_CH0_WAV_A  0x01FA
#define GIGA_CH0_WAV_X  0x01FB
#define GIGA_CH0_KEY_L  0x01FC
#define GIGA_CH0_KEY_H  0x01FD
#define GIGA_CH0_OSC_L  0x01FE
#define GIGA_CH0_OSC_H  0x01FF

#define GIGA_CH1_WAV_A  0x02FA
#define GIGA_CH1_WAV_X  0x02FB
#define GIGA_CH1_KEY_L  0x02FC
#define GIGA_CH1_KEY_H  0x02FD
#define GIGA_CH1_OSC_L  0x02FE
#define GIGA_CH1_OSC_H  0x02FF

#define GIGA_CH2_WAV_A  0x03FA
#define GIGA_CH2_WAV_X  0x03FB
#define GIGA_CH2_KEY_L  0x03FC
#define GIGA_CH2_KEY_H  0x03FD
#define GIGA_CH2_OSC_L  0x03FE
#define GIGA_CH2_OSC_H  0x03FF

#define GIGA_CH3_WAV_A  0x04FA
#define GIGA_CH3_WAV_X  0x04FB
#define GIGA_CH3_KEY_L  0x04FC
#define GIGA_CH3_KEY_H  0x04FD
#define GIGA_CH3_OSC_L  0x04FE
#define GIGA_CH3_OSC_H  0x04FF

#define AUDIO_CONFIG_INI  "audio_config.ini"


namespace Audio
{
#ifndef STAND_ALONE
    bool getRealTimeAudio(void);

    void initialise(void);
    void initialiseChannels(void);
    void saveWaveTables(void);
    void restoreWaveTables(void);
    void initialiseEditor(void);

    void fillCallbackBuffer(void);
    void fillBuffer(void);
    void playBuffer(void);
    void playSample(void);
    void clearQueue(void);

    void process(void);
#endif
}

#endif
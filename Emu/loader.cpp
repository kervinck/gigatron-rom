#include <stdint.h>
#include <stdio.h>
#include <string>

#include "cpu.h"
#include "editor.h"
#include "loader.h"
#include "timing.h"


namespace Loader
{
    enum LoaderState {FirstByte=0, MsgLength, LowAddress, HighAddress, Message, LastByte, ResetIN, NumLoaderStates};
    enum FrameState {Resync=0, Frame, Execute, NumFrameStates};


    bool _startUploading = false;


    bool getStartUploading(void) {return _startUploading;}
    void setStartUploading(bool start) {_startUploading = start;}

    void sendByte(uint8_t value, uint8_t& checksum)
    {
        Cpu::setIN(value);
        checksum += value;
    }

    bool sendFrame(int vgaY, uint8_t firstByte, uint8_t* message, uint8_t len, uint16_t address, uint8_t& checksum)
    {
        static LoaderState loaderState = LoaderState::FirstByte;
        static uint8_t payload[PAYLOAD_SIZE];

        bool sending = true;

        switch(loaderState)
        {
            case LoaderState::FirstByte: // 8 bits
            {
                if(vgaY == VSYNC_START+8)
                {
                    for(int i=0; i<len; ++i) payload[i % PAYLOAD_SIZE] = message[i % PAYLOAD_SIZE];
                    sendByte(firstByte, checksum);
                    checksum += firstByte << 6;
                    loaderState = LoaderState::MsgLength;
                }
            }
            break;

            case LoaderState::MsgLength: // 6 bits
            {
                if(vgaY == VSYNC_START+14)
                {
                    sendByte(len, checksum);
                    loaderState = LoaderState::LowAddress;
                }
            }
            break;

            case LoaderState::LowAddress: // 8 bits
            {
                if(vgaY == VSYNC_START+22)
                {
                    sendByte(address & 0x00FF, checksum);
                    loaderState = LoaderState::HighAddress;
                }
            }
            break;

            case LoaderState::HighAddress: // 8 bits
            {
                if(vgaY == VSYNC_START+30)
                {
                    sendByte(address >> 8, checksum);
                    loaderState = LoaderState::Message;
                }
            }
            break;

            case LoaderState::Message: // 8*PAYLOAD_SIZE bits
            {
                static int msgIdx = 0;
                if(vgaY == VSYNC_START+38+msgIdx*8)
                {
                    sendByte(payload[msgIdx], checksum);
                    if(++msgIdx == PAYLOAD_SIZE)
                    {
                        msgIdx = 0;
                        loaderState = LoaderState::LastByte;
                    }
                }
            }
            break;

            case LoaderState::LastByte: // 8 bits
            {
                if(vgaY == VSYNC_START+38+PAYLOAD_SIZE*8)
                {
                    uint8_t lastByte = -checksum;
                    sendByte(lastByte, checksum);
                    checksum = lastByte;
                    loaderState = LoaderState::ResetIN;
                }
            }
            break;

            case LoaderState::ResetIN:
            {
                if(vgaY == VSYNC_START+39+PAYLOAD_SIZE*8)
                {
                    Cpu::setIN(0xFF);
                    loaderState = LoaderState::FirstByte;
                    sending = false;
                }
            }
            break;
        }

        return sending;
    }

    // TODO: fix the Gigatron version of upload so that it can send more than 60 bytes, (i.e. break up the payload into multiple packets of 60, 1 per packet per frame)
    void upload(int vgaY)
    {
        static bool frameUploading = false;
        static FILE* fileToUpload = NULL;
        static uint8_t payload[10000];
        static uint8_t payloadSize = 0;

        if(_startUploading == true  ||  frameUploading == true)
        {
            uint16_t hexBaseAddress = Editor::getHexBaseAddress();
            if(_startUploading == true)
            {
                _startUploading = false;
                frameUploading = true;
                std::string filename = std::string(".//vCPU//" + *Editor::getDirectoryName(Editor::getCursorY()));
                fileToUpload = fopen(filename.c_str(), "rb");
                if(fileToUpload == NULL)
                {
                    frameUploading = false;
                    return;
                }

                payloadSize = uint8_t(fread(payload, 1, PAYLOAD_SIZE, fileToUpload));
                fclose(fileToUpload);

                for(int i=0; i<payloadSize; i++) Cpu::setRAM(hexBaseAddress+i, payload[i]);

                Cpu::setRAM(0x0016, hexBaseAddress-2 & 0x00FF);
                Cpu::setRAM(0x0017, (hexBaseAddress & 0xFF00) >>8);
                Cpu::setRAM(0x001a, hexBaseAddress-2 & 0x00FF);
                Cpu::setRAM(0x001b, (hexBaseAddress & 0xFF00) >>8);
                frameUploading = false;
                return;
            }
            
            static uint8_t checksum = 0;
            static FrameState frameState = FrameState::Resync;

            switch(frameState)
            {
                case FrameState::Resync:
                {
                    if(sendFrame(vgaY, -1, payload, payloadSize, hexBaseAddress, checksum) == false)
                    {
                        checksum = 'g'; // loader resets checksum
                        frameState = FrameState::Frame;
                    }
                }
                break;

                case FrameState::Frame:
                {
                    if(sendFrame(vgaY,'L', payload, payloadSize, hexBaseAddress, checksum) == false)
                    {
                        frameState = FrameState::Execute;
                    }
                }
                break;

                case FrameState::Execute:
                {
                    if(sendFrame(vgaY, 'L', payload, 0, hexBaseAddress, checksum) == false)
                    {
                        checksum = 0;
                        frameState = FrameState::Resync;
                        frameUploading = false;
                    }
                }
                break;
            }
        }
    }

}

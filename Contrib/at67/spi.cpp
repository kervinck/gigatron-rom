#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cassert>

#include "spi.h"
#include "cpu.h"
#include "inih/INIReader.h"

#ifndef SPIVERBOSE
# define SPIVERBOSE 0
#endif

namespace Spi {

  // ---- bus composition

  static Device *spi0 = 0;
  static Device *spi1 = 0;

  void clock(uint16_t b, uint16_t a) {
    if (spi0)
      spi0->clock(b, a);
    if (spi1)
      spi1->clock(b, a);
  }
  
  bool config(INIReader &reader, const std::string &sectionString) {
    if (sectionString == "SD0") {
      spi0 = new SDCard(0);
      return spi0->config(reader, sectionString);
    }
    if (sectionString == "SD1") {
      spi1 = new SDCard(1);
      return spi1->config(reader, sectionString);
    }
    if (sectionString == "MCP1") {
      fprintf(stderr, "Spi::Device: MCP device is not yet supported\n");
    }
    return false;
  }



  // ---- generic device
  
  Device::Device(int num)
    : cs((num >= 0 && num < 4) ? (0x4 << num) : 0), mask(0)
  {
    if (num < 0 || num > 3)
      fprintf(stderr, "Spi::Device: port number must be in range 0..3\n");
  }

  uint8_t Device::spiselect(void)
  {
    // first byte returned after selecting the device
    return 0xff;
  }
  
  void Device::clock(uint16_t b, uint16_t a)
  {
    bool selected = (cs && !(a & cs));
    // cs was just asserted
    if (selected && (b & cs)) {
#if SPIVERBOSE
      fprintf(stderr, "sdi%d: selected\n", ffs(cs)-3);
#endif
      mask = 0x80;
      miso_byte = spiselect();
      Cpu::setXIN(miso_byte & mask ? 0xf : 0);
    }
#if SPIVERBOSE
    if (!selected && !(b & cs))
      fprintf(stderr, "sdi%d: deselected\n", ffs(cs)-3);
#endif
    // clock change
    if (selected && ((a ^ b) & 1))
      {
        if (a & 1)
          {
            // clock rising (latch)
            if (a & 0x8000)
              mosi_byte |= mask;
            else
              mosi_byte &= ~mask;
          }
        else
          {
            // clock falling (shift)
            if (! (mask >>= 1)) {
              mask = 0x80;
#if SPIVERBOSE
              fprintf(stderr,"sdi%d: sent 0x%02x recv 0x%02x\n", ffs(cs)-3, miso_byte, mosi_byte);
#endif
              miso_byte = spibyte(mosi_byte);
            }
            Cpu::setXIN((miso_byte & mask) ? 0xf : 0);
          }
      }
  }
  
  // ---- sdcard device


  static uint8_t crc7(const uint8_t *ptr, uint32_t count)
  {
    int i;
    uint8_t crc = 0;
    while (count--)
      {
        crc = crc ^ *ptr++;
        for (i=0; i<8; i++) {
          if (crc & 0x80)
            crc ^= 0x89;
          crc = (crc << 1);
        } 
      }
    // byte { crc7, 1 }
    return crc | 1;
  }
  
  static uint16_t crc16(const uint8_t *ptr, uint32_t count)
  {
    int i;
    uint16_t crc = 0;
    while (count--)
      {
        crc = crc ^ (((uint16_t)(*ptr++)) << 8);
        for (i=0; i<8; i++) {
          if (crc & 0x8000)
            crc = (crc << 1) ^ 0x1021;
          else
            crc = (crc << 1);
        } 
      }
    return crc;
  }
  
  SDCard::SDCard(int num)
    : Device(num), idle(1), state(0), count(0), len(0), fd(0)
  {
    buffer = new uint8_t[512];
  }

  SDCard::~SDCard()
  {
    delete [] buffer;
    if (fd)
      fclose((FILE*)(fd));
  }
  
  bool SDCard::config(INIReader &reader, const std::string &section)
  {
    std::string result;
    std::map<std::string, Type> typemap;
    typemap["NONE"] = NONE;
    typemap["MMC"] = MMC;
    typemap["SDSC"] = SDSC;
    typemap["SDHC"] = SDHC;

    type = NONE;
    result = reader.Get(section, "Type", "NONE");
    if (typemap.find(result) == typemap.end())
      fprintf(stderr, "Spi::SDCard: card type must be one of NONE, MMC, SDSC, SDHC\n");
    else
      type = typemap[result];
    if (type != NONE) {
      filename = reader.Get(section, "Filename", std::string());
      if (filename.empty()) {
        fprintf(stderr, "Spi:SDCard: no filename\n");
        type = NONE;
      }
    }
    if (type != NONE) {
      fd = (File*)fopen(filename.c_str(), "rb+");
      if (! fd) {
        fprintf(stderr, "Spi::SDCard: cannot open file: %s\n", filename.c_str());
        type = NONE;
      }
    }
    if (type != NONE && fd) {
      filelen = 0;
#ifdef _MSC_VER
      if (_fseeki64((FILE*)fd, 0, SEEK_END) >= 0)
        filelen = (long long)_ftelli64((FILE*)fd);
#elif defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L
      if (fseeko((FILE*)fd, 0, SEEK_END) >= 0)
        filelen = (long long)ftello((FILE*)fd);
#else
      if (fseek((FILE*)fd, 0, SEEK_END) >= 0)
        filelen = (long long)ftell((FILE*)fd);
#endif
      if (filelen <= 0) {
        fprintf(stderr, "Spi::SDCard: cannot measure file size: %s\n", filename.c_str());
        fclose((FILE*)fd);
        type = NONE;
        fd = 0;
      } else if (filelen % (256 * 1024)) {
        fprintf(stderr, "Spi::SDCard: file size should be a multiple of 256K: %s\n", filename.c_str());
        fclose((FILE*)fd);
        type = NONE;
        fd = 0;
      }
    }
    return true;
  }

  uint8_t SDCard::spiselect(void)
  {
    // This function is called whenever the spi port is selected (/SSx
    // falling edge).  Its return value is the byte transmitted to the
    // host during the next 8 clock cycles.
    //    In the case of a SD card, it also resets the state to CMD
    // (listening for a command) except during a long action (during a
    // read) or when the state is INIT or APPCMD which are also
    // listening for a command (but while uninitialized or after a
    // CMD55.)
    Context ctx = context();
    if (action() == BUSY)
      return 0x00;
    if (ctx != INIT && ctx != APPCMD)
      ctx = CMD;
    set_wait_state(ctx);
    return 0xff;
  }
  
  uint8_t SDCard::spibyte(uint8_t in)
  {
    // This function is called whenever a byte is exchanged.  Its
    // argument is the byte received by the slave.  Its return value
    // is the next byte to be sent by the slave.
    //    In the case of a SD card, what happens depends on the
    // card state represented by the (context,action) pair.
    Context c = context();
    Action a = action();
    // consistent action behavior
    switch(a)
      {
      case WAIT:
        if (in == 0xff || type == NONE) // waiting for master data
          return 0xff;
        break;
      case RECV:
        buffer[count] = in;
        if (++count < len)
          return 0xff;
        break;
      case SEND:
        if (c == READM && in == 64 + 12) // received CMD12!
          break;
        if (count < len)
          return buffer[count++];
        break;
      case BUSY: // busy state for len bytes
        if (count++ < len)
          return 0;
        break;
      }

    switch(c)
      {
      case INIT:
      case CMD:
      case APPCMD:
        {
          if (a == WAIT)      // got first byte
            set_recv_state(c, 6, in);
          else if (a == SEND || a == BUSY) // just sent reply
            set_wait_state(c);
          else                // got command
            sdcommand(c);
          return 0xff;
        }
      case REG:
        {
          assert(a == SEND);
          buffer[0] = 0xfe;
          buffer[16] = crc7(buffer+1, 15);
          uint16_t crc = crc16(buffer+1, 16);
          buffer[17] = (crc >> 8);
          buffer[18] = (crc & 0xff);
          set_send_state(CMD, 16+3);
          return 0xff;
        }
      case READ:
        {
          assert(a == SEND);
          if (! read_data())
            set_send_r1_state(CMD, 9); // send error token
          else
            set_send_state(CMD, 512+3);
          return 0xff;
        }
      case READM:
        {
          assert(a == SEND);
          if (in == 64 + 12)
            set_recv_state(CMD, 6, in);
          else if (! read_data())
            set_send_r1_state(CMD, 9); // send error token
          else
            set_send_state(READM, 512+3);
          offset += 512;
          return 0xff;
        }
      case WRITE:
        {
          assert (a == SEND);
          set_wait_state(WRITE1);
          return 0xff;
        }
      case WRITE1:
        {
          if (a == WAIT) {
            set_recv_state(WRITE1, 512+3, in);
          } else {
            set_busy_state(CMD, 4);
            if (buffer[0] == 0xfe && write_data())
              return 0x5;
            else
              return 0xd;
          }
          return 0xff;
        }
      case WRITEM:
        {
          assert(a == SEND || a == BUSY);
          set_wait_state(WRITEM1);
          return 0xff;
        }
      case WRITEM1:
        {
          if (a == WAIT && in == 0xfd) {
            set_busy_state(CMD, 4);   // stop tran
          } else if (a == WAIT) {
            set_recv_state(WRITEM1, 512+3, in);
          } else if (buffer[0] == 0xfc && write_data()) {
            offset += 512;
            set_busy_state(WRITEM, 4);
            return 0x5;
          } else {
            set_busy_state(CMD, 4);
            return 0xd;
          }
          return 0xff;
        }
      default:
        break;
      }
    set_send_r1_state(CMD, 4);
    return 0xff;
  }

  void SDCard::sdcommand(Context context)
  {
    int cmd = buffer[0] & 0x3f;
    if (context == INIT && memcmp(buffer, "\x40\0\0\0\0\x95", 6))
      {
        // In the init state, the only accepted command is command 0
        set_send_r1_state(INIT, 5);
        return;
      }
    if (buffer[0] != cmd + 64)
      {
        set_send_r1_state(CMD, 5);
        return;
      }
    if (context == APPCMD)
      {
        cmd += 128;
      }
    if (idle)
      {
        if (cmd != 0 && cmd != 1 && cmd != 8 && cmd != 128+41 && cmd != 55 && cmd != 58)
          cmd = 0xff;
      }
    switch(cmd)
      {
      case 128+41: // ACMD41: APP_SEND_OP_COND
            {
              if (type < SDSC)
                set_send_r1_state(CMD, 4 + idle);
              else {
                idle = 0;
                set_send_r1_state(CMD, 0);
              }
              break;
            }
      case 128+23: // ACMD22: SET_WR_BLOCK_ERASE_COUNT
        {
          // ignored but not illegal
          set_send_r1_state(CMD, 0);
          break;
        }
      case 0:  // CMD0: GO_IDLE_STATE
        {
          idle = 1;
          set_send_r1_state(CMD, idle);
          break;
        }
      case 1:  // CMD1: SEND_OP_COND
        {
          idle = 0;
          set_send_r1_state(CMD, 0);
          break;
        }
      case 8:  // CMD8: SEND_IF_COND
        {
          buffer[0] = idle;
          if (type < SDSC)
            set_send_r1_state(CMD, 4 + idle);
          else
            set_send_state(CMD, 5);
          break;
        }
      case 9:  // CMD9: SEND_CSD
        { 
          long size = filelen / (512 * 1024);
          if (type >= SDHC) {  //1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6
            memcpy(buffer, "\0" "\x40\x0e\x00\x32\x5b\x59\x00\x00\x00\x00\x7f\x80\x0a\x40\x40\xf1" "XX", 16 + 3);
            buffer[10] = (size & 0xff);
            buffer[9] = (size & 0xff00) >> 8;
            buffer[8] = (size & 0xcf0000) >> 16;
          } else {
            memcpy(buffer, "\0" "\x00\x0e\x00\x32\x5b\x59\x00\x00\x00\x00\x7f\x80\x0a\x40\x40\xf1" "XX", 16 + 3);
            buffer[7]  |= (size & 0xc00) >> 10;
            buffer[8]  |= (size & 0x3fc) >> 2;
            buffer[9]  |= (size & 0x3) << 6;
          }
          set_send_state(REG, 1);
          break;
        }
      case 10: // CMD10: SEND_CID
        {
          memcpy(buffer, "\0" "\xbbSD00000\x11\0\0\0\0\0\1\xf1" "XX", 16+3);
          set_send_state(REG, 1);
          break;
        }
      case 12: // CMD12: STOP_TRANSMISSION
        {
          set_busy_state(CMD, 3);
          break;
        }
      case 16: // CMD16: SET_BLOCK_LENGTH
        {
          long bl = (buffer[1]<<24)|(buffer[2]<<16)|(buffer[3]<<8)|(buffer[4]);
          if (bl != 512)
            set_send_r1_state(CMD, 64);
          else
            set_send_r1_state(CMD, 0);
          break;
        }
      case 17: // CMD17: READ_SINGLE_BLOCK
        {
          offset = (buffer[1]<<24)|(buffer[2]<<16)|(buffer[3]<<8)|(buffer[4]);
          if (type >= SDHC)
            offset *= 512;
          if (offset > filelen - 512)
            set_send_r1_state(CMD, 64);
          else
            set_send_r1_state(READ, 0);
          break;
        }
      case 18: // CMD18: READ_MULTIPLE_BLOCK
        {
          offset = (buffer[1]<<24)|(buffer[2]<<16)|(buffer[3]<<8)|(buffer[4]);
          if (type >= SDHC)
            offset *= 512;
          if (offset > filelen - 512)
            set_send_r1_state(CMD, 64);
          else
            set_send_r1_state(READM, 0);
          break;
        }
      case 23: // CMD23: SET_BLOCK_COUNT (MMC only)
        {
          set_send_r1_state(CMD, (type == MMC) ? 64 : 4); // unsupported
          break;
        }
      case 24: // CMD24: WRITE_SINGLE_BLOCK
        {
          offset = (buffer[1]<<24)|(buffer[2]<<16)|(buffer[3]<<8)|(buffer[4]);
          if (type >= SDHC)
            offset *= 512;
          if (offset > filelen - 512)
            set_send_r1_state(CMD, 64);
          else
            set_send_r1_state(WRITE, 0);
          break;
        }
      case 25: // CMD18: WRITE_MULTIPLE_BLOCK
        {
          offset = (buffer[1]<<24)|(buffer[2]<<16)|(buffer[3]<<8)|(buffer[4]);
          if (type >= SDHC)
            offset *= 512;
          if (offset > filelen - 512)
            set_send_r1_state(CMD, 64);
          else
            set_send_r1_state(WRITEM, 0);
          break;
        }
      case 55: // CMD55: APP_CMD
        {
          set_send_r1_state(APPCMD, idle);
          break;
        }
      case 58: // CMD58: READ_OCR
        {
          buffer[0] = 0;
          buffer[1] = (type >= SDHC) ? 0x40 : 0;
          buffer[2] = 0xff;
          buffer[3] = 0x80;
          buffer[4] = 0;
          set_send_state(CMD, 5);
          break;
        }
      default:
        {
          set_send_r1_state(CMD, 4 + idle);
          break;
        }
      }
  }

  bool SDCard::read_data()
  {
    uint16_t crc;
#if SPIVERBOSE
    fprintf(stderr, "sdi0: reading block at offset 0x%llx\n", offset);
#endif
#ifdef _MSC_VER
    if (fd == 0 || offset != (long long)(__int64)offset || _fseeki64((FILE*)fd, (__int64)offset, SEEK_SET) < 0)
      return false;
#elif defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L
    if (fd == 0 || offset != (long long)(off_t)offset || fseeko((FILE*)fd, (off_t)offset, SEEK_SET) < 0)
      return false;
#else
    if (fd == 0 || offset != (long long)(long)offset || fseek((FILE*)fd, (long)offset, SEEK_SET) < 0)
      return false;
#endif
    if (fread((void*)(buffer+1), 1, 512, (FILE*)fd) != 512)
      return false;
    crc = crc16(buffer+1, 512);
    buffer[0] = 0xfe;
    buffer[513] = (crc >> 8);
    buffer[514] = (crc & 0xff);
    return true;
  }
  
  bool SDCard::write_data()
  {
#if SPIVERBOSE
    fprintf(stderr, "sdi0: writing block at offset 0x%llx\n", offset);
#endif
#ifdef _MSC_VER
    if (fd == 0 || offset != (long long)(__int64)offset || _fseeki64((FILE*)fd, (__int64)offset, SEEK_SET) < 0)
      return false;
#elif defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L
    if (fd == 0 || offset != (long long)(off_t)offset || fseeko((FILE*)fd, (off_t)offset, SEEK_SET) < 0)
      return false;
#else
    if (fd == 0 || offset != (long long)(long)offset || fseek((FILE*)fd, (long)offset, SEEK_SET) < 0)
      return false;
#endif
    if (fwrite((void*)(buffer+1), 1, 512, (FILE*)fd) != 512)
      return false;
    return true;
  }
  
 
}  // namespace Spi

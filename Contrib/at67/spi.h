#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <inttypes.h>
#include <map>
#include <string>

class INIReader;
class File;

namespace Spi {

  void clock(uint16_t b, uint16_t a);
  bool config(INIReader &reader, const std::string &sectionString);

  class Device {
  public:
    Device(int port);
    virtual void clock(uint16_t b, uint16_t a);
    virtual bool config(INIReader &reader, const std::string &sectionString) = 0;
    virtual uint8_t spiselect(void);
    virtual uint8_t spibyte(uint8_t) = 0;
  protected:
    const uint8_t cs;
    uint8_t mask;
    uint8_t miso_byte;
    uint8_t mosi_byte;
  };

  class SDCard : public Device {
  protected:

    enum Type { NONE = 0, MMC, SDSC, SDHC };
    
    uint8_t idle;
    int     state;
    int     count;
    int     len;
    uint8_t *buffer;
    long long   offset;
    Type        type;
    std::string filename;
    long long   filelen;
    File       *fd;

    enum Action { WAIT=0, RECV=1, SEND=2, BUSY=3 };
    enum Context { INIT=0, CMD, APPCMD, REG, READ, READM, WRITE, WRITE1, WRITEM, WRITEM1 };
    
    Action action(void) {
      return Action(state & 0xf); }
    Context context(void) {
      return Context(state >> 4); }
    void set_wait_state(Context context) {
      state = (context<<4) | Action::WAIT; }
    void set_recv_state(Context context, int n) {
      state = (context<<4) | Action::RECV; len=n; count=0; }
    void set_recv_state(Context context, int n, uint8_t b) {
      state = (context<<4) | Action::RECV; len=n; buffer[0]=b; count=1; }
    void set_send_state(Context context, int n) {
      state = (context<<4) | Action::SEND; len=n; count=0; }
    void set_send_r1_state(Context context, uint8_t r1) {
      buffer[0] = r1; set_send_state(context, 1); }
    void set_busy_state(Context context, int n) {
      state = (context<<4) | Action::BUSY; len=n; count=0; }

    uint8_t spiselect(void);
    uint8_t spibyte(uint8_t);
    void    sdcommand(Context ctx);
    bool    read_data();
    bool    write_data();
  public:
    SDCard(int num);
    virtual ~SDCard();
    virtual bool config(INIReader &reader, const std::string &sectionString);
  };

}

#endif

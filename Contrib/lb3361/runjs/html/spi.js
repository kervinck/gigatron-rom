
const ACTION_WAIT = 0;
const ACTION_RECV = 1;
const ACTION_SEND = 2;
const ACTION_BUSY = 3;

const CTX_INIT    = 0;
const CTX_CMD     = 1;
const CTX_APPCMD  = 2;
const CTX_REG     = 3;
const CTX_READ    = 4;
const CTX_READM   = 5;
const CTX_WRITE   = 6;
const CTX_WRITE1  = 7;
const CTX_WRITEM  = 8;
const CTX_WRITEM1 = 9;

function crc7(b, j, c)
{
    let crc = 0;
    while(c--) {
        crc = crc ^ b[j++];
        for (let i = 0; i < 8; i++) {
            crc = (crc & 0x80) ? ((crc << 1) ^ 0x89) : (crc << 1);
        }
    }
    return crc & 0xff | 0x01;
}

function crc16(b, j, c)
{
    let crc = 0;
    while (c--) {
        crc = crc ^ (b[j++] << 8);
        for (let i = 0; i < 8; i++) {
            crc = (crc & 0x8000) ? ((crc << 1) ^ 0x1021) : (crc << 1);
        }
    }
    return crc & 0xffff;
}


export class Spi {
    /** Create a new Spi
     * @param {Gigatron} cpu
     * @param {Object} options
     */
    constructor(cpu, id) {
        this.cpu = cpu
        /* spi stuff */
        this.cs = (id >=0 && id < 4) ? (4 << id) : 0;
        this.mask = 0;
        this.miso = 0;
        this.mosi = 0;
        /* sd stuff */
        this.vhd = 0;
        this.vhdlen = 0;
        this.buffer = new Uint8Array(520);
        this.idle = 1;
        this.action = 0;
        this.context = 0;
        this.count = 0;
        this.len = 0;
        this.offset = 0;
        this.serial = 0;
    }

    /** Advance spi simulation by one tick */
    tick() {
        let b = this.cpu.prevctrl;
        if (b >= 0) {
            let a = this.cpu.ctrl;
            let selected = this.cs && !(a & this.cs);
            if (selected && (b & this.cs)) {
                this.mask = 0x80;
                this.miso = this.spiselect();
                this.cpu.miso = (this.miso & this.mask) ? 0xf : 0;
            }
            if (selected && ((a ^ b) & 1)) {
                if (a & 1) {
                    /* clock rising (latch) */
                    let mask = this.mask;
                    let mosi = this.mosi;
                    this.mosi =  (a & 0x8000) ? (mosi | mask) : (mosi & ~mask)
                } else {
                    /* clock falling (shift) */
                    this.mask >>= 1;
                    if (! this.mask) {
                        this.mask = 0x80;
                        this.miso = this.spibyte(this.mosi)
                    }
                    this.cpu.miso = (this.miso & this.mask) ? 0xf : 0;
                }
            }
        }
    }

    loadvhdurl(url) {
        let req = new XMLHttpRequest();
        req.open('GET', url);
        req.responseType = 'arraybuffer';
        /* req.setRequestHeader('accept-encoding','gzip'); */
        req.onload = (event) => {
            if (req.status == 200) {
                let vhd = new Uint8Array(req.response);
                if (vhd.length > 0) {
                    this.vhd = vhd;
                    this.vhdlen = vhd.length;
                    this.serial += 1;
                }
            }
        };
        req.onerror = (event) => {
            console.log("error while reading vhd from", url);
        };
        req.send();
    }

    loadvhdfile(file) {
        let reader = new FileReader();
        reader.onload = (event) => {
            let vhd = new Uint8Array(reader.result);
            if (vhd.length > 0) {
                this.vhd = vhd;
                this.vhdlen = vhd.length;
                this.serial += 1;
            }
        };
        reader.onerror = (event) => {
            console.log("error while reading vhd from", file);
        };
        reader.readAsArrayBuffer(file);
    }

    /** enable/disable SD card emulation */
    stop() {
        this.vhdlen = 0;
    }
    start() {
        if (this.vhd)
            this.vhdlen = this.vhd.length;
    }
    
    /** Set state (action,context) */
    set_wait_state(ctx) {
        this.context = ctx;
        this.action = ACTION_WAIT;
    }
    set_recv_state(ctx, n) {
        this.context = ctx;
        this.action = ACTION_RECV;
        this.len = n;
        this.count = 0;
    }
    set_recv_state1(ctx, n, b) {
        this.context = ctx;
        this.action = ACTION_RECV;
        this.len = n;
        this.buffer[0] = b;
        this.count = 1;
    }
    set_send_state(ctx, n) {
        this.context = ctx;
        this.action = ACTION_SEND;
        this.len = n;
        this.count = 0;
    }
    set_send_r1_state(ctx, r1) {
        this.buffer[0] = r1;
        this.set_send_state(ctx, 1);
    }
    set_busy_state(ctx, n) {
        this.context = ctx;
        this.action = ACTION_BUSY;
        this.len = n;
        this.count = 0;
    }

    /** Which byte is returned by the device when selected */
    spiselect() {
        if (this.action == ACTION_BUSY) {
            return 0x00;
        }
        if (this.context != CTX_INIT && this.context != CTX_APPCMD) {
            this.context = CTX_CMD;
        }
        this.set_wait_state(this.context);
        return 0xff;
    }

    /** Which byte is returned to the master when receiving mosi */
    spibyte(mosi) {
        let c = this.context;
        let a = this.action;
        switch(a) {
        case ACTION_WAIT:
            if (mosi == 0xff || this.vhdlen <= 0) { return 0xff; }
            break;
        case ACTION_RECV:
            this.buffer[this.count] = mosi;
            if (++this.count < this.len) { return 0xff; }
            break;
        case ACTION_SEND:
            if (c == CTX_READM && mosi == 64 + 12) { break; } // CMD12
            if (this.count < this.len) { return this.buffer[this.count++]; }
            break;
        case ACTION_BUSY:
            if (this.count++ < this.len) { return 0; }
            break;
        }
        switch(c) {
        case CTX_INIT:
        case CTX_CMD:
        case CTX_APPCMD:
            if (a == ACTION_WAIT) {
                this.set_recv_state1(c, 6, mosi);
            } else if (a == ACTION_SEND || a == ACTION_BUSY) {
                this.set_wait_state(c);
            } else {
                this.sdcommand(c);
            }
            return 0xff;
        case CTX_REG:
            this.buffer[0] = 0xfe;
            this.buffer[16] = crc7(this.buffer, 1, 15);
            let crc = crc16(this.buffer, 1, 16);
            this.buffer[17] = crc >> 8;
            this.buffer[18] = crc;
            this.set_send_state(CTX_CMD, 16 + 3);
            return 0xff;
        case CTX_READ:
            if (! this.read_data()) {
                this.set_send_r1_state(CTX_CMD, 9);
            } else {
                this.set_send_state(CTX_CMD, 512+3);
            }
            return 0xff;
        case CTX_READM:
            if (mosi == 64 + 12) {
                this.set_recv_state1(CTX_CMD, 6, mosi);
            } else if (! this.read_data()) {
                this.set_send_r1_state(CTX_CMD, 9);
            } else {
                this.set_send_state(CTX_READM, 512+3);
            }
            this.offset += 512;
            return 0xff;
        case CTX_WRITE:
            this.set_wait_state(CTX_WRITE1);
            return 0xff;
        case CTX_WRITE1:
            if (a == ACTION_WAIT) {
                this.set_recv_state1(CTX_WRITE1, 512+3, mosi);
            } else {
                this.set_busy_state(CTX_CMD, 4);
                return (this.buffer[0] == 0xfe && this.write_data()) ? 0x5 : 0xd;
            }
            return 0xff;
        case CTX_WRITEM:
            this.set_wait_state(CTX_WRITEM1);
            return 0xff;
        case CTX_WRITEM1:
            if (a == ACTION_WAIT && mosi == 0xfd) {
                this.set_busy_state(CTX_CMD, 4); // stop tran
            } else if (a == ACTION_WAIT) {
                this.set_recv_state1(CTX_WRITEM1, 512+3, mosi);
            } else if (this.buffer[0] == 0xfc && this.write_data()) {
                this.offset += 512;
                this.set_busy_state(ACTION_WRITEM, 4);
                return 0x5;
            } else {
                this.set_busy_state(CTX_CMD, 4);
                return 0xd;
            }
            return 0xff;
        }
        this.set_send_r1_state(CTX_CMD, 4);
        return 0xff;
    }

    sdcommand(ctx) {
        let buffer = this.buffer;
        let cmd = buffer[0] & 0x3f;
        if (ctx == CTX_INIT
            && !(buffer[0]==0x40 && buffer[1]==0 && buffer[2]==0
                 && buffer[3]==0 && buffer[4]==0 && buffer[5]==0x95) ) {
            this.set_send_r1_state(CTX_INIT, 5);
            return;
        }
        if (buffer[0] != cmd + 64) {
            this.set_send_r1_state(CTX_CMD, 5);
            return;
        }
        if (ctx == CTX_APPCMD) {
            cmd += 128;
        }
        if (this.idle && cmd != 0 && cmd != 1 && cmd != 8
            && cmd != 128+41 && cmd != 55 && cmd != 58) {
            cmd = 0xff;
        }
        switch(cmd) {
        case 128+41: // ACMD41: APP_SEND_OP_COND
            if (this.vhdlen == 0) {
                this.set_send_r1_state(CTX_CMD, 4 + this.idle)
            } else {
                this.idle = 0;
                this.set_send_r1_state(CTX_CMD, 0)
            }
            break;
        case 128+23: // ACMD22: SET_WR_BLOCK_ERASE_COUNT
            // ignored but not illegal
            this.set_send_r1_state(CTX_CMD, 0);
            break;
        case 0:  // CMD0: GO_IDLE_STATE
            this.idle = 1;
            this.set_send_r1_state(CTX_CMD, this.idle);
            break;
        case 1:  // CMD1: SEND_OP_COND
            this.idle = 0;
            this.set_send_r1_state(CTX_CMD, 0);
            break;
        case 8:  // CMD8: SEND_IF_COND
            buffer[0] = this.idle;
            if (this.vhdlen == 0) {
                this.set_send_r1_state(CTX_CMD, 4 + idle);
            } else {
                this.set_send_state(CTX_CMD, 5);
            }
            break;
        case 9: { // CMD9: SEND_CSD
            let size = this.vhdlen / (512 * 1024);
            buffer.set([0, 0x40, 0xe, 0, 0x32, 0x5b, 0x59, 0, 0,
                           0, 0, 0x7f, 0x80, 0xa, 0x40, 0x40, 0xf1], 0);
            buffer[10] = size & 0xff;
            buffer[9] = (size & 0xff00) >> 8;
            buffer[8] = (size & 0xcf0000) >> 16;
            this.set_send_state(CTX_REG, 1); }
            break;
        case 10: // CMD10: SEND_CID
            buffer.set([0x00, 0xbb, 83, 68, 48, 48, 48, 48, 48,
                              0x11, 0, 0, 0, 0, 0, 1, 0xf1, 88, 88], 0);
            buffer[11] = this.serial;
            buffer[12] = this.serial >> 8;
            this.set_send_state(CTX_REG, 1);
          break;
        case 12: // CMD12: STOP_TRANSMISSION
            this.set_busy_state(CTX_CMD, 3);
            break;
        case 16: { // CMD16: SET_BLOCK_LENGTH
            let bl = (buffer[1]<<24)|(buffer[2]<<16)|(buffer[3]<<8)|(buffer[4]);
            if (bl != 512) {
                this.set_send_r1_state(CTX_CMD, 64);
            } else {
                set_send_r1_state(CTX_CMD, 0);
            } }
            break;
        case 17: // CMD17: READ_SINGLE_BLOCK
            
            this.offset = (buffer[1]<<24)|(buffer[2]<<16)|(buffer[3]<<8)|(buffer[4]);
            this.offset *= 512;
            if (this.offset > this.vhdlen - 512) {
                this.set_send_r1_state(CTX_CMD, 64);
            } else {
                this.set_send_r1_state(CTX_READ, 0);
            }
            break;
        case 18: // CMD18: READ_MULTIPLE_BLOCK
            this.offset = (buffer[1]<<24)|(buffer[2]<<16)|(buffer[3]<<8)|(buffer[4]);
            this.offset *= 512;
            if (this.offset > this.vhdlen - 512) {
                this.set_send_r1_state(CTX_CMD, 64);
            } else {
                this.set_send_r1_state(CTX_READM, 0);
            }
            break;
        case 24: // CMD24: WRITE_SINGLE_BLOCK
            this.offset = (buffer[1]<<24)|(buffer[2]<<16)|(buffer[3]<<8)|(buffer[4]);
            this.offset *= 512;
            if (this.offset > this.vhdlen - 512) {
                this.set_send_r1_state(CTX_CMD, 64);
            } else {
                this.set_send_r1_state(CTX_WRITE, 0);
            }
            break;
        case 25: // CMD18: WRITE_MULTIPLE_BLOCK
            this.offset = (buffer[1]<<24)|(buffer[2]<<16)|(buffer[3]<<8)|(buffer[4]);
            this.offset *= 512;
            if (this.offset > this.vhdlen - 512) {
                this.set_send_r1_state(CTX_CMD, 64);
            } else {
                this.set_send_r1_state(CTX_WRITEM, 0);
            }
            break;
        case 55: // CMD55: APP_CMD
            this.set_send_r1_state(CTX_APPCMD, this.idle);
            break;
        case 58: // CMD58: READ_OCR
            buffer.set([0, 0x40, 0xff, 0x80, 0], 0);
            this.set_send_state(CTX_CMD, 5);
            break;
        default:
            this.set_send_r1_state(CTX_CMD, 4 + this.idle);
            break;
        }
        return 0;
    }

    read_data() {
        let offset = this.offset;
        let buffer = this.buffer;
        let vhd = this.vhd;
        buffer[0] = 0xfe;
        for (let i=0; i<512; i++)
            buffer[1 + i] = vhd[offset + i];
        let crc = 0; /* crc16(buffer, 1, 512) */
        buffer[513] = crc >> 8;
        buffer[514] = crc;
        return 1;
    }

    write_data() {
        let offset = this.offset;
        let buffer = this.buffer;
        let vhd = this.vhd;
        buffer[0] = 0xfe;
        for (let i=0; i<512; i++)
            vhd[offset + i] = buffer[1 + i];
        return 1;
    }
    
}

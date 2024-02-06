/*
    SingleFileDrive - Emulates a USB stick for easy data transfer
    Copyright (c) 2022 Earle F. Philhower, III.  All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <SingleFileDrive.h>
#include <LittleFS.h>

#include <functional>


Adafruit_USBD_MSC usb_msc;

SingleFileDrive singleFileDrive;


static const uint32_t _hddsize = (256 * 1024 * 1024); // 256MB
static const uint32_t _hddsects = _hddsize / 512;


SingleFileDrive::SingleFileDrive() {
}

SingleFileDrive::~SingleFileDrive() {
    end();
}



void SingleFileDrive::onDelete(void (*cb)(uint32_t), uint32_t cbData) {
    _cbDelete = cb;
    _cbDeleteData = cbData;
}

void SingleFileDrive::onPlug(void (*cb)(uint32_t), uint32_t cbData) {
    _cbPlug = cb;
    _cbPlugData = cbData;
}

void SingleFileDrive::onUnplug(void (*cb)(uint32_t), uint32_t cbData) {
    _cbUnplug = cb;
    _cbUnplugData = cbData;
}

bool SingleFileDrive::begin(const char *localFile, const char *dosFile) {
    if (_started) {
        return false;
    }
    usb_msc.setID("PicoDisk", "Mass Storage", "1.0");
    usb_msc.setCapacity(_hddsects, _hddsize/_hddsects);
    auto msc_read_callback = [](uint32_t lba, void* buffer, uint32_t bufsize)->int32_t { return singleFileDrive.read10(lba,0,buffer,bufsize); };
    auto msc_write_callback = [](uint32_t lba, uint8_t* buffer, uint32_t bufsize)->int32_t { return singleFileDrive.write10(lba, 0, buffer, bufsize); };
    auto msc_flush_callback = [](void)->void {};
    auto msc_start_stop_callback = [](uint8_t power_condition, bool start, bool load_eject)->bool { return singleFileDrive.startstop(power_condition, start, load_eject); };
    usb_msc.setReadWriteCallback(msc_read_callback, msc_write_callback, msc_flush_callback);
    usb_msc.setStartStopCallback(msc_start_stop_callback);
    usb_msc.setUnitReady(true);
    usb_msc.begin();
    _localFile = strdup(localFile);
    _dosFile = strdup(dosFile);
    _started = true;
    return true;
}

void SingleFileDrive::end() {
    usb_msc.setUnitReady(false);
    _started = false;
    free(_localFile);
    free(_dosFile);
    _localFile = nullptr;
    _dosFile = nullptr;
}

void SingleFileDrive::bootSector(char buff[512]) {
    // 256MB FAT16 stolen from mkfs.fat
    // dd if=/dev/zero of=/tmp/fat.bin bs=1M seek=255 count=1
    // mkfs.fat -F 16 -r 16 -n PICODISK -i 12345678 -s 128 -m ':(' /tmp/fat.bin
    const uint8_t hdr[] = {
        0xeb, 0x3c, 0x90, 0x6d, 0x6b, 0x66, 0x73, 0x2e, 0x66, 0x61, 0x74, 0x00,
        0x02, 0x80, 0x80, 0x00, 0x02, 0x00, 0x08, 0x00, 0x00, 0xf8, 0x80, 0x00,
        0x20, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00,
        0x80, 0x00, 0x29, 0x78, 0x56, 0x34, 0x12, 0x50, 0x49, 0x43, 0x4f, 0x44,
        0x49, 0x53, 0x4b, 0x20, 0x20, 0x20, 0x46, 0x41, 0x54, 0x31, 0x36, 0x20,
        0x20, 0x20, 0x0e, 0x1f, 0xbe, 0x5b, 0x7c, 0xac, 0x22, 0xc0, 0x74, 0x0b,
        0x56, 0xb4, 0x0e, 0xbb, 0x07, 0x00, 0xcd, 0x10, 0x5e, 0xeb, 0xf0, 0x32,
        0xe4, 0xcd, 0x16, 0xcd, 0x19, 0xeb, 0xfe, 0x3a, 0x28, 0x0d, 0x0a, 0x00
    };
    memset(buff, 0, 512);
    memcpy(buff, hdr, sizeof(hdr));
    buff[0x1fe] = 0x55;
    buff[0x1ff] = 0xff;
}

static char _toLegalFATChar(char c) {
    const char *odds = "!#$%&'()-@^_`{}~";
    c = toupper(c);
    if (((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'Z')) || strchr(odds, c)) {
        return c;
    } else {
        return '~';
    }
}

void SingleFileDrive::directorySector(char buff[512]) {
    const uint8_t lbl[] = {
        0x50, 0x49, 0x43, 0x4f, 0x44, 0x49, 0x53, 0x4b, 0x20, 0x20, 0x20, 0x08, 0x00, 0x00, 0xac, 0x56,
        0x82, 0x55, 0x82, 0x55, 0x00, 0x00, 0xac, 0x56, 0x82, 0x55
    }; //, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    memset(buff, 0, 512);
    memcpy(buff, lbl, sizeof(lbl));
    buff += 32; // Skip the just-set label

    // Create a legal 11-char UPPERCASE FILENAME WITH 0x20 PAD
    char SFN[11];
    memset(SFN, ' ', 11);
    for (int i = 0; (i < 8) && _dosFile[i] && (_dosFile[i] != '.'); i++) {
        SFN[i] = _toLegalFATChar(_dosFile[i]);
    }
    char *dot = _dosFile + strlen(_dosFile) - 1;
    while ((dot >= _dosFile) && (*dot != '.')) {
        dot--;
    }
    if (*dot == '.') {
        dot++;
        for (int i = 0; (i < 3) && dot[i]; i++) {
            SFN[8 + i] = _toLegalFATChar(dot[i]);
        }
    }
    uint8_t chksum = 0; // for LFN
    for (int i = 0; i < 11; i++) {
        chksum = (chksum >> 1) + (chksum << 7) + SFN[i];
    }

    // Create LFN structure
    int entries = (strlen(_dosFile) + 12) / 13; // round up
    for (int i = 0; i < entries; i++) {
        *buff++ = (entries - i) | (i == 0 ? 0x40 : 0);
        const char *partname = _dosFile + 13 * (entries - i - 1);
        for (int j = 0; j < 13; j++) {
            uint16_t u;
            if (j > (int)strlen(partname)) {
                u = 0xffff;
            } else {
                u = partname[j] & 0xff;
            }
            *buff++ = u & 0xff;
            *buff++ = (u >> 8) & 0xff;
            if (j == 4) {
                *buff++ = 0x0f; // LFN ATTR
                *buff++ = 0;
                *buff++ = chksum;
            } else if (j == 10) {
                *buff++ = 0;
                *buff++ = 0;
            }
        }
    }

    // Create SFN
    memset(buff, 0, 32);
    for (int i = 0; i < 11; i++) {
        buff[i] = SFN[i];
    }
    buff[0x0b] = 0x20; // ATTR = Archive
    // Ignore creation data/time, etc.
    buff[0x1a] = 0x03; // Starting cluster 3
    File f = LittleFS.open(_localFile, "r");
    int size = f.size();
    f.close();
    buff[0x1c] = size & 255;
    buff[0x1d] = (size >> 8) & 255;
    buff[0x1e] = (size >> 16) & 255; // 16MB or smaller
}

void SingleFileDrive::fatSector(char fat[512]) {
    memset(fat, 0, 512);
    fat[0x00] = 0xff;
    fat[0x01] = 0xf8;
    fat[0x02] = 0xff;
    fat[0x03] = 0xff;
    int cluster = 3;
    File f = LittleFS.open(_localFile, "r");
    int size = f.size();
    f.close();
    while (size > 65536) {
        fat[cluster * 2] = (cluster + 1) & 0xff;
        fat[cluster * 2 + 1] = ((cluster + 1) >> 8) & 0xff;
        cluster++;
        size -= 65536;
    }
    fat[cluster * 2] = 0xff;
    fat[cluster * 2 + 1] = 0xff;
}


int32_t SingleFileDrive::read10(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    if (!_started || (lba >= _hddsects)) {
        return -1;
    }

    uint32_t toread = bufsize;
    uint8_t *curbuff = (uint8_t *)buffer;

    while (bufsize > 0) {
        if (lba == 0) {
            bootSector(_sectBuff);
        } else if ((lba == 128) || (lba == 256)) {
            fatSector(_sectBuff);
        } else if (lba == 384) {
            directorySector(_sectBuff);
        } else if (lba >= 640) {
            File f = LittleFS.open(_localFile, "r");
            f.seek((lba - 640) * 512);
            f.read((uint8_t*)_sectBuff, 512);
            f.close();
        } else {
            memset(_sectBuff, 0, sizeof(_sectBuff));
        }

        uint32_t cplen = 512 - offset;
        if (bufsize < cplen) {
            cplen = bufsize;
        }
        memcpy(curbuff, _sectBuff + offset, cplen);
        curbuff += cplen;
        offset = 0;
        lba++;
        bufsize -= cplen;
    }

    return toread;
}

int32_t SingleFileDrive::write10(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    if (!_started || (lba >= _hddsects)) {
        return -1;
    }

    uint32_t addr = lba * 512 + offset;
    uint32_t hotspot = 384 * 512 + 0x20;
    if ((addr > hotspot) || (addr + bufsize < hotspot)) {
        // Did not try and erase the file entry, ignore
        return bufsize;
    }
    int off = hotspot - addr;
    uint8_t *ptr = (uint8_t *)buffer;
    ptr += off;
    if (*ptr == 0xe5) {
        if (_cbDelete) {
            _cbDelete(_cbDeleteData);
        }
    }

    return bufsize;
}


bool SingleFileDrive::startstop(uint8_t power_condition, bool start, bool load_eject)
{
  if (_started && load_eject)
  {
    if (start)
    {
      // load disk storage
      if (_cbPlug)
      {
          _cbPlug(_cbPlugData);
      }
    }
    else
    {
      // unload disk storage
      if (_cbUnplug)
      {
          _cbUnplug(_cbUnplugData);
      }
    }
  }
  return true;
}

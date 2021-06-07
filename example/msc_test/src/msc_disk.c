/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "tusb.h"
#include "esp_log.h"
#include "ffconf.h"
#include "ff.h"
#include "diskio.h"

#define DISK_BLOCK_SIZE CONFIG_DISK_BLOCK_SIZE
#define DEFAULT_PHYSICAL_DRIVER_NUM 0 //TODO:default 0, please care if mount two devices
#define LOGICAL_DISK_NUM 1

typedef int32_t wl_handle_t;
static bool ejected[LOGICAL_DISK_NUM] = {true};

void usb_msc_mount(void)
{
    // Reset the ejection tracking every time we're plugged into USB. This allows for us to battery
    // power the device, eject, unplug and plug it back in to get the drive.
    for (uint8_t i = 0; i < sizeof(ejected); i++) {
        ejected[i] = false;
    }
    ESP_LOGI(__func__, "");
}

void usb_msc_umount(void)
{
    ESP_LOGW(__func__, "");
}

bool usb_msc_ejected(void)
{
    bool all_ejected = true;
    for (uint8_t i = 0; i < sizeof(ejected); i++) {
        all_ejected &= ejected[i];
    }
    return all_ejected;
}

// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
  (void) lun;

  const char vid[] = "Espressif";
  const char pid[] = "Mass Storage";
  const char rev[] = "1.0";

  memcpy(vendor_id  , vid, strlen(vid));
  memcpy(product_id , pid, strlen(pid));
  memcpy(product_rev, rev, strlen(rev));
  ESP_LOGD(__func__, "");
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    ESP_LOGD(__func__, "");
    if (lun > 0) {
        return false;
    }

    if (ejected[lun]) {
        // Set 0x3a for media not present.
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3A, 0x00);
        return false;
    }

    return true;
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size)
{
    (void) lun;
    disk_ioctl(DEFAULT_PHYSICAL_DRIVER_NUM, GET_SECTOR_COUNT, block_count);
    disk_ioctl(DEFAULT_PHYSICAL_DRIVER_NUM, GET_SECTOR_SIZE, block_size);
    ESP_LOGD(__func__, "GET_SECTOR_COUNT = %d，GET_SECTOR_SIZE = %d",*block_count,*block_size);
}

bool tud_msc_is_writable_cb(uint8_t lun)
{
    if (lun > 1) {
        return false;//only have one LUN
    }
    ESP_LOGD(__func__, "");
    return true;
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
    (void) lun;
    (void) power_condition;
    ESP_LOGI(__func__, "");

    if (load_eject) {
        if (!start) {
            // Eject but first flush.
            if (disk_ioctl(DEFAULT_PHYSICAL_DRIVER_NUM, CTRL_SYNC, NULL) != RES_OK) {
                return false;
            } else {
                ejected[0] = true;
            }
        } else {
            // We can only load if it hasn't been ejected.
            return !ejected[0];
        }
    } else {
        if (!start) {
            // Stop the unit but don't eject.
            if (disk_ioctl(DEFAULT_PHYSICAL_DRIVER_NUM, CTRL_SYNC, NULL) != RES_OK) {
                return false;
            }
        }
        // Always start the unit, even if ejected. Whether media is present is a separate check.
    }

    return true;
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
    (void) lun;
    //ESP_LOGI(__func__, "");
    const uint32_t block_count = bufsize / DISK_BLOCK_SIZE;
    disk_read(DEFAULT_PHYSICAL_DRIVER_NUM, buffer, lba, block_count);
    return block_count * DISK_BLOCK_SIZE;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
    (void) lun;
    (void) offset;
    //ESP_LOGI(__func__, "");
    const uint32_t block_count = bufsize / DISK_BLOCK_SIZE;
    disk_write(DEFAULT_PHYSICAL_DRIVER_NUM, buffer, lba, block_count);
    return block_count * DISK_BLOCK_SIZE;
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
int32_t tud_msc_scsi_cb (uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize)
{
  // read10 & write10 has their own callback and MUST not be handled here
  ESP_LOGD(__func__, "");

  void const* response = NULL;
  uint16_t resplen = 0;

  // most scsi handled is input
  bool in_xfer = true;

  switch (scsi_cmd[0])
  {
    case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
      // Host is about to read/write etc ... better not to disconnect disk
      resplen = 0;
    break;

    default:
      // Set Sense = Invalid Command Operation
      tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

      // negative means error -> tinyusb could stall and/or response with failed status
      resplen = -1;
    break;
  }

  // return resplen must not larger than bufsize
  if ( resplen > bufsize ) resplen = bufsize;

  if ( response && (resplen > 0) )
  {
    if(in_xfer)
    {
      memcpy(buffer, response, resplen);
    }else
    {
      // SCSI output
    }
  }

  return resplen;
}

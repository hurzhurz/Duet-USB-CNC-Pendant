# Configuration File
(work in progress)

Instead of just hard coded values, some settings are now possible to set via a config file.

It is a YAML file named "config.yml" stored on a LittleFS partition on the flash memory of the RP2040.

The config file (with default values) can be found here: [data/config.yml](/data/config.yml)

## Config Upload

To upload the config file to the Raspberry Pi Pico, there are two options:
1. via PlatformIO function "Upload Filesystem Image" which takes the file from the data directory, builds an image and uploads it
2. without any installed tool:
   1. Create filesystem image using my browser/javascript based [littlefs-image-creator](https://hurzhurz.github.io/littlefs-image-creator/)
      * apply "512KB FS" preset
      * select prepared "config.yml" from your disk
      * click "Create UF2 file"
   2. boot Raspberry Pi Pico in BOOTSEL mode
      * by pressing BOOTSEL button while connection
      * or by using "Reboot Raspberry Pi Pico to BOOTSEL mode via serial" button on the littlefs-image-creator page
   3. copy littlefs.uf2 file to the appearing flash drive

## Config Download

When normally booted, the Raspberry Pico will present a flash drive.  
This drive will show a read-only version of the currently used config file.  
This can be used to download the current config to then edit it and upload it via the above mention methods.
Directly uploading via this flash drive is not possible, as the used SingleFileDrive library doesn't support write access.

## YAML Structure

For details, look at the example [config.yml](/data/config.yml).  
The config for each device will be merged from 3 possible sections:
1. "default" section: should contain shared config for all devices and device types
2. CLASSNAME (e.g. "WHB04B-6") section: should contain config shared for all devices of this type
3. CLASSNAME_VIDxxxx_PIDxxxx (e.g. "Numpad_VID062A_PID4101") section: can contain config specific for a single device, for example if a separate keyboard and numpad are used.  
This requires that the devices can be differentiated by the vendor or product id.


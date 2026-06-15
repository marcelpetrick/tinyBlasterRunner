# Tiny Blaster Runner

Side-scrolling arcade shooter for the TENSTAR T-Display ESP32 clone (135×240 ST7789 display, two buttons). Built with native ESP-IDF.

**Author: Marcel Petrick <mail@marcelpetrick.it>**

**Note: projected is generated with AI.**

**License: GPLv3 or later. See `LICENSE`.**

## Environment

```sh
source /home/mpetrick/.local/opt/esp-idf-v5.5.4/export.sh
```

## Build

```sh
idf.py build
```

## Flash And Monitor

```sh
idf.py -p /dev/serial/by-id/usb-1a86_USB_Single_Serial_5B0A006803-if00 flash monitor
```

## Monitor Only (no reflash)

```sh
idf.py -p /dev/serial/by-id/usb-1a86_USB_Single_Serial_5B0A006803-if00 monitor
```

Press `Ctrl-]` to exit the monitor.

## Build/Flash Helper Script

`build_flash.sh` wraps the common `idf.py` commands. It auto-sources the IDF environment if needed and defaults to `/dev/ttyACM0` at 460800 baud.

```sh
./build_flash.sh build      # compile only
./build_flash.sh deploy     # build + flash
./build_flash.sh monitor    # open serial monitor
./build_flash.sh all        # build + flash + monitor
```

Override port or baud if needed:

```sh
PORT=/dev/ttyUSB0 FLASH_BAUD=115200 ./build_flash.sh deploy
```

## Clean Build

```sh
idf.py fullclean
```

## Erase Flash (restore blank device)

```sh
idf.py -p /dev/serial/by-id/usb-1a86_USB_Single_Serial_5B0A006803-if00 erase-flash
```

## Build Artifacts

After a successful `idf.py build` all binaries are under `build/`:

| File | Size | Flash address |
|---|--:|---|
| `bootloader/bootloader.bin` | ~26 KB | `0x1000` |
| `partition_table/partition-table.bin` | ~3 KB | `0x8000` |
| `ota_data_initial.bin` | 8 KB | `0xf000` |
| `tdisplay_games.bin` (app) | ~242 KB | `0x20000` |

Total flash footprint: ~279 KB out of the 2 MB factory app partition (~14% used).

The `idf.py flash` command writes all four files in a single `esptool` invocation. Individual `.bin` files can also be written manually using the `python -m esptool` command printed at the end of every build.

## Factory Backup

A full 16 MB factory flash backup was written outside the repository:

```text
/tmp/tenstar-t-display-factory-16mb.bin
sha256: 09384a159808cd02e1666be189d9d377f1fcf7fe88ac8a9b0a82d91caf92cb28
```

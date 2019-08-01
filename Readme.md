# ST-Link v3 interfaces

This repository contains code and libraries that can only be used on a Linux platform.

## Compilation

To compile the main example, which just toggles the GPIO, type the following in the directory `example/bridge` :

```bash
g++ ../../src/common/criticalsectionlock.cpp ../../src/common/stlink_interface.cpp ../../src/common/stlink_device.cpp ../../src/bridge/bridge.cpp  -o main -I../../src/bridge/ -I ../../src/common/ -L../../linux_x64/ main_example.cpp -lSTLinkUSBDriver -Wl,-rpath=../../linux_x64/
```

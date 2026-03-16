# STM32N6OTAProgramming

Clone this repo and open STM32N6 and WiFiChip in VSCode. Follow the instructions in their respective READMEs.

## Compilation
Compile both projects as described in the READMEs. Compile the PC-site:
Compile PC_Site:
```bash
cd PC_Site
cmake -B build
cmake --build build
```

## How to run this example
Reset both microcontrollers and run:
```bash
./PC_Site/build/tcp_socket_server
```

Open both microcontrollers' serial output on the console to follow what they do. After the STM32's output is "Writing finished". Reset the STM32N6 and press the user button for 3 seconds, till you see the output of the flashed firmware.

## Explanation
The WiFi-Chips task is to flash a new application to the STM32N6. The script on the PC_site loads the compiled binary and transmits it over WiFi. The WiFi-Chip receives the data and sends it to the STM32N6 via SPI. **Important** The current version is very innefficient. One needs to add some buffering and acknowledging and maybe also checksums to check the integrity of the firmware. 

On the STM32N6-side, the first-stage booloader (FSBL) decides whether to boot Appli or Appli2 through the user's button input. Appli's purpose is to flash the binary of Appli2. It connects to the WiFI-Chip and writes the data it receives from the WiFi chip into the Flash region where Appli2 sits. For this, we need to execute Appli from RAM as we cannot write onto the flash while we read instructions from it. Thus, the FSBl copies the Appli's code from Flash to the RAM before starting it.

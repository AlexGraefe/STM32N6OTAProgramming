#!/bin/bash
set -euo pipefail

run() {
    "$@" || {
        echo ""
        echo -e "\033[0;31mError: Failed to run: $*.\033[0m" >&2
        exit 1
    }
}

run STM32_Programmer_CLI -c port=SWD -d ./FSBL/build/FSBL-trusted.bin 0x70000000 -el /usr/local/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/ExternalLoader/MX66UW1G45G_STM32N6570-DK.stldr
run STM32_Programmer_CLI -c port=SWD -d ./Appli/build/Appli-trusted.bin 0x70100000 -el /usr/local/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/ExternalLoader/MX66UW1G45G_STM32N6570-DK.stldr
# run STM32_Programmer_CLI -c port=SWD -d ./Appli2/build/Appli-trusted.bin 0x70200000 -el /usr/local/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/ExternalLoader/MX66UW1G45G_STM32N6570-DK.stldr
# run STM32_Programmer_CLI -c port=SWD -d Model/network_data.hex -el /usr/local/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/ExternalLoader/MX66UW1G45G_STM32N6570-DK.stldr

echo ""
echo -e "\033[0;32mSuccessfully wrote FSBL and application binaries into memory!\033[0m"

#!/bin/sh

VERSION="1.1"


rm -rf -- "VU_meter_v${VERSION}"
rm -rf -- "VU_meter_v${VERSION}.zip"
mkdir -p -- "VU_meter_v${VERSION}/SH1106/"
mkdir -p -- "VU_meter_v${VERSION}/SSD1306/"

(
    cd ..
    export BUILD=RELEASE
    sed -i 'src/config.h' -e 's/ENABLE_WATERMARK (0)/ENABLE_WATERMARK (1)/'

    for DRIVER in "SSD1306" "SH1106"; do
        export MCU=atmega88p
        export F_CPU=20000000
        make clean
        sed -i 'src/config.h' -e 's/OLED_DRIVER ([^)]\+)/OLED_DRIVER (OLED_DRIVER_'"${DRIVER}"')/'
        make -j8
        cp 'build/RELEASE/main.hex' 'utils/VU_meter_v'"${VERSION}"'/'"${DRIVER}"'/mega88p_20Mhz.hex'

        export MCU=atmega328p
        export F_CPU=20000000
        make clean
        sed -i 'src/config.h' -e 's/OLED_DRIVER ([^)]\+)/OLED_DRIVER (OLED_DRIVER_'"${DRIVER}"')/'
        make -j8
        cp 'build/RELEASE/main.hex' 'utils/VU_meter_v'"${VERSION}"'/'"${DRIVER}"'/mega328p_20Mhz.hex'

        export MCU=atmega88p
        export F_CPU=16000000
        make clean
        sed -i 'src/config.h' -e 's/OLED_DRIVER ([^)]\+)/OLED_DRIVER (OLED_DRIVER_'"${DRIVER}"')/'
        make -j8
        cp 'build/RELEASE/main.hex' 'utils/VU_meter_v'"${VERSION}"'/'"${DRIVER}"'/mega88p_16Mhz.hex'

        export MCU=atmega328p
        export F_CPU=16000000
        make clean
        sed -i 'src/config.h' -e 's/OLED_DRIVER ([^)]\+)/OLED_DRIVER (OLED_DRIVER_'"${DRIVER}"')/'
        make -j8
        cp 'build/RELEASE/main.hex' 'utils/VU_meter_v'"${VERSION}"'/'"${DRIVER}"'/mega328p_16Mhz.hex'

        cp -R 'hardware/' 'utils/VU_meter_v'"${VERSION}"
    done
)

zip -r9 "VU_meter_v${VERSION}.zip" "VU_meter_v${VERSION}"

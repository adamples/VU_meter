#!/bin/sh

VERSION="1.1"


rm -rf -- "VU_meter_v${VERSION}"
rm -rf -- "VU_meter_v${VERSION}.zip"
mkdir -p -- "VU_meter_v${VERSION}/SH1106/"
mkdir -p -- "VU_meter_v${VERSION}/SSD1306/"

(
    cd ..
    BUILD=RELEASE

    for DRIVER in "SSD1306" "SH1106"; do
        BOARD=WZ10
        make clean
        make -j8
        cp 'build/RELEASE/main.hex' 'utils/VU_meter_v'"${VERSION}"'/'"${DRIVER}"'/mega88_168_328_20Mhz.hex'

        BOARD=ARDUINO
        make clean
        make -j8
        cp 'build/RELEASE/main.hex' 'utils/VU_meter_v'"${VERSION}"'/'"${DRIVER}"'/mega88_168_328_16Mhz.hex'

        cp -R 'hardware/' 'utils/VU_meter_v'"${VERSION}"
    done
)

zip -r9 "VU_meter_v${VERSION}.zip" "VU_meter_v${VERSION}"

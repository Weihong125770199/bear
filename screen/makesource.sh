PACKAGE=vendor.mediatek.hardware.screen@1.0
LOC=temp_dir



hidl-gen -o $LOC -Lc++-impl -rvendor.mediatek.hardware:vendor/mediatek/proprietary/hardware/interfaces \
        -randroid.hidl:system/libhidl/transport $PACKAGE
hidl-gen -o $LOC -Landroidbp-impl -rvendor.mediatek.hardware:vendor/mediatek/proprietary/hardware/interfaces \
        -randroid.hidl:system/libhidl/transport $PACKAGE



hidl-gen -o $LOC -Landroidbp -rvendor.mediatek.hardware:vendor/mediatek/proprietary/hardware/interfaces \
        -randroid.hidl:system/libhidl/transport $PACKAGE

hidl-gen -o $LOC -Lhash -rvendor.mediatek.hardware:vendor/mediatek/proprietary/hardware/interfaces \
        -randroid.hidl:system/libhidl/transport $PACKAGE

hidl-gen -o $LOC -Ljava -rvendor.mediatek.hardware:vendor/mediatek/proprietary/hardware/interfaces \
        -randroid.hidl:system/libhidl/transport $PACKAGE

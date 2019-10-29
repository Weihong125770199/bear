echo off


fastboot oem format
if errorlevel  1 goto exit_fail
echo ====start download
fastboot --set-active=b
 if errorlevel  1 goto exit_fail
 
echo ====bootloader.img
fastboot flash bootloader RCAR3_IMAGE\bootloader.img
if errorlevel  1 goto exit_fail 
echo ====dtb.img
fastboot flash dtb RCAR3_IMAGE\dtb.img
if errorlevel  1 goto exit_fail 

echo ====dtbo.img
fastboot flash dtbo RCAR3_IMAGE\dtbo.img
if errorlevel  1 goto exit_fail 

echo ====boot.img
fastboot flash boot RCAR3_IMAGE\boot.img
if errorlevel  1 goto exit_fail 


echo ====system.img
fastboot flash system RCAR3_IMAGE\system.img
if errorlevel  1 goto exit_fail 

echo ====vendor.img
fastboot flash vendor RCAR3_IMAGE\vendor.img
if errorlevel  1 goto exit_fail 

echo ====product.img
fastboot flash product RCAR3_IMAGE\product.img
if errorlevel  1 goto exit_fail 

echo ====userdata
fastboot format -u userdata
if errorlevel  1 goto exit_fail 
echo ====metadata
fastboot erase metadata
if errorlevel  1 goto exit_fail 

:exit_success
echo x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x update successful

exit 0


:exit_fail
echo x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x update fail
exit -1


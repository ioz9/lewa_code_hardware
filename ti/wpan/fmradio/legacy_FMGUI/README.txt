TI FM stack based GUI

legacy_FMGUI folder contains FM  stack based GUI.

To build execute mm at
legacy_FMGUI/FM> mm

and following binaries needs to be copied to image

Install: out/target/product/blaze/system/app/FmRxApp.apk
Install: out/target/product/blaze/system/app/FmTxApp.apk
Install: out/target/product/blaze/system/app/FmService.apk
Install: out/target/product/blaze/system/framework/fmradioif.jar
Install: out/target/product/blaze/system/etc/permissions/com.ti.fm.fmradioif.xml
Install: out/target/product/blaze/system/lib/libfmradio.so

NOTE:
1. Please execute the 'mm' build step on an Android source repository which
has already been built, so that all the build dependecies are met.

2. The above binaries are not build along with android file system by default and needs to be  build seperately in legacy_FMGUI\FM folder by executing mm.

3. If user wants to include in Android file system build he can modify Android.mk of fmradio and can include legacy_FMGUI in Android.mk

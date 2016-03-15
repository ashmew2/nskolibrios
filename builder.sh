rm -rf build-Linux-kolibrios/
PKG_CONFIG_PATH=/usr/local/lib/pkgconfig make TARGET=kolibrios

if [ "$?" -ne "0" ]; then
    echo "The make step failed. Aborting."
    exit 2
fi

cd build-Linux-kolibrios

cp ../kolibrios/*obj .

kos32-ld *.o ~/ofiles/*.o *.obj -T/home/bob/kolibrios/contrib/sdk/sources/newlib/libc/app.lds -nostdlib -static --image-base 0 -L/home/autobuild/tools/win32/mingw32/lib -lgcc /home/autobuild/tools/win32/lib/libz.a /home/autobuild/tools/win32/lib/libdll.a /home/autobuild/tools/win32/lib/libapp.a /home/autobuild/tools/win32/lib/libc.dll.a -o _netsurf -Map=../nsbin.map

if [ "$?" -eq "0" ]; then
    objcopy -O binary _netsurf nsbin
    cp nsbin ../nsbin
    if [ "$?" -eq "0" ]; then
	echo
	echo "nsbin is your executable. Run it on KolibriOS. Enjoy."
    fi
else
    echo "Something went wrong with the linker process. Failed to build executable."
    exit -1
fi

date
ls -l ../nsbin ../nsbin.map

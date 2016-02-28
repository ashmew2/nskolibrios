rm -rf build-Linux-kolibrios/
make TARGET=kolibrios
cd build-Linux-kolibrios

cp ../kolibrios/*obj .

kos32-ld *.o ~/ofiles/*.o *.obj -T/home/bob/kolibrios/contrib/sdk/sources/newlib/libc/app.lds -nostdlib -L/home/autobuild/tools/win32/mingw32/lib -lgcc /home/autobuild/tools/win32/lib/libz.a /home/autobuild/tools/win32/lib/libdll.a /home/autobuild/tools/win32/lib/libapp.a /home/autobuild/tools/win32/lib/libc.dll.a -o netsurfe

echo "Ignore the errors above (for now...)"
echo "_netsurf is your executable. Enjoy."

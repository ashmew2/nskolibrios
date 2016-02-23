rm -rf build-Linux-kolibrios/
make TARGET=kolibrios
cd build-Linux-kolibrios

cp ../kolibrios/*obj .
kos32-ld -L/home/autobuild/tools/win32/mingw32/lib *.o ~/ofiles/*.o *.obj -lz -dll -lapp -lc -lgcc -o ../_netsurf

echo "Ignore the errors above (for now...)"
echo "_netsurf is your executable. Enjoy."

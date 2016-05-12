if [ "$1" == "" ]; then
	exit
fi

cd ../$1
rm -rf build-*
GCCSDK_INSTALL_ENV=/home/bob/kolibrios/contrib/sdk/sources/ CXX__=kos32-g++ CC=kos32-gcc AR__=kos32-ar HOST=kolibrios make install
cd ../netsurf

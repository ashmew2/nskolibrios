OFILES=/home/ashish/ofiles
WORKSPACE=/home/ashish/dev-netsurf/workspace/

# for libname in libwapcaplet libparserutils libnsgif libnsbmp
# do
#     cd ${WORKSPACE}/${libname}/src
#     echo
#     echo
#     echo "Inside directory: $(pwd)"
#     PWD=/home/ashish/ofiles/${libname}-
#     cfilelist=$(find . -regex '.*.c$')
# #    echo ${cfilelist}

#      for cfile in ${cfilelist}; do
#  	ofile=${cfile#./}
#  	ofile=${ofile%.c}
#  	ofile=${ofile//\//-}.o
#  	echo "Building ${cfile} to ${PWD}${ofile}"
# 	~/i586-kos32/kos32/bin/i586-kos32-gcc -c ${cfile} -I../include/ -I./ -I/home/ashish/i586-kos32/kos32/i586-kos32/include/ -o ${PWD}${ofile}
#      done
# done

# for libname in libhubbub 
# do
#     cd ${WORKSPACE}/${libname}/src
#     echo
#     echo
#     echo "Inside directory: $(pwd)"
#     PWD=/home/ashish/ofiles/${libname}-
#     cfilelist=$(find . -regex '.*.c$')
# #    echo ${cfilelist}

#      for cfile in ${cfilelist}; do
#  	ofile=${cfile#./}
#  	ofile=${ofile%.c}
#  	ofile=${ofile//\//-}.o
#  	echo "Building ${cfile} to ${PWD}${ofile}"
# 	~/i586-kos32/kos32/bin/i586-kos32-gcc -c ${cfile} -I${WORKSPACE}/libparserutils/include/ -I../include/ -I./ -I/home/ashish/i586-kos32/kos32/i586-kos32/include/ -o ${PWD}${ofile}
#      done
# done

# for libname in libdom libcss
# do
#     cd ${WORKSPACE}/${libname}/src
#     echo
#     echo
#     echo "Inside directory: $(pwd)"
#     PWD=/home/ashish/ofiles/${libname}-
#     cfilelist=$(find . -regex '.*.c$')
# #    echo ${cfilelist}

#      for cfile in ${cfilelist}; do
#  	ofile=${cfile#./}
#  	ofile=${ofile%.c}
#  	ofile=${ofile//\//-}.o
#  	echo "Building ${cfile} to ${PWD}${ofile}"
# 	~/i586-kos32/kos32/bin/i586-kos32-gcc -c ${cfile} -I${WORKSPACE}/libwapcaplet/include -I${WORKSPACE}/libparserutils/include -I../include/ -I./ -I/home/ashish/i586-kos32/kos32/i586-kos32/include/ -o ${PWD}${ofile}
#      done
# done

# for libname in libutf8proc
# do
#     cd ${WORKSPACE}/${libname}/src
#     echo
#     echo
#     echo "Inside directory: $(pwd)"
#     PWD=/home/ashish/ofiles/${libname}-
#     cfilelist=utf8proc.c
# #    echo ${cfilelist}

#      for cfile in ${cfilelist}; do
#  	ofile=${cfile#./}
#  	ofile=${ofile%.c}
#  	ofile=${ofile//\//-}.o
#  	echo "Building ${cfile} to ${PWD}${ofile}"
# 	~/i586-kos32/kos32/bin/i586-kos32-gcc -c ${cfile} -I../include/libutf8proc/ -I./ -I/home/ashish/i586-kos32/kos32/i586-kos32/include/ -o ${PWD}${ofile}
#      done
# done

# for libname in libnsutils
# do
#     cd ${WORKSPACE}/${libname}/src
#     echo
#     echo
#     echo "Inside directory: $(pwd)"
#     PWD=/home/ashish/ofiles/${libname}-
#     cfilelist=$(find . -regex '.*.c$')
# #    echo ${cfilelist}

#      for cfile in ${cfilelist}; do
#  	ofile=${cfile#./}
#  	ofile=${ofile%.c}
#  	ofile=${ofile//\//-}.o
#  	echo "Building ${cfile} to ${PWD}${ofile}"
# 	~/i586-kos32/kos32/bin/i586-kos32-gcc -c ${cfile} -I../include/ -I./ -I/home/ashish/i586-kos32/kos32/i586-kos32/include/ -o ${PWD}${ofile}
#      done
# done

for libname in libdom
do
    cd ${WORKSPACE}/${libname}/bindings/hubbub
    echo
    echo
    echo "Inside directory: $(pwd)"
    PWD=/home/ashish/ofiles/${libname}-bindings-
    cfilelist=parser.c
#    echo ${cfilelist}

     for cfile in ${cfilelist}; do
 	ofile=${cfile#./}
 	ofile=${ofile%.c}
 	ofile=${ofile//\//-}.o
 	echo "Building ${cfile} to ${PWD}${ofile}"
	~/i586-kos32/kos32/bin/i586-kos32-gcc -c ${cfile} -I../../../libhubbub/include/ -I../../../libdom/include/ -I../../../libdom/src/ -I../../../libwapcaplet/include/ -I/home/ashish/i586-kos32/kos32/i586-kos32/include/ -o ${PWD}${ofile}
     done
done

#!/bin/sh 

TEST_PATH=$1
TEST_OUT=${TEST_PATH}/svg

mkdir -p ${TEST_OUT}

svgdecode()
{
    OUTF=$(basename ${1} .svg)
    ${TEST_PATH}/test_decode_svg ${1} > ${TEST_OUT}/${OUTF}.svg
}


for SVG in $(ls test/data/*.svg);do
    svgdecode ${SVG}
done


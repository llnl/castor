#!/bin/bash

SRC_DIR=.
BUILD_DIR=./build
OUT_DIR=./out

rm -r ${BUILD_DIR}

echo "builds going into ${BUILD_DIR}"
echo ${BUILD_DIR}

# brew install coreutils

# initialize cmake
cmake -S "${SRC_DIR}" -B "${BUILD_DIR}"

# build everything using cmake
cmake --build "${BUILD_DIR}" -j$(nproc)

# run ctest on build
ctest --test-dir "${BUILD_DIR}" --parallel $(nproc)

# move static library and headers into output directory
cmake --install ${BUILD_DIR} --prefix ${OUT_DIR}

# create a sample executable that can be used
cp ${BUILD_DIR}/test/Test20 ${OUT_DIR}/goose
#!/bin/sh

PROJECT_HOME=$(pwd -P)
MXE_HOME=$(cd ${1}; pwd -P)

(test -d ${MXE_HOME}/usr/bin) || { echo "Invalid MXE_HOME: ${1}"; exit 1; }
shift

if [ "${1}" = "w32" ]; then
    HOST="i686-w64-mingw32"
elif [ "${1}" = "w64" ]; then
    HOST="x86_64-w64-mingw32"
else
    echo "Unknown target: ${1}. Must be one of: w32|w64";
    exit 1;
fi
shift
CONFLAGS="${1}"

echo "PROJECT_HOME=${PROJECT_HOME}"
echo "MXE_HOME=${MXE_HOME}"
echo "HOST=${HOST}"

PREFIX=${PROJECT_HOME}/${HOST}
export PATH=${MXE_HOME}/usr/bin:${MXE_HOME}/usr/${HOST}/bin:${PATH};
unset CC CFLAGS CPPFLAGS LDFLAGS LD_LIBRARY_PATH

set -x
${MXE_HOME}/usr/${HOST}/qt/bin/qmake -spec win32-x-g++ && make -j3

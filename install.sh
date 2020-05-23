#!/bin/bash

set -eu

IPATH=$RUMI_PATH

if [ -z "$IPATH" ]
then
    echo "set RUMI_PATH"
    exit 1
fi

mkdir -p "$IPATH/bin/"

make rum

./compile.sh

cp src/rum "$IPATH/bin/"

#cp -r rumlib/* "$IPATH"

echo "Done!"

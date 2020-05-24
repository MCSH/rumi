#!/bin/bash

set -eu

IPATH=$RUMI_PATH

if [ -z "$IPATH" ]
then
    echo "set RUMI_PATH"
    exit 1
fi

mkdir -p "$IPATH/bin/"

./compile.sh

cp src/rum2 "$IPATH/bin/rum"

#cp -r rumlib/* "$IPATH"

echo "Done!"

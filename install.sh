#!/bin/bash

IPATH=$RUMI_PATH

if [ -z "$IPATH" ]
then
    echo "set RUMI_PATH"
    exit 1
fi

mkdir -p "$IPATH/bin/"

make rum

cp rum "$IPATH/bin/"

#cp -r rumlib/* "$IPATH"

echo "Done!"

#!/bin/bash

IPATH=$RUMI_PATH

if [ -z "$IPATH" ]
then
    echo "set RUMI_PATH"
    exit 1
fi

mkdir -p "$IPATH/bin/"

make rum
make rumi

cp rum rumi "$IPATH/bin/"

cp -r rumlib/* "$IPATH"

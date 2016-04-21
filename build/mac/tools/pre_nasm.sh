#!/bin/bash

xcode_dev_path=`xcode-select --print-path`
cp -f nasm $xcode_dev_path/usr/bin
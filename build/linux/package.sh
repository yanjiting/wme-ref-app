#!/usr/bin/env bash

get_abs() 
{
    eval "in_value=\$$1"
    abs_path=`cd "$in_value"; pwd`
    eval "$1=$abs_path"
}

DIR="`pwd`/`dirname $0`/"
get_abs DIR
ROOT="$DIR/../../"
get_abs ROOT

target="`pwd`/wme4linux.tar"
listOfTrainHeaders=("api" 
                    "mediaengine/util/include"
                    "mediaengine/tp/include"
                    "distribution")

rm -f wme4linux.tar
rm -f wme4linux.tar.*
tar cfT $target /dev/null
cd $ROOT
for headerPath in "${listOfTrainHeaders[@]}"
do
    tar uf $target "$headerPath"
done

gzip $target


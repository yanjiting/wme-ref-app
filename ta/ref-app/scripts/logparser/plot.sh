#!/bin/bash

#BEAVER_PLUGINS=${PWD}
#export BEAVER_PLUGINS

echo $BEAVER_PLUGINS

if [ $# -lt 1 ]; then
    echo "Usage: plot.sh logfile [regex-filter] [filter-tag]"
    exit 1
fi

REGEX=""
if [ $# -gt 1 ]; then
    REGEX=$2
fi

FILTER_TAG="filter"
if [ $# -eq 3 ]; then
    FILTER_TAG=$3
fi

generate_plot() {
    # $1 : logpath
    abspath="$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
    dirpath=$(dirname "$abspath")
    pngname=$(basename "$abspath" .log)
    pngpath=$dirpath"/"$pngname".png"
    litepngpath=$dirpath"/"$pngname".lite.png"
    # fixme, the awk below attempts to filter those logs of client, however this is not true
    # we need enhance beaver framework to do first level matching before matching by re
    #awk '/[0-9]+\.?[0-9]*s 0|1|2|\-1/' "$1" > /tmp/qos_client.log;extract.py /tmp/qos_client.log
    cat "$1" > /tmp/qos_client.log;extract.py /tmp/qos_client.log
    
    if [ "$REGEX" == "" ]; then
        plot.py -i /tmp/qos_client.dump --view view_qos_adjustment --save $dirpath"/QoS_"$pngname".png" --sub "$pngname"
        plot.py -i /tmp/qos_client.dump --view view_video_qos --save $dirpath"/VideoQoS_"$pngname".png" --sub "$pngname"
        plot.py -i /tmp/qos_client.dump --view view_qos_receiver --save $dirpath"/QoSRecv_"$pngname".png" --sub "$pngname"
        #plot.py -i /tmp/qos_client.dump --view view_qos_adjustment --save "$litepngpath" --sub "$pngname"
    else
        filterpngpath=$dirpath"/"$pngname"."$FILTER_TAG".png"
        grep -E "$REGEX" /tmp/qos_client.dump | plot.py -i - --view view_qos_adjustment --save "$filterpngpath" --sub "$pngname"
    fi
}

find "$1" -name '*.log' -print0 | while IFS= read -r -d '' f; do
    echo "processing \"$f\""
    generate_plot "$f"
done


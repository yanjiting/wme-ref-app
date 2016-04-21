#!/bin/bash

# set it to "" if need reinstall
INSTALL_OPT="NOINSTALL=1"

realpath() {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

SCRIPT_PATH=$(realpath "$0")
ORIGIN_DIR=$(dirname $SCRIPT_PATH)

log_dir="$ORIGIN_DIR/trace"

remove_last_log_dir()
{
    last_dir=`ls -rt $log_dir | tail -n1`
    last_dir_path="$log_dir/$last_dir"
    echo "removing dir: $last_dir_path"
    rm -rf $last_dir_path
}

rename_last_log_dir()
{
    last_dir=`ls -rt $log_dir | tail -n1`
    last_dir_path="$log_dir/$last_dir"
    echo "rename dir: $last_dir_path"
    mv $last_dir_path "${last_dir_path}_fail"
}

run_mp2p_qos()
{
    cucumber ta_features -t @p2p -t @wifi -t @contention -t @c4 -t @qos-enable ${INSTALL_OPT}
    if [ $? -eq 0 ]; then
        # remove it when success
        remove_last_log_dir
    else
        # run qos disable immediately in case of the network problem still exist
        echo "----->>>>>qos enable failed, run qos disable immediately..."
        cucumber ta_features -t @p2p -t @wifi -t @contention -t @c4 -t @qos-disable ${INSTALL_OPT}
    fi
}

run_concur_p2p()
{
    cucumber ta_features -t @concur ${INSTALL_OPT}
    if [ $? -ne 0 ]; then
        # rename failed cases
        rename_last_log_dir
    fi
}

while true; do
    #run_mp2p_qos
    run_concur_p2p
    echo "Sleep 60 seconds then repeat"
    sleep 60
done

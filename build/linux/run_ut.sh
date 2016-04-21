#!/bin/bash

# save UT status info and result
ut_status_info=""
ut_status_result=0

checkUTStatus() {
 	echo "********************************************"
 	echo ""
 	echo -e $ut_status_info
 	echo "********************************************"
 	if [ $ut_status_result -ne 0 ]; then
 		echo "Error: found some failed status"
 		exit 2
 	fi
    return $ut_status_result
}

recordUTStatus() {
	"$@"
	status=$?
    if [ $status -ne 0 ]; then
        echo "Error with command: $1"
		ut_status_info=${ut_status_info}"[Fail]: $@\n"
		ut_status_result=1
	else
		ut_status_info=${ut_status_info}"[Success]: $@\n"
    fi
    return $status
}

copywbtlogs()
{
   wbtfiles=`ls -1rt /tmp/*.wbt`
   wbtfiles=`echo $wbtfiles | awk '{print $(NF)" "$(NF-1)}'`
   mkdir -p "$1"/wbxlogs/$2
   for x in $wbtfiles; do
     cp $x "$1"/wbxlogs/$2
   done
}

recordUTStatus ../../unittest/tp/bld/linux/bin/unittest_tp  tpTest_linux.xml
recordUTStatus ../../unittest/util/bld/linux/bin/unittest_util utilTest_linux.xml

checkUTStatus

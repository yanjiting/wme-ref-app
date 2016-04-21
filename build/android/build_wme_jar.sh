#!/bin/bash
#

ROOT=`pwd`/../..
OPENH264=$ROOT/vendor/openh264/libs/android
DIST=$ROOT/distribution/android/armv7

echox() { color=$1; shift; [ `uname` = 'Linux' ] && echo -e "\033[${color}m$*\033[00m" || echo "$*"; }

if [ "$1" = "debug" ]; then
    configuration=debug
elif [ "$1" = "release" ]; then
    configuration=release
else
    echo "usage: $0 debug|release" && exit 0
fi
mkdir -p $DIST/$configuration

##================================================================
## for openh264 libs
cp -f $OPENH264/lib*.so $DIST/$configuration/


##================================================================
#
# Pls define the following variables by correspoding shell VARIBLEs:
# (*) sdk:		android sdk dir
# (*) target:   android target platform
# (*) src:   	java source dir
# (*) build: 	object dest dir
# (*) extra:    extra depended jar
#

ANT=`which ant`
[ "$ANT" = "" ] && echox 31 "Please install ant" && exit 1

#
# set android target platform and sdk dir
[ "$ANDROID_HOME" = "" ] && echox 31 "[WARN] Please set ANDROID_HOME" && exit 1
SDK_DIR="$ANDROID_HOME"

MEDIASESSION=$ROOT/mediasession
TARGET=android-19

#
# set target object name
OBJ_NAME=wmeclient 

#
# set java source dir and dest dir, NOTE: should be absolute dir
SRC_DIR=$MEDIASESSION/src/java/src

CUR_SEC=`date +%s`
DST_DIR=/tmp/build-$CUR_SEC
BUILD_XML=/tmp/build-jar-$CUR_SEC.xml

#
# set extra depended jar, NOTE: should be absolute dir
EXTRA_DIR=$MEDIASESSION

#
# generate build-jar.xml for ant
cat > $BUILD_XML <<EOF
<project name="${OBJ_NAME}" basedir="." default="main">
	<property name="src.dir"     value="\${src}"/>
	<property name="build.dir"   value="\${build}"/>
	<property name="android.dir"  value="\${sdk}/platforms/\${target}/" />  
	<property name="extra.dir"  value="\${extra}"/>

	<property name="classes.dir" value="\${build.dir}/classes"/>
	<property name="jar.dir"     value="\${build.dir}/jar"/>

	<target name="clean">
		<delete dir="\${build.dir}"/>
	</target>

	<target name="compile">
		<mkdir dir="\${classes.dir}"/>
		<javac srcdir="\${src.dir}" destdir="\${classes.dir}" includeantruntime="false" target="1.6">
			<classpath>
				<fileset dir="\${android.dir}" includes="*.jar" />
				<fileset dir="\${extra.dir}" includes="*.jar" />
			</classpath>
		</javac>
	</target>

	<target name="jar" depends="compile">
		<mkdir dir="\${jar.dir}"/>
		<jar destfile="\${jar.dir}/\${ant.project.name}.jar" basedir="\${classes.dir}"/>
	</target>

	<target name="main" depends="clean,jar"/>
</project>
EOF


#
# run ant
"$ANT" -Dsdk=$SDK_DIR -Dtarget=$TARGET \
	-Dsrc=$SRC_DIR -Dbuild=$DST_DIR \
	-Dextra=$EXTRA_DIR \
	-f $BUILD_XML

echo $DST_DIR/jar/wmelient.jar
if [ -e $DST_DIR/jar/wmeclient.jar ]; then
    cp $DST_DIR/jar/wmeclient.jar $DIST/$configuration/
    rm -rf $DST_DIR $BUILD_XML
else
    rm -rf $DST_DIR $BUILD_XML
    echo "Error for building wmelient.jar"
    exit 1
fi

exit 0

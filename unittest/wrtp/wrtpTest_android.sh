cd ./jni/jni
ndk-build -B
if [ $errorlevel -neq  0 ]
then
echo build $1 failed
fi

cd ./../../
ndk-build -B
if [ $errorlevel -neq  0 ]
then
echo build $1 failed
fi
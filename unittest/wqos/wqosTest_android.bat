cd .\jni\jni
call ndk-build -B
if %errorlevel% NEQ 0 (
 	echo build failed
    pause 
) else (
    cd .\..\..\
    call ndk-build -B
    pause 
)

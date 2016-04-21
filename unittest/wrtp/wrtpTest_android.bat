::wrtpTest_android.bat [-t] [-q] :"-t" means build test only, "-q" means quit script when build finished
cd .\jni\jni   
@if /I "%1" equ "-t" @(
    goto test
) 
@if /I "%2" equ "-t" @(
    goto test
) 

@echo build all
call ndk-build -B
if %errorlevel% NEQ 0 (
    echo modules build failed
    goto end
) 
    
:test
@echo build unittest
cd .\..\..\
call ndk-build -B
if %errorlevel% NEQ 0 (
    echo test build failed
    goto end
) 

:end

@if /I "%1" neq "-q" @(
    @if /I "%2" neq "-q" @(
        @pause
    ) 
) 

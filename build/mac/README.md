#Build 
     sh build.sh
     
#Build and Run Unit Test 
    sh build_ut.sh
    
#Build Problems

### nasm error 
* check the nasm version 

        which nasm
        nasm -v
    
* make sure /usr/bin/nasm or /opt/local/bin/nasm exist 
    
        
        ls /usr/bin/nasm
        ls /opt/local/bin/nasm
        
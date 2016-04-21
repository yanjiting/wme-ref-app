#!/bin/bash
serverUser="wme"
serverIP="10.224.76.84"
echo "Please enter the return directly!" 
ssh-keygen -t rsa
#chmod 755 ~/.ssh
authkey=`cat ~/.ssh/id_rsa.pub`
echo "add public key to the server "$serverUser"@"$serverIP
ssh $serverUser@$serverIP "echo $authkey>>~/.ssh/authorized_keys"

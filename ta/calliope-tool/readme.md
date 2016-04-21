> prepare environment

    Install CentOS 6.x 64bit
    yum install -y git gcc gcc-c++ python-devel

	
> checkout the source code

    git clone git@sqbu-github.cisco.com:Calliope/calliope-vagrant.git
    copy bootstrap.py calliope-vagrant

	
> Start to install dependencies:

    cd calliope-vagrant
    sh FIRST_TIME_SETUP.sh 
    python bootstrap.py

	
> Upgrade Linus and run it

   python checkupdate.py

> How to verify

	http://<Linus IP>:5000/calliope/api/v1/ping   

> Patches (Now linus changed a lot, we need to pacth the script, i dont have time to make them script either)

    yum remove monit
    scp 10.224.166.110:/usr/bin/monit /usr/bin/
    scp 10.224.166.110:/etc/monit.conf /etc
    mkdir -p /var/monit

    yum install -y boost_1.57.0_cisco1.x86_64

    Add:

    register_to_registrar_enabled : "False"

    to /opt/calliope/etc/media_agent.yaml

    monit stop linus
    monit start linus

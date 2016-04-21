import os,sys,shutil
import errno
import socket

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise

def get_local_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("gmail.com",80))
    ip = (s.getsockname()[0])
    s.close()
    return ip

def linus_upload_config(config_file_name="configs/media_agent.yaml"):
    mkdir_p('/opt/calliope/etc')
    config_file = "/opt/calliope/etc/media_agent.yaml"
    shutil.copy2(config_file_name, config_file)
    os.system("sed -i -e 's/192.168.34.12/%s/g' %s" % (get_local_ip(), config_file))
    os.system("sed -i -e '/agent_log_filename/a authenticate_incoming_requests : Off' %s" % config_file)

def linus_install_monit_config(version=1):
    filename = linus_create_monit_config(version)
    etc_monit = '/etc/monit.d' 
    mkdir_p(etc_monit)
    shutil.copy2(filename, etc_monit)
    os.remove(filename)

def linus_create_monit_config(version=1):
    linus_cfg_file_template='configs/linus_monit.cfg.template'
    linus_cfg_file='linus_monit.cfg'
    with open(linus_cfg_file_template, 'r') as fin:
        with open(linus_cfg_file, 'w') as fout:
            for line in fin.readlines():
                fout.write(line.replace('VERSION', str(version)))
    return linus_cfg_file

def copy_puppet_modules():
    shutil.copytree('modules', '/tmp/modules')
    shutil.copytree('manifests', '/tmp/manifests')

def run_puppet_modules(manifest_file="nodes.pp"):
    savedPath = os.getcwd()
    os.chdir('/tmp')
    os.system('puppet apply manifests/%s --modulepath modules'%manifest_file)
    os.chdir(savedPath)

def prepare_environment():
    os.system('cp configs/cisco-wx2-repo.repo /etc/yum.repos.d/')
    os.system('yum install -y monit')
    os.system('yum install -y puppet')
    os.system('yum install -y gcc')
    os.system('service iptables stop')
    os.system('chkconfig iptables off')
    #os.system('service monit start')

prepare_environment()
linus_install_monit_config(722)
linus_upload_config("configs/media_agent_standalone.yaml")
copy_puppet_modules()
run_puppet_modules("linus.pp")

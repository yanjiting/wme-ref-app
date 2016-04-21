#!/usr/bin/python
import sys
import os
import commands
import subprocess
temp_path = './temp/'
temp_data_path = './temp/data/'
temp_info_path = './temp/info/'
temp_html_path = './temp/html/'
temp_xml_path = './temp/xml/'

wme_path = '../../../'
mediaengine_path = '../../../mediaengine/'
wbxtrace_path = '../../../mediaengine/wbxtrace/objs/'
unittest_distribution = '../../../unittest/distribution'
current_path = os.getcwd()
#make sure the output folder is cc_workspace


#################################################################################################################


def init_mac_setting(run_mode, build_mode):
    global cc_workspace, module_list
    print 'init_mac_setting %s %s' %(run_mode, build_mode)
    #global setting
    cc_workspace = '/tmp/cc/'
    # shark ,  "wsevp.build"
    #"Wbx_aagc.build", "libg711.build", "libg722.build", "libopus101.build", "libwebrtcilbc.build", "libwebrtcspl.build", "wbx_aec.build", "wbx_cng.build", "wbx_dagc.build", "wbx_resample.build", "wbx_sa.build", "wbx_tsm.build", "wbx_vad.build", "wbxaecodec.build", dolphin
    module_list = [
        {"module_name":"util_module", 	    "src_root_path":current_path+"/../../../mediaengine/util",		     "projects":[cc_workspace+"util.build"],},
        {"module_name":"tp_module", 	    "src_root_path":current_path+"/../../../mediaengine/tp",             "projects":[cc_workspace+"tp.build"],},
        {"module_name":"wqos_module",   	"src_root_path":current_path+"/../../../mediaengine/wqos", 		     "projects":[cc_workspace+"wqos.build"],},
        {"module_name":"wrtp_module",    	"src_root_path":current_path+"/../../../mediaengine/transmission", 	 "projects":[cc_workspace+"srtp.build", cc_workspace+"wrtp.build"],},
        {"module_name":"wme_module", 	    "src_root_path":current_path+"/../../../mediaengine/wme",	   	     "projects":[cc_workspace+"wmeclient.build"],},
        {"module_name":"shark_module",   	"src_root_path":current_path+"/../../../mediaengine/shark", 	     "projects":[cc_workspace+"wseclient.build", cc_workspace+"wsertp.build",cc_workspace+"wmeutil.build"],},
        {"module_name":"dolphin_module",    "src_root_path":current_path+"/../../../mediaengine/dolphin",   	 "projects":[cc_workspace+"wbxaudioengine.build"],},
	    {"module_name":"appshare_module",   "src_root_path":current_path+"/../../../mediaengine/appshare",   	 "projects":[cc_workspace+"appshare.build"],},	
	    {"module_name":"mediasession_module",   "src_root_path":current_path+"/../../../mediasession",   	         "projects":[cc_workspace+"MediaSession.build"],},			
    ]
    if build_mode == 'debug':
        configString = '/Debug'
    else:
        configString = '/Release' 
    
    for i in range(len(module_list)):
        for j in range(len(module_list[i]['projects'])):
            module_list[i]['projects'][j] = module_list[i]['projects'][j] + configString                

#################################################################################################################
#run_mode = sim|dev, build_mode = debug|release
def init_ios_setting(run_mode, build_mode):
    global cc_workspace, module_list
    print 'init_ios_setting %s %s' %(run_mode, build_mode)
    
    #global setting
    cc_workspace = '/tmp/cc_iOS/'
    # shark ,  "wsevp.build"
    # wbx_aagc.build,wbx_aecm.build,wbx_cng.build,wbx_dagc.build,wbx_plc.build,wbx_resample.build,wbx_sa.build,wbx_tsm.build,wbx_vad.build,wbxaecodec.build =>dolphin
# iLbc_webrtc.build
# libg711.build
# libg722.build
# libopus101.build
# pp20aec.build
# pp20cmod.build
# pp20util.build

    module_list = [
        {"module_name":"util_module", 	    "src_root_path":current_path+"/../../../mediaengine/util",		     "projects":[cc_workspace+"util_ios.build"],},
        {"module_name":"tp_module", 	    "src_root_path":current_path+"/../../../mediaengine/tp",             "projects":[cc_workspace+"tp_ios.build"],},
        {"module_name":"wqos_module",   	"src_root_path":current_path+"/../../../mediaengine/wqos", 		     "projects":[cc_workspace+"wqos_ios.build"],},
        {"module_name":"wrtp_module",    	"src_root_path":current_path+"/../../../mediaengine/transmission", 	 "projects":[cc_workspace+"srtp.build"],},
        {"module_name":"wrtp_module",    	"src_root_path":current_path+"/../../../mediaengine/transmission", 	 "projects":[cc_workspace+"wrtp_ios.build"],},        
        {"module_name":"wme_module", 	    "src_root_path":current_path+"/../../../mediaengine/wme",	   	     "projects":[cc_workspace+"wmeclient.build"],},
        {"module_name":"shark_module",   	"src_root_path":current_path+"/../../../mediaengine/shark", 	     "projects":[cc_workspace+"wseclient.build", cc_workspace+"wsertp.build", cc_workspace+"wmeutil.build"],},
        {"module_name":"dolphin_module",    "src_root_path":current_path+"/../../../mediaengine/dolphin",   	 "projects":[cc_workspace+"wbxaudioengine.build"],},
	    {"module_name":"appshare_module",   "src_root_path":current_path+"/../../../mediaengine/appshare",   	 "projects":[cc_workspace+"appshare.build"],},	
	    #{"module_name":"wp2p_module",       "src_root_path":current_path+"/../../../mediaengine/wp2p",   	     "projects":[cc_workspace+"wp2p_ios.build"],},		    
	    {"module_name":"wbxtrace_module",   "src_root_path":current_path+"/../../../mediaengine/wbxtrace",   	 "projects":[cc_workspace+"wbxtrace.build"],},		    	    
	    {"module_name":"mediasession_module",   "src_root_path":current_path+"/../../../mediasession",   	         "projects":[cc_workspace+"MediaSession.build"],},			
    ] 
       
    if run_mode == 'dev':
        platformString = 'iphoneos/'
    else:
        platformString = 'iphonesimulator/'
            
    if build_mode == 'debug':
        configString = '/Debug'
    else:
        configString = '/Release'    
    for i in range(len(module_list)):
        for j in range(len(module_list[i]['projects'])):
            module_list[i]['projects'][j] = module_list[i]['projects'][j]+configString+'-'+platformString


#run_mode = sim|dev, build_mode = debug|release
def init_android_setting(run_mode, build_mode):
    global cc_workspace, module_list
    print 'init_android_setting %s %s' %(run_mode, build_mode)
    
    #global setting
    cc_workspace = '/'

    module_list = [
        {"module_name":"util", 	       "path":"../../../mediaengine/util"         },
        {"module_name":"tp", 	       "path":"../../../mediaengine/transmission" },
        {"module_name":"wqos", 	       "path":"../../../mediaengine/wqos" 		  },
        {"module_name":"wrtp", 	       "path":"../../../mediaengine/wrtp" 		  },
        {"module_name":"wme", 	       "path":"../../../mediaengine/wme" 		  },
        {"module_name":"shark", 	   "path":"../../../mediaengine/shark" 	      },
        {"module_name":"dolphin", 	   "path":"../../../mediaengine/dolphin" 	  },
        {'module_name':'mediasession', "path":"../../../mediasession"             },        
    ]



if __name__ == "__main__":
    args = sys.argv[1:]
    if len(args) < 2:
        print "Usage: ./setting [sim|dev] [debug|release]"
        sys.exit()
    run_mode = args[0]
    build_mode = args[1]
    #init_ios_setting(run_mode, build_mode);

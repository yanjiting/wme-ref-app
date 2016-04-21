import os
import sys
import setting
import commands
import shutil
import clean

temp_info_path = setting.temp_info_path

class Lcov(object):
    def prepare_gcov_file(self, src_path_list, gcno_path_list, gcda_path_list,):
        if 0 == len(src_path_list) or 0 == len(gcno_path_list) or 0 == len(gcda_path_list):
             print "Invalid input parameters"
             sys.exit(1)

        for n in range(len(src_path_list)):
            for f in os.listdir(src_path_list[n]):
                if f.lower().split('.')[-1] == 'cpp':
                    gcnofilename = f[:-3]+'gcno'
                    gcdafilename = f[:-3]+'gcda'

                    if os.path.exists(gcda_path_list[n]) and (gcdafilename in os.listdir(gcda_path_list[n])):
                        try:
                            shutil.copy(os.path.join(gcda_path_list[n], gcdafilename), src_path_list[n])
                        except Exception, e:
                            print e
                        if os.path.exists(gcno_path_list[n]) and (gcnofilename in os.listdir(gcno_path_list[n])):
                            try:
                                shutil.copy(os.path.join(gcno_path_list[n], gcnofilename), src_path_list[n])
                            except Exception, e:
                                print e

    def gen_info(self, platform,  input_folder_list, info_filename):
        '''
        Generate info files from gcda and gcno files. And output folder is temp_info_path.

        :param input_folder_list: A list of Path of gcno, gcda and src files.
        :type input_folder_list: [string]
        :param info_filename: Name of info file.
        :type info_filename: string
        '''
        gcov_tool = ''
        lcov_com = ''
        if platform == 'android' :
            gcov_dir = os.environ.get('ANDROID_NDK_HOME')
	    gcov_tool = gcov_dir + '/toolchains/x86-4.8/prebuilt/darwin-x86_64/bin/i686-linux-android-gcov '
            lcov_cmd = 'lcov  -c  --gcov-tool %s' % (gcov_tool)
        else:
            lcov_cmd = 'lcov -c --rc lcov_branch_coverage=1 '
            
        for input_folder in input_folder_list:
            lcov_cmd = lcov_cmd + '-d ' + input_folder + ' '
        lcov_cmd += '--output-file ' + os.path.join(temp_info_path, info_filename) + '.info'
        print 'lcov commamd:', lcov_cmd
        return lcov_cmd

    def merge_info(self, input_folder, output_folder, info_filename):
        '''
        Merge info files from temp_info_path to output_folder.

        :param input_folder: Path of input info file.
        :type input_folder: string
        :param output_folder: Path of output info file.
        :type output_folder: string
        :param info_filename: Name of info file.
        :type info_filename: string
        '''
        merge_cmd = 'lcov -q --rc lcov_branch_coverage=1 '
        for f in os.listdir(input_folder):
            filename = os.path.join(input_folder, f)
            if os.path.isfile(filename):
                if f.lower().split('.')[-1] == 'info':
                    merge_cmd = merge_cmd + '-a ' + filename + ' '
        merge_cmd = merge_cmd + '-o ' + os.path.join(output_folder, info_filename) + '.info'
        print merge_cmd
        return merge_cmd

    def remove_useless_infos(self, output_folder, info_filename):
        '''
        Remove Useless info from old info file.
        '''
        file_name = os.path.join(output_folder, info_filename) + '.info'
        remove_cmd = 'lcov --remove %s *boost* */Applications* *vendor* *include* *api* -o %s' %(file_name,file_name)
        print remove_cmd
        return remove_cmd

    def execute(self, cmd):
        '''
        Execute command.
        '''
        print commands.getstatusoutput(cmd)[1]

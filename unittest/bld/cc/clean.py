import os
import shutil


class Cleaner(object):
    '''
    Clean gcda, gcno and temporary info files from environment.
    '''
    
    def clean_obj(self, obj_path):
        if os.path.exists(obj_path):
            shutil.rmtree(obj_path)    
        
    def clean_obj_list(self, obj_path_list):
        if len(obj_path_list) != 0:
            for obj_path in obj_path_list :
                self.clean_obj(obj_path)
                    
    def clean_src(self, src_path):
        for root,dirs,files in os.walk(src_path):
            for name in files:
                pathname = os.path.splitext(os.path.join(root,name))
                #print "clean", pathname
                if (pathname[-1] == ".gcda" or pathname[-1] == ".gcno"):
                    os.remove(os.path.join(root,name))
        
    def clean_src_list(self, src_path_list):
        #print src_path_list
        if len(src_path_list) != 0:
            for src_path in src_path_list :
                #print "clean", src_path
                self.clean_src(src_path)
    
    def clean_path(self, path):
        if os.path.exists(path):
            shutil.rmtree(path)
        os.makedirs(path)    
        
    def delete_path(self, path):
        if os.path.exists(path):
            shutil.rmtree(path)
        

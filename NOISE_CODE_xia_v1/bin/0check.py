#!/usr/bin/pyenv python

import sys, os
import readConfig
def check():
    if not os.path.exists("Config"):
        print("Config File not exists !!!!!")
    config = readConfig.config()
    config.readConfig("Config")
    if not config.checkConfig():
        print("Error: Config File has some problem!!!")
        return False
    else:
        print("----------------------------------------")
        print("| Config has been setted sucessfully!!!")
        print("----------------------------------------")
        print("| Main Directory:")
        print("| \t"+config.main_dir)
        print("| Code Directory:")
        print("| \t"+config.code_dir)
        print("| Cut Time Begin : "+str(config.cut_start_time))
        print("| Length of Time Window : "+str(config.cut_length))
        filter_range = list(map(str, config.filter_range))
        #print(filter_range)
        str_filter = " ".join(filter_range)
        print("| Filter Range : "+str_filter)
        print("| Cross Correlation Length : "+str(config.correlation_length))
        print("| Day Thresold : "+str(config.thresold_days))
        print("-----------------------------------------")
    if not os.path.exists(config.main_dir):
        print("Error : data directory not exists!!!")
        return False
    if not os.path.exists(config.code_dir):
        print("Error : code directory not exists!!!")
        return False
    if not os.path.exists(config.main_dir+"/station.lst.all"):
        print("Error : station list file not exist!!!")
        return False
    if not os.path.exists(config.main_dir+"/file_dir"):
        print("Error : file list not exists!!!!")
        return True
    else:
        print("-------------------------------------")
        print("| Files check sucessfully !!!")
        print("-------------------------------------")

if __name__ == "__main__":
    check()

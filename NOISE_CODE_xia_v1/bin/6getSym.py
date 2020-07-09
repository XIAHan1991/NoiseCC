#!/usr/bin/pyenv python

import readConfig
import os, glob

if __name__ == "__main__":
    myConfig = readConfig.config()
    myConfig.readConfig("Config")
    os.chdir(myConfig.main_dir)
    os.chdir("stack")
    os.system("ls *.SAC > filelist")
    os.system(myConfig.code_dir+"/CH/yangch")

#!/usr/bin/pyenv python

import readConfig
import os, glob

if __name__ == "__main__":
    myConfig = readConfig.config()
    myConfig.readConfig("Config")
    os.chdir(myConfig.main_dir)
    filter_dir = str(myConfig.filter_range[2])+"to"+str(myConfig.filter_range[1])+"_cor"
    with open("file_dir", 'r') as fileF:
        for fileInfo in fileF:
            fileInfo  = fileInfo.strip()
            try:
                os.chdir(fileInfo)
                os.chdir(filter_dir)
                os.system('cp -f ../sac_db.out ./')
                if not os.path.exists("COR"):
                    os.mkdir("COR")
                os.system(myConfig.code_dir+"/NEWCORR/justCOR "+str(myConfig.correlation_length))
                os.chdir(myConfig.main_dir)
            except Exception:
                break

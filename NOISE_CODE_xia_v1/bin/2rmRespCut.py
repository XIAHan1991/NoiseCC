#!/usr/bin/pyenv python

import readConfig
import os, glob, datetime

if __name__ == "__main__":
    myConfig = readConfig.config()
    myConfig.readConfig("Config")
    os.chdir(myConfig.main_dir)
    with open("file_dir", 'r') as fileF:
        for fileInfo in fileF:
            fileInfo  = fileInfo.strip()
            try:
                os.chdir(fileInfo)
                startTime = str(myConfig.cut_start_time)
                cutLength = str(myConfig.cut_length)
                os.system(myConfig.code_dir+"/CUT_TRANS/cut_trans_coda "+startTime+" "+cutLength)
                os.chdir("..")
            except Exception:
                break

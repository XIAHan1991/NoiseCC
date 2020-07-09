#!/usr/bin/pyenv python

import readConfig
import os, glob

if __name__ == "__main__":
    myConfig = readConfig.config()
    myConfig.readConfig("Config")
    os.chdir(myConfig.main_dir)
    filter_dir = str(myConfig.filter_range[2])+"to"+str(myConfig.filter_range[1])+"_cor"
    if not os.path.exists("listfile"):
        os.mkdir("listfile")
    with open("file_dir", 'r') as fileF:
        for fileInfo in fileF:
            try:
                fileInfo  = fileInfo.strip()
                os.chdir(fileInfo)
                os.chdir(filter_dir)
                os.chdir("COR")
                path = myConfig.main_dir+"/"+fileInfo+"/"+filter_dir+"/COR/"

                ccFiles = glob.glob("COR*.SAC")
                os.chdir("..")
                os.system("cp ../event_station.tbl ./")
                outF1 = open(fileInfo, "w")
                for ccFile in ccFiles:
                    ccFile.strip()
                    print(ccFile)
                    outF1.write(ccFile+"\n")
                outF1.close()
                os.system(myConfig.code_dir+"/STACK/getdays_xia "+fileInfo+" event_station.tbl")

                outF3 = open('temp', 'w')
                outF3.write(path+"\n")
                with open(fileInfo+".lst", 'r') as infos:
                    for info in infos:
                        info =info.strip()
                        outF3.write(info+"\n")
                outF3.close()
                os.system("mv temp "+myConfig.main_dir+"/listfile/"+fileInfo+".lst")
                os.system("rm -rf "+fileInfo+" "+fileInfo+".lst")
                os.chdir(myConfig.main_dir)
            except Exception:
                break
    os.chdir(myConfig.main_dir)
    if not os.path.exists("stack"):
        os.mkdir("stack")
    stackPath = myConfig.main_dir+"/stack/"
    os.chdir("listfile")
    if os.path.exists("all.list"):
        os.system("rm -rf all.list")
    os.system("ls *.lst > all.list")
    if not os.path.exists("scripts"):
        os.mkdir("scripts")
    os.system(myConfig.code_dir+"/STACK/ymkstack_xia all.list "+stackPath+" "+str(myConfig.thresold_days))

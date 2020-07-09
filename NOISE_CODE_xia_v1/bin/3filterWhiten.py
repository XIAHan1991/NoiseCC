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
                if not os.path.exists(filter_dir):
                    os.mkdir(filter_dir)
                daysDir = glob.glob("*_*_*_*_*_*")
                os.chdir(filter_dir)
                os.system("cp -f ../* ./ &> warning")
                for dayDir in daysDir:
                    if not os.path.exists(dayDir):
                        os.mkdir(dayDir)
                    os.chdir(dayDir)
                    os.system("cp -r ../../"+dayDir+"/ft_* ./")
                    outParam = open("param.dat",'w')
                    for sacFile in glob.glob("ft_*"):
                        sacFile.strip()
                        filerRange = list(map(str, myConfig.filter_range))
                        outParam.write(" ".join(filerRange)+" 1 1 "+sacFile+"\n")
                    outParam.close()
                    os.system(myConfig.code_dir+"/FTA/filter4_f/filter4 param.dat")
                    os.system(myConfig.code_dir+"/FTA/white_outphamp/whiten_phamp param.dat")
                    os.chdir("..")
                os.chdir(myConfig.main_dir)

            except Exception:
                break

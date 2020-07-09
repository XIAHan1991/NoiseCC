#!/usr/bin/pyenv python
import readConfig
import os, glob, datetime
if __name__ == "__main__":
    myConfig = readConfig.config()
    myConfig.readConfig("Config")
    #print(myConfig.main_dir)
    os.chdir(myConfig.main_dir)
    with open("file_dir", 'r') as fileF:
        for fileInfo in fileF:
            fileInfo  = fileInfo.strip()
            os.system("cp station.lst.all "+fileInfo+"/station.lst")
            try:
                os.chdir(fileInfo)
                outFile = open("event.dat", 'w')
                for days in glob.glob("*_*_*_*_*"):
                    my_date = datetime.datetime.strptime(days, '%Y_%m_%d_%H_%M_%S')
                    my_time = my_date.strftime("%H%M%S")
                    #print(my_time)
                    year ="%4d" % my_date.year
                    month = "   %2d" % my_date.month
                    day = " %2d" % my_date.day
                    outFile.write(year+month+day+" "+my_time+"\n")
                outFile.close()
                os.system(myConfig.code_dir+"/SET_SACDB/set_sacdb_cut")    

                os.chdir("..")
            except Exception:
                break

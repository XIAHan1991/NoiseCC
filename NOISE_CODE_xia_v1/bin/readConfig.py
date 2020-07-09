#!/usr/bin/pyenv python


class config:

    def __init__(self):
        self.main_dir = ''
        self.code_dir = ''
        self.cut_start_time = 0
        self.cut_length = 0
        self.filter_range = []
        self.correlation_length = 0
        self.thresold_days  = 0
        self.label = True

    def readConfig(self, file):
        with open(file, 'r') as conFile:
            for s in conFile:
                s = s.strip()
                if len(s) == 0:   continue
                elif s[0] == '#': continue
                elif s[:8] == "main_dir":
                    s =s.replace("=", " ")
                    self.main_dir = s.split()[1]
                elif s[:8] == "code_dir":
                    s = s.replace("=", " ")
                    self.code_dir = s.split()[1]
                else:             exec("self."+s)
    def checkConfig(self):
        if len(self.main_dir) == 0:
            print("Error: Not Set Main Directory!!!!!")
            label = False
        if len(self.code_dir) == 0:
            print("Error: Not Set Code Directory!!!!!")
            label = False
        if self.cut_start_time <= 0:
            print("Warning: Cutting time window start from "+str(self.cut_start_time)+ " second!!!")
        if self.cut_length <= 0:
            self.label = False
            print("Error: Length of time window is not correct!!!")
        if len(self.filter_range) != 4:
            self.label = False
            print("Error: Format of filter range is not correct!!!")
        else:
            for i in range(0, 3):
                if self.filter_range[i] <= self.filter_range[i + 1]:
                    print("Error: Filter range is not corrected !!!!")
                    self.label = False
                    break
        if self.correlation_length <= 0:
            self.label = False
            print("Error: Length of Cross Correlation is less than zero !!!")

        if self.thresold_days <= 0:
            print("Warning: The thresold days is "+str(self.thresold_days)+"!!!")
        return self.label


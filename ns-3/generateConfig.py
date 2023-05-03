import csv
import sys
import subprocess


filename=sys.argv[1]+"_config.csv"
with open(filename, 'w', newline='') as writefile:
        writer=csv.writer(writefile)
        inp = input("Enable DDoS (Y/N)?\n")
        attack = False
        icaap = False
        if (inp[0].upper() == "Y"):
            attack = True
            writefile.writelines("DDoS=1\n")
        else:
            writefile.writelines("DDoS=0\n")
        inp = input("Enable token bucket (Y/N)? \n")
        if (inp[0].upper() == "Y"):
            icaap = True
            writefile.writelines(["icaap=1\n"])
        else:
            writefile.writelines(["icaap=0\n"])
        if attack and icaap:
            inp = input("Enable mitigation (Y/N)? \n")
            if (inp[0].upper() == "Y"):
                writefile.writelines(["mit=1\n"])
            else:
                writefile.writelines(["mit=0\n"])
        else:
            writefile.writelines(["mit=0\n"])
        writefile.writelines(["run=1\n"])
        writefile.writelines(["fillrates=\"400_500_1200_200\"\n"])
        writefile.writelines(["capacities=\"200_200_500_200\"\n"])
        inp = input("Select model (1: Orginal Duke topology, 2: Optimized Duke topology)\n")
        if inp == "1":
            writefile.writelines(["config=\"../topology/interface/caseDuke.txt\"\n"])
            writefile.writelines(["device=\"../topology/interface/dataDuke.txt\"\n"])
        else:
            writefile.writelines(["config=\"../topology/interface/caseDukeV3.txt\"\n"])
            writefile.writelines(["device=\"../topology/interface/dataDuke.txt\"\n"])
        writefile.writelines(["timeStep=1.0\n"])
        writefile.writelines(["endTime=10800.0\n"])
        writefile.writelines(["portNum=5005\n"])
        writefile.writelines(["model_id=9\n"])
        inp = input("Application Type (1: Smoothing (Minimizing netload variation), 2: Tieflow Management (Tieflow=0), 3: Cluster Netload minimization (OPF=0), 4: OPF Tracking, 5: Area Error Minimization\n")
        writefile.writelines(["app_id="+inp+"\n"])
        writefile.writelines(["model_id=0\n"])
        inp = input("Communication type 1: Ideal, 2: Simulated\n")
        writefile.writelines(["commtype="+inp+"\n"])
        if inp == "1":
            inp = input("Include Regulator setpoints (Not supported with simulated) 1: Yes, 0: No\n")
            writefile.writelines(["include_regulator="+inp+"\n"])
        else:
            writefile.writelines(["include_regulator=0\n"])
        inp = input("Clustertype 1: Active Power, 2: Sensitivity based, 3: Active power and Sensitivity based\n")
        writefile.writelines(["Clustertype="+inp+"\n"])
        writefile.writelines(["includeswitch=0\n"])
        writefile.writelines(["jacobianstudy=0\n"])
        inp = input("Reconfiguration 1: Yes, 0: No\n")
        writefile.writelines(["recon="+inp+"\n"])
        writefile.writelines(["vis_mod=0\n"])
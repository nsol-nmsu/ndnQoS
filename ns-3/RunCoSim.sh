#!/bin/bash
run=1
DDoS=1
mit=1
icaap=1
fillrates="400_500_1200_200"
capacities="200_200_500_200"
config="../topology/interface/caseDukeV3.txt"
device="../topology/interface/dataDuke.txt"
timeStep=1.0
endTime=10800.0
portNum=5005

NS_GLOBAL_VALUE="RngRun=$i" ./waf --run="ndn-sync-Duke --Run=$run --DDoS=$DDoS --Mitigation=$mit --ICAAP=$icaap --Fillrates='$fillrates' --Capacities='$capacities' --ConfigFile=$config --DeviceFile=$device --TimeStep=$timeStep --EndTime=$endTime --PortNum=$portNum"
                  


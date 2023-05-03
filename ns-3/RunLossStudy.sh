#!/bin/bash
run=1
lossRate=500
config="../topology/interface/caseDukeV3.txt"
device="../topology/interface/dataDuke.txt"
timeStep=1.0
endTime=10800.0
portNum=5015


NS_GLOBAL_VALUE="RngRun=$i" ./waf --run="ndn-sync-Duke --Run=$run --LossRate=$lossRate --ConfigFile=$config --DeviceFile=$device --TimeStep=$timeStep --EndTime=$endTime --PortNum=$portNum"
                  


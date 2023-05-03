from __future__ import division 
import csv 
import sys 
from collections import defaultdict 
import numpy as np 
import matplotlib.pyplot as plt 
import glob

def main(filepath=None, maxSuccess=None):
    count = []
    node_count = 0;
    latency_sum = {}
    tp = []
    sent = []
    nodes = {};
    for i in range(0,35):
        tp.append(0)
        count.append(0)
        sent.append(0)
    if filepath is None and maxSuccess is None:
        sys.stderr.write("usage: python3 %s <file-path> <expected-success-number>\n")
        return 1
    xmax = 250
    cumulatedFullLatencies = []
    filesToProcess = glob.glob(filepath+"*" )
    print filesToProcess
    f = open("Loss-DER-%s"%filepath,"w")
    f.write("Setpoints,Measurments\n")

    for filename in filesToProcess:
        pending = {}
        pending1 = {}
        fullLatencies = []
        latencies = defaultdict(lambda: [])
        print("Trying :" + filename)
        latency_count = {};
        lossCount = {};

        with open(filename, 'r') as csvh:
            #dialect = csv.Sniffer().sniff(csvh.read(10*1024))
            dialect = csv.Sniffer().has_header(csvh.read(1024))
            csvh.seek(0)
            reader = csv.reader(csvh, delimiter=' ', skipinitialspace=True)
            next(reader) # skip header
            
            for srcid, dstid, timesent, timerecv, latency, flowcls, pktname in reader:
                #print ("*",pktname,type(pktname))
                if not ("PV" in pktname or "BESS" in pktname): 
                    continue
                #print ("**",pktname)
                flowcls = ""
                if "data" in pktname:
                    flowcls  = "Measurments"
                elif "phy" in pktname:
                    flowcls = "Setpoints"
                if flowcls not in latency_count:
                    latency_count[flowcls] = 0
                    lossCount[flowcls] = 0
                latency_count[flowcls] += 1
                #print(timerecv)
                if timerecv == "7500.000000000":
                   lossCount[flowcls] += 1

        
	lat1 = "N/A"
	lat2 = "N/A"
	lat3 = "N/A"
	if "Setpoints" in latency_count:
		lat1 = str(latency_count["Setpoints"])
	        f.write(str(lossCount["Setpoints"]/int(lat1))+",")
	else:
	        f.write("N/A,")
	if "Measurments" in latency_count:
		lat2 = str(latency_count["Measurments"])
        	f.write(str(lossCount["Measurments"]/int(lat2))+",")
	else:
	        f.write("N/A,")
        f.write("\n")
	print ("Setpoints" +": " + lat1);
        print ("Measurments" +": " +lat2);
        
        
        #print(str(lossCount["TypeI"]) + "  " + str(int(lat1)))





    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))


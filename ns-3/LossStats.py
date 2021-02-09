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
    f = open("Loss-%s"%filepath,"w")
    f.write("TypeI,TypeII,TypeIII\n")

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
                if flowcls not in latency_count:
                    latency_count[flowcls] = 0
                    lossCount[flowcls] = 0
                latency_count[flowcls] += 1
                print(timerecv)
                if timerecv == "400.000000000":
                   lossCount[flowcls] += 1

        print ("TypeI" +": " +str(latency_count["TypeI"]));
        print ("TypeII" +": " +str(latency_count["TypeII"]));
        print ("TypeIII" +": " +str(latency_count["TypeIII"]));
        f.write(str(lossCount["TypeI"]/latency_count["TypeI"])+",")
        f.write(str(lossCount["TypeII"]/latency_count["TypeII"])+",")
        f.write(str(lossCount["TypeIII"]/latency_count["TypeIII"])+"\n")


    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))


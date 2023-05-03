from __future__ import division 
import csv 
import sys 
from collections import defaultdict 
import numpy as np 
import matplotlib.pyplot as plt 
import glob

def main(filepath=None, traceID=None, maxSuccess=None):
    latency_sum = []

    if filepath is None and maxSuccess is None:
        sys.stderr.write("usage: python3 %s <file-path> <expected-success-number>\n")
        return 1
    xmax = 250
    cumulatedFullLatencies = []
    filesToProcess = glob.glob(filepath+"*" )
    print (filesToProcess)
    for filename in filesToProcess:
        pending = {}
        fullLatencies = []
        latencies = defaultdict(lambda: [])
        print("Trying :" + filename)
        with open(filename, 'r') as csvh:
            #dialect = csv.Sniffer().sniff(csvh.read(10*1024))
            dialect = csv.Sniffer().has_header(csvh.read(1024))
            csvh.seek(0)
            reader = csv.DictReader(csvh, dialect=dialect)

            for row in reader:

                time = float(row['time'])
                if row['event'] == 'start':
                   if (row['txid']) not  in pending:
                      pending[(row['txid'])] = time

                elif (row['event'] == 'completed') and row['txid'] in pending: 
                     latency_sum[int(time)] += 1000. * (time - pending[( row['name'])])
                     fullLatencies.append(1000. * (time - pending[( row['name'])]))
                     del pending[( row['name'])]

    c = 0
    f = open(traceID + "UnsortedLatencies.txt","w")
    for lat in fullLatencies:
        f.write(str(lat) + "\n")
 
    #Dump Latencies
    cumulatedFullLatencies.sort()
    f = open(traceID+"SortedLatencies.txt","w")
    for each in cumulatedFullLatencies:
        f.write(str(each) + "\n")

    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))


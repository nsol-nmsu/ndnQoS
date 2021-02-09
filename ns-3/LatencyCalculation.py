import csv
import sys
filename=sys.argv[1]
tempFilename="Avg_"+filename
with open(tempFilename, 'w', newline='') as writefile:
        writer=csv.writer(writefile)
        writer.writerow(["Type I", "Type II", "Type III"])
        #temp="Loss-lat_ndn-case"+str(ii)+"-run"
        #filename= temp+str(j)+".csv"
                
        print(filename)
        f = open(filename, "r")
        lineno=0.0
        type1lat=0.0
        type2lat=0.0
        type3lat=0.0
        count1=0.0
        count2=0.0
        count3=0.0
        for x in f:
                values=x.split()
#               print(values)
                lineno=lineno+1
                if(values[5]=="TypeI") and float(values[3]) != 400:
                        count1=count1+1
                        type1lat=type1lat+float(values[4])
                        print(values[4])
                if(values[5]=="TypeII") and float(values[3]) != 400:
                        count2=count2+1
                        type2lat=type2lat+float(values[4])
                        print(values[4])
                if(values[5]=="TypeIII") and float(values[3]) != 400:
                        count3=count3+1
                        type3lat=type3lat+float(values[4])
                        print(values[4])
        writer.writerow([type1lat/count1,type2lat/count2,type3lat/count3])



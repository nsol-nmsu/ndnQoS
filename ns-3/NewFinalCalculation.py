import csv
for ii in range (1,85):
	tempFilename="FinalCalculation"+str(ii)+".csv"
	with open(tempFilename, 'w', newline='') as writefile:
                #writer=csv.writer(writefile)
                writer = open(writefile.name, "w")
                #writer.writerow(["Type I","Type II", "Type III", "Lat Type I", "Lat Type II", "Lat Type III"])
                writer.write("Type I,Type II,Type III,Lat Type I,Lat Type II,Lat Type III\n")
                for j in range(1,6):
                        print(j)
                        temp="Loss-lat_ndn-case"+str(ii)+"-run"
                        filename= temp+str(j)+".csv"
                        temp="Avg_lat_ndn-case"+str(ii)+"-run"
                        Latfilename=temp+str(j)+".csv"
                        print(Latfilename)
                        type3=0
                        type1=0
                        type2=0

                        Lat_type3=0
                        Lat_type1=0
                        Lat_type2=0
                        f = open(filename, "r")
                        lineno=0
                        for x in f:
                                values=x.split(",")                                 
                                print(str(values))
                                lineno=lineno+1
                                if(lineno==2): 
                                    type1=values[0]
                                if(lineno==2): 
                                    type2=values[1]
                                if(lineno==2): 
                                    type3=values[2].split("\n")[0]

                        g= open(Latfilename,"r")
                        lineno=0
                        for x in g:
                                values = x.split(",")
                                print(values)
                                lineno=lineno+1
                                if(lineno==2):
                                        Lat_type1=values[0]
                                        Lat_type2=values[1]
                                        Lat_type3=values[2].split("\n")[0]
                        #writer.writerow([type1,type2,type3,Lat_type1,Lat_type2,Lat_type3])
                        writer.write(str(type1)+","+str(type2)+","+str(type3)+","+str(Lat_type1)+","+str(Lat_type2)+","+str(Lat_type3)+"\n")
                        print(type3)

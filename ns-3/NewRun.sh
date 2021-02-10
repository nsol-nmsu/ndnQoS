#!/bin/bash


input="./simulationInput.csv"
while IFS=, read -r Test TB_Size_class_1 TB_Size_class_2 TB_Size_class_3 Token_gene_rate_Class_1 Token_gene_rate_Class_2 Token_gene_rate_Class_3 Remark Rate_Range
do
    echo "Test $Test"
	max=5
	for (( i=1; i <= $max; ++i ))
		do
			NS_GLOBAL_VALUE="RngRun=$i" ./waf --run="ndn-case39cyber --Run=$i --Test=$Test --configFilePath=\"/home/george/ndnQoS/topology/interface/case650.txt\" --TB_Size_class_1=$TB_Size_class_1 --TB_Size_class_2=$TB_Size_class_2  --TB_Size_class_3=$TB_Size_class_3 --Token_gene_rate_Class_1=$Token_gene_rate_Class_1 --Token_gene_rate_Class_2=$Token_gene_rate_Class_2 --Token_gene_rate_Class_3=$Token_gene_rate_Class_3 --Rate_Range=$Rate_Range"  &
	
			echo "Start instance $i for id $Test"
	done 
	wait
	echo "Finished for $Test at $(date)!"
	#echo "All Simulations Complete"
break
done < $input
echo "All Simulations Complete for New Run"
                  


~


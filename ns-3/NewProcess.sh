#!/bin/bash


input="./simulationInput.csv"
while IFS=, read -r Test TB_Size_class_1 TB_Size_class_2 TB_Size_class_3 Token_gene_rate_Class_1 Token_gene_rate_Class_2 Token_gene_rate_Class_3 Remark Rate_Range
do
          echo "Test No $Test"
	  max=5
	for (( i=1; i <= $max; ++i ))
		do
			python preproc-cases-ndn.py ndn-case$Test-run$i.csv
			python LossStats.py lat_ndn-case$Test-run$i.csv
			python3 LatencyCalculation.py lat_ndn-case$Test-run$i.csv
			echo "Finished instance $i for $Test at $(date)!"
	done 
	wait
   	echo "All Simulations Complete"
	
done < $input
python3 NewFinalCalculation.py                  


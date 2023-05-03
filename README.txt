I. ICAAP & Co-Simulation
===========
Text that follows "//" are comments and should not be run. 

1. Code Checkout:
----------------
git clone -recursive https://github.com/nsol-nmsu/ndnQoS.git
cd ndnQoS/	// Move to ndnQoS directory

2. Check branch details (make sure you are in 'qos' branch in NDN_QoS, ndnSIM, NFD and ndn-cxx folders)
-----------------------
git branch -a	// Show all branches and the current branch with * symbol.
git remote -v	// Display git url

3. Build code
-------------
cd ns-3/	// Move to ns-3 directory
./waf configure --enable-examples -d optimized	// configure with examples
./waf	// build code once

4. Pushing the code
-------------------
git branch -a	// Make sure you are in proper branch(qos) before making modifications.
git checkout <branch_name>	// To checkout to a spacific branch
git status	// In modified git directory(ndnSIM,NFD,ndn-cxx) will show modified or new file in red color
git add .	// Add all modified files to repository or select individual files instead of dot to add that file alone.
git status	// All the files added to git will change color to green. If something is not added, will remain in red color
git commit -m "Add meaningful comment summerizing the modification made"	// Code commiting locally
git push origin qos	// Pushing the code to remote branch

5.Run QoS enabled Co-Simulation: 
Variables to change simulation configuration can be changed in Script "ns-3/RunCoSim.sh" and are as as follow:
	run: 		Identifier number for a specific run. 
	DDoS:		Set to 0 (False) for no DDoS traffic and 1 (True) for DDoS traffic.
	mit:		Set to 0 (False) to disable mitigation and 1 (True) to enable it.
	icaap: 		Set to 0 (False) to disable ICAAP and 1 (True) to enable it.
	fillrates:	Set token bucket fillrates for the differing classes with each value seperated by a "_".
	capacities: Set token bucket capacities for the differing classes with each value seperated by a "_".
	config:		Specify the file location of the network topology file you wish to run.
	device:		Specify the file location of the device location file.
	timeStep:	The time in seconds between synchronization points with other simulators.
	endTime: 	The duration of the simulation in seconds.
	portNum: 	The port number used by the co-simulation for communication between simulators.
----------------------------
cd ns-3
bash RunCoSim.sh

6.Run Loss Study: 
Variables to change simulation configuration can be changed in Script located at "ns-3/RunLossStudy.sh" and are as as follow:
	run: 		Identifier number for a specific run. 
	lossRate: 	The loss rate you wish to simulate as an integer. Take loss rate and multiply by 1000. Example: 0.01 == 10, 0.5 == 500.
	config: 	Specify the file location of the network topology file you wish to run.
	device: 	Specify the file location of the device location file.
	timeStep: 	The time in seconds between synchronization points with other simulators.
	endTime: 	The duration of the simulation in seconds.
	portNum: 	The port number used by the co-simulation for communication between simulators.
----------------------------
cd ns-3
bash RunLossStudy.sh

II. Folder Structure
=====================
1. ns-3 			- Code base
2. process_result	- Python scripts for simulations and result processing.
3. thirdparty		- Thirdparty libraries used (Required to build the code without error)
4. topology			- Topology files for Monte Carlo simulations used in scenario files
	Topology files avaible are caseDuke.txt, caseDukeV3.txt, dataDuke.txt. File caseDuke.txt is a 
	one-to-one representation of Duke feeder, while caseDukeV3.txt has a more streamlined layout with
	less hops and more redundant paths. 


III. ICAAP & CoSimulation Files (New Mexico State University)
=====================================================

1. ns-3/src/ndnSIM/NFD/daemon/fw

	ndn-priority-tx-queue.cpp
	ndn-priority-tx-queue.hpp
	ndn-qos-queue.cpp
	ndn-qos-queue.hpp
	ndn-token-bucket.cpp
	ndn-token-bucket.hpp
	qos-strategy.cpp
	qos-strategy.hpp
	qos-strategy-mitigation.cpp
	qos-strategy-mitigation.hpp
	TBucketDebug.cpp
	TBucketDebug.hpp

2. ns-3/src/ndnSIM/apps

	TBucketRef.cpp
	TBucketRef.hpp
	ndn-QoS-consumer.cpp
	ndn-QoS-consumer.hpp
    ndn-QoS-producer.cpp
    ndn-QoS-producer.hpp
	tokenBucketDriver.cpp
	tokenBucketDriver.hpp
	ndn-synchronizer.cpp
	ndn-synchronizer.hpp
	ndn-synchronizer-socket.cpp
	ndn-synchronizer-socket.hpp
	ndn-synchronizer-DOE.cpp
	ndn-synchronizer-DOE.hpp
	ndn-synchronizer-DDoS.cpp
	ndn-synchronizer-DDoS.hpp	
	ndn-synchronizer-Loss.cpp
	ndn-synchronizer-Loss.hpp	
	parser-ReDisPV.cpp				
	parser-ReDisPV.hpp				
	parser-OpenDSS.cpp				
	parser-OpenDSS.hpp				


IV . Documentation
==================
Documentation for codebase is available in ns-3/src/ndnSIM/docs/html/ folder.
Open index.html in browser and you will be able to find the documentation similar to ndnSIM.


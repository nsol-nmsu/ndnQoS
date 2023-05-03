/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <fstream>
#include <string>
#include <stdlib.h>
#include <iomanip>
#include <iostream>
#include <time.h>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/icens-app-helper.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/netanim-module.h"
#include "src/applications/model/icens-sub-cosim.h"

#include "nlohmann/json.hpp"

#include <unordered_map>
namespace ns3 {

//NS_LOG_COMPONENT_DEFINE ("iCenS");
using json = nlohmann::json;

void SentPacketCallbackPhy(uint32_t, Ptr<Packet>, const Address &, uint32_t);
void ReceivedPacketCallbackCom(uint32_t, Ptr<Packet>, const Address &, uint32_t, uint32_t size, uint32_t subscription, Ipv4Address local_ip);

void SentPacketCallbackCom(uint32_t, Ptr<Packet>, const Address &, uint32_t);
void ReceivedPacketCallbackPhy(uint32_t, Ptr<Packet>, const Address &, uint32_t, uint32_t size, uint32_t subscription, Ipv4Address local_ip);


// Vectors to store the various node types
std::vector<int> com_nodes, agg_nodes, phy_nodes;

// Vectors to store source and destination edges to fail
std::vector<int> srcedge, dstedge;

std::pair<int,int> flow_pair;

// Vectors to store agg<--->com and phy<--->agg p2p IPs, packets are directed to these IP addresses
std::vector<std::string> com_ips;
std::vector<std::pair<int,std::string>> phy_to_agg_map;
std::string GetIP(int nodeid);
bool HasIP(int nodeid);
// Nodes and their corresponding IPs, used to output namespaces in tracefile
std::vector<std::pair<int,std::string>> node_ip_map;
std::vector<std::pair<int,std::string>> wac_ip_map;
std::vector<std::pair<int,std::string>> lc_ip_map;
std::vector<std::pair<int,std::string>> pmu_ip_map;

// Node and interface pairs to fail, between com and agg
std::vector<std::pair<int,int>> node_link_map_fail;

// Split a line into a vector of strings
std::vector<std::string> SplitString(std::string);

//Store unique IDs to prevent repeated installation of app
bool IsPMUAppInstalled( std::string PMUID );
bool IsLCAppInstalled( std::string LCID );
bool IsWACAppInstalled(std::string WACID);

//Get node IP from subnet defined in topology file
std::string GetIPFromSubnet(std::string nodeid, std::string subnet, int num);

std::string GetLCIP(int pdcid);
std::string GetWACIP(int wacid);

std::vector<std::string> uniquePMUs;
std::vector<std::string> uniqueLCs;
std::vector<std::string> uniqueWACs;
std::vector<std::pair<int,int>> all_flows;

std::pair<int,int> pmu_nodes;
std::pair<int,int> lc_nodes;
std::pair<int,int> wac_nodes;


bool NodeInComm(int);
bool NodeInAgg(int);
bool NodeInPhy(int);
int GetNodeFromIP(std::string);
std::string GetAggIP(int);

//Trace file
std::ofstream tracefile;

std::ofstream flowfile;

//Gets current time to make packets unique
time_t seconds;

int main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Open the configuration file for reading
  std::ifstream configFile ("/newhome/george/folder/case650.txt", std::ios::in);
  ifstream jsonFile ( "/newhome/george/folder/data650.txt", std::ios::in );	// Device - Node mapping

  json jf = json::parse( jsonFile );
  json::iterator it = jf.begin();

  unordered_map<int,std::vector<std::string>> nameMap;

  // Debug print
  while ( it !=  jf.end() )
  {
	nameMap[ it.value() ].push_back( it.key() );
	it++;
  }

  std::cout<<"hs\n";

  std::string strLine;
  bool gettingNodeCount = false, buildingNetworkTopo = false, TypeI = false, TypeII = false;
  bool failLinks = false, injectData = false;
  std::vector<std::string> netParams;

  NodeContainer nodes;
  int nodeCount = 0;

  // Setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::DropTailQueue::MaxPackets", StringValue("50"));
  //Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));

  PointToPointHelper p2p;
  NetDeviceContainer devices;
  Ipv4AddressHelper addresses;

  std::pair<int,std::string> node_ip_pair;

  ProducerHelper proHelper;
  ApplicationContainer proApps;
  CosimHelper consumerHelper;
  ApplicationContainer conApps;
  unordered_map<int,int> used;
  std::unordered_map<int,int> usedS;
  std::unordered_map<int,int> usedR;


  uint16_t wacPort = 5000;
  uint16_t lcPort = 6000;
  uint16_t setpointPort = 6500;
  uint16_t pmuPort = 4000;
  uint16_t bgdPort = 1000;

  Ptr<Ipv4> ipv4node;

  flowfile.open("ip_all_flows.csv", std::ios::out);
  std::vector<int> lastIP = {10, 0, 0, 4};

  if (configFile.is_open ()) {
        while (std::getline(configFile, strLine)) {

                //Determine what operation is ongoing while reading the config file
		if(strLine.substr(0,7) == "BEG_000") { gettingNodeCount = true; continue; }
                if(strLine.substr(0,7) == "END_000") {
		       std::cout<<nodeCount<<std::endl;	
			//Create nodes
			nodeCount++; //Counter increment needed because nodes start from 1 not 0
			gettingNodeCount = false;
			nodes.Create(nodeCount);
			//Install internet protocol stack on nodes
			InternetStackHelper stack1;
  			stack1.Install (nodes);

			continue; 
		}
                if(strLine.substr(0,7) == "BEG_001") { buildingNetworkTopo = true; continue; }
                if(strLine.substr(0,7) == "END_001") { buildingNetworkTopo = false; continue; }
		if(strLine.substr(0,7) == "BEG_005") { TypeI = true; continue; }
                if(strLine.substr(0,7) == "END_005") { TypeI = false; continue; }
		if(strLine.substr(0,7) == "BEG_006") { TypeII = true; continue; }
                if(strLine.substr(0,7) == "END_006") { TypeII = false; continue; }
		if(strLine.substr(0,7) == "BEG_100") { failLinks = true; continue; }
                if(strLine.substr(0,7) == "END_100") { failLinks = false; continue; }
		if(strLine.substr(0,7) == "BEG_101") { injectData = true; continue; }
                if(strLine.substr(0,7) == "END_101") { injectData = false; continue; }


		if(gettingNodeCount == true) {
                        //Getting number of nodes to create
			netParams = SplitString(strLine);
			if(netParams[2] == "routers"){
			   nodeCount = stoi(netParams[1]);
			}
		}
		else if(buildingNetworkTopo == true) {
                        //Building network topology
                        netParams = SplitString(strLine);
                        p2p.SetDeviceAttribute("DataRate", StringValue(netParams[2]+"Mbps"));
			//p2p.SetDeviceAttribute("DataRate", StringValue("50Kbps"));
                        p2p.SetChannelAttribute("Delay", StringValue(netParams[3]+"ms"));
          		devices = p2p.Install(nodes.Get(std::stoi(netParams[0])), nodes.Get(std::stoi(netParams[1])));
			std::string IP = std::to_string(lastIP[0])+"."+std::to_string(lastIP[1])+"."
				+ std::to_string(lastIP[2])+"."+std::to_string(lastIP[3]);
			addresses.SetBase (Ipv4Address (IP.c_str()), Ipv4Mask(netParams[5].c_str()));
  			addresses.Assign (devices);

			lastIP[3]+=4;
                        if(lastIP[3]>=256){
                           lastIP[3] = 4;
                           lastIP[2]+=1;
                        }

                        node_ip_pair.first = std::stoi(netParams[0]);
                        node_ip_pair.second = GetIPFromSubnet(netParams[0], IP, 1);
                        node_ip_map.push_back(node_ip_pair);


                      //  if(!HasIP(std::stoi(netParams[1]))){ 
			node_ip_pair.first = std::stoi(netParams[1]);
                        node_ip_pair.second = GetIPFromSubnet(netParams[1], IP, 2);
                        node_ip_map.push_back(node_ip_pair);
                //}
		//	if(!HasIP(std::stoi(netParams[0]))){
                //      }

                }
		
		else if(TypeI == true) {
			//Install apps on PDCs and PMUs for data exchange
			netParams = SplitString(strLine);

			//Install app on unique PDC IDs
			if (IsLCAppInstalled(netParams[0]) == false) {
				proHelper.SetAttribute ("LocalPort", UintegerValue (lcPort));
        			proApps = proHelper.Install (nodes.Get (std::stoi(netParams[0])));
                                
				used[std::stoi( netParams[0] )] = 1;
				usedR[std::stoi(netParams[0])] = 1;


				//Install flow app on PMUs to send data to PDCs
                		consumerHelper.SetAttribute ("RemoteAddress", AddressValue (InetSocketAddress (
								Ipv4Address(GetIP(std::stoi(netParams[1])).c_str()), setpointPort)));
                		consumerHelper.SetAttribute ("RemotePort", UintegerValue(setpointPort));
				//consumerHelper.SetAttribute ("Interval", TimeValue (Seconds (1.0/(float(rand()%10+85))))); //0.016 or 0.02
                		consumerHelper.SetAttribute ("Subscription", UintegerValue (0));
                		consumerHelper.SetAttribute ("PacketSize", UintegerValue (200));
                		conApps = consumerHelper.Install (nodes.Get (std::stoi(netParams[0])));
				auto apps = conApps.Get(0)->GetObject<ns3::iCenSSub>();
                        }
			
			if ( IsPMUAppInstalled( netParams[1] ) == false ){
                                proHelper.SetAttribute ("LocalPort", UintegerValue (setpointPort));
                                proApps = proHelper.Install (nodes.Get (std::stoi(netParams[1])));
				proHelper.SetAttribute ("LocalPort", UintegerValue (lcPort));
                                proApps = proHelper.Install (nodes.Get (std::stoi(netParams[1])));

	
	               		consumerHelper.SetAttribute ("RemoteAddress", AddressValue (InetSocketAddress (Ipv4Address(GetIP(std::stoi(netParams[0])).c_str()), lcPort)));
                		consumerHelper.SetAttribute ("RemotePort", UintegerValue(lcPort));
               			consumerHelper.SetAttribute ("Subscription", UintegerValue (0));
               			consumerHelper.SetAttribute ("PacketSize", UintegerValue (200));
               			conApps = consumerHelper.Install (nodes.Get (std::stoi(netParams[1])));

                       		usedS[std::stoi(netParams[1])] = 1;
                       		used[std::stoi(netParams[1])] = 1;
				auto apps = conApps.Get(0)->GetObject<ns3::iCenSSub>();

                        }

			// Save the flow
			flow_pair.first = stoi( netParams[0] );
			flow_pair.second = stoi( netParams[1] );
			all_flows.push_back( flow_pair );
			//Write flow to file
			flowfile << netParams[0] << " " << netParams[1] << " TypeI" << std::endl;
		}
		else if(TypeII == true) {
                        //Install apps on WACs and PMUs for data exchange
                        netParams = SplitString(strLine);

                        //Install app on unique PDC IDs
                        if (IsWACAppInstalled(netParams[0]) == false) {
                                proHelper.SetAttribute ("LocalPort", UintegerValue (wacPort));
                               proApps = proHelper.Install (nodes.Get (std::stoi(netParams[0])));
                               used[std::stoi(netParams[0])] = 1;
                               usedR[std::stoi(netParams[0])] = 1;

                        }

			if ( IsPMUAppInstalled( netParams[1] ) == false ) { 

    	   			//Install flow app on PMUs to send data to WACs
				consumerHelper.SetAttribute ("RemoteAddress", AddressValue (InetSocketAddress (Ipv4Address(GetIP(std::stoi(netParams[0])).c_str()), wacPort)));
				consumerHelper.SetAttribute ("Interval", TimeValue (Seconds (1.0/(float(rand()%10+145))))); //0.016 or 0.02
        	                consumerHelper.SetAttribute ("Subscription", UintegerValue (0));
      		                consumerHelper.SetAttribute ("PacketSize", UintegerValue (200));
	                        conApps = consumerHelper.Install (nodes.Get (std::stoi(netParams[1])));
        	                used[std::stoi(netParams[1])] = 1;
                	        usedS[std::stoi(netParams[1])] = 1;
			}

			// Save the flow
			flow_pair.first = stoi( netParams[0] );
			flow_pair.second = stoi( netParams[1] );
			all_flows.push_back( flow_pair );

			//Write flow to file
			flowfile << netParams[0] << " " << netParams[1] << " TypeII" << std::endl;

                }
		else if(failLinks == true) {
                        //Fail links specified in the config file
                        netParams = SplitString(strLine);

			ipv4node = nodes.Get(stoi(netParams[1]))->GetObject<Ipv4> ();
                        Simulator::Schedule (Seconds ( ((double)stod(netParams[2])) ),&Ipv4::SetDown,ipv4node, stoi(netParams[4]));
                        Simulator::Schedule (Seconds ( ((double)stod(netParams[3])) ),&Ipv4::SetUp,ipv4node, stoi(netParams[4]));

		}
		else if(injectData == true) {
                        //Install apps on WACs, PDCs and PMUs for background data injection
                        netParams = SplitString(strLine);

			//Install app on target node to be used for background data injection
                        proHelper.SetAttribute ("Port", UintegerValue (bgdPort));
                        proApps = proHelper.Install (nodes.Get (std::stoi(netParams[0])));
                        usedR[std::stoi(netParams[0])] = 1;

			std::string targetDOS = "";
                        targetDOS = GetIP(std::stoi(netParams[0]));

                        //Install flow app on PMUs to send background data to target node
                        consumerHelper.SetAttribute ("RemoteAddress", AddressValue (InetSocketAddress (Ipv4Address(targetDOS.c_str()), bgdPort)));

			consumerHelper.SetAttribute ("Interval", TimeValue (Seconds (1.0/(float(rand()%10+295))))); //0.0002 = 5000pps
                        consumerHelper.SetAttribute ("Subscription", UintegerValue (0));
                        consumerHelper.SetAttribute ("PacketSize", UintegerValue (1024));
                        ApplicationContainer bgdApps = consumerHelper.Install (nodes.Get (std::stoi(netParams[1])));
                        usedS[std::stoi(netParams[1])] = 1;

		}
		else {
			//std::cout << "reading something else " << strLine << std::endl;
		}

	}

  }
  else {
	std::cout << "Cannot open configuration file!!!" << std::endl;
	exit(1);
  }

  configFile.close();

  //Populate the routing table
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();


  //Open trace file for writing
  tracefile.open("tcp-case39cyber-trace.csv", std::ios::out);
  tracefile << "nodeid, event, name, payloadsize, time" << std::endl;

  std::string strcallback;

  for ( int i=0; i<( int )nodes.size(); i++ ) {
     if( usedS[i] == 1 ){
        strcallback = "/NodeList/" + std::to_string(i) + "/ApplicationList/" + "*/SentPacketSub";
        Config::ConnectWithoutContext(strcallback, MakeCallback(&SentPacketCallbackPhy));
	strcallback = "/NodeList/" + std::to_string(i) + "/ApplicationList/" + "*/ReceivedPacketP";
        Config::ConnectWithoutContext(strcallback, MakeCallback(&ReceivedPacketCallbackPhy));
     }
  }

  for ( int i=0; i<( int )nodes.size(); i++ ) {
     if( usedR[i] == 1 ){
      strcallback = "/NodeList/" + std::to_string(i) + "/ApplicationList/" + "*/ReceivedPacketP";
      Config::ConnectWithoutContext(strcallback, MakeCallback(&ReceivedPacketCallbackCom));
     }
  }
  //Run actual simulation
  Simulator::Stop (Seconds(220.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

//Split a string delimited by space
std::vector<std::string> SplitString(std::string strLine) {
        std::string str = strLine;
        std::vector<std::string> result;
        std::istringstream isstr(str);
        for(std::string str; isstr >> str; )
                result.push_back(str);

        return result;
}

//Store unique PDC IDs to prevent repeated installation of app
bool IsLCAppInstalled(std::string PDCID) {
	for (int i=0; i<(int)uniqueLCs.size(); i++) {
		if(PDCID.compare(uniqueLCs[i]) == 0) {
			return true;
		}
	}

	uniqueLCs.push_back(PDCID);
	return false;
}

//Store unique PDC IDs to prevent repeated installation of app
bool IsPMUAppInstalled(std::string PMUID) {
	for (int i=0; i<(int)uniquePMUs.size(); i++) {
		if(PMUID.compare(uniquePMUs[i]) == 0) {
			return true;
		}
	}

	uniquePMUs.push_back(PMUID);
	return false;
}


//Store unique WAC IDs to prevent repeated installation of app
bool IsWACAppInstalled(std::string WACID) {
        for (int i=0; i<(int)uniqueWACs.size(); i++) {
                if(WACID.compare(uniqueWACs[i]) == 0) {
                        return true;
                }
        }

        uniqueWACs.push_back(WACID);
        return false;
}

//Get node IP from subnet defined in topology file
std::string GetIPFromSubnet(std::string nodeid, std::string subnet, int num) {

	std::string node_p2p_ip = "";
	std::pair<int,std::string> all_nodes_ip_pair;

 	std::size_t dotposition = subnet.find(".",0) + 1;
        node_p2p_ip = node_p2p_ip + subnet.substr(0,dotposition);

        std::string ipsecondtofourthoctet = subnet.substr( dotposition, std::string::npos );
        dotposition = ipsecondtofourthoctet.find(".",0) + 1;
        node_p2p_ip = node_p2p_ip + ipsecondtofourthoctet.substr(0,dotposition);

        std::string ipthirdtofourthoctet = ipsecondtofourthoctet.substr( dotposition, std::string::npos );
        dotposition = ipthirdtofourthoctet.find(".",0) + 1;
        node_p2p_ip = node_p2p_ip + ipthirdtofourthoctet.substr(0,dotposition);

        std::string ipfourthoctet = ipthirdtofourthoctet.substr( (ipthirdtofourthoctet.find(".",0))+1, std::string::npos );
        node_p2p_ip = node_p2p_ip + std::to_string(std::stoi(ipfourthoctet) + num);
	//Store all node and their IPs, used for trace file
	all_nodes_ip_pair.first = std::stoi(nodeid);
	all_nodes_ip_pair.second = node_p2p_ip;
	//node_ip_map.push_back(all_nodes_ip_pair);

	return node_p2p_ip;
}

//Define callbacks for writing to tracefile
void SentPacketCallbackPhy(uint32_t nodeid, Ptr<Packet> packet, const Address &address, uint32_t seqNo) {

	int packetSize = packet->GetSize ();
	std::cout<<"Sending\n";

	//Remove 4 extra bytes used for subscription
        packetSize = packetSize - 4;

	packet->RemoveAllPacketTags ();
    	packet->RemoveAllByteTags ();

    	//Get subscription value set in packet's header
    	//iCenSHeader packetHeader;
    	//packet->RemoveHeader(packetHeader);
	std::stringstream ss;   std::string str_ip_address;
                        ss << InetSocketAddress::ConvertFrom (address).GetIpv4();
                        ss >> str_ip_address;
        if (InetSocketAddress::ConvertFrom (address).GetPort () == 6000) {
           tracefile << nodeid << ", sent, " << "/power/TypeI/data" << nodeid << "/" 
	           << InetSocketAddress::ConvertFrom (address).GetIpv4 () << "/" << GetNodeFromIP(str_ip_address) 
		   << "/" << seqNo << ", " << packetSize << ", " << std::fixed << std::setprecision(9) 
		   << (Simulator::Now().GetNanoSeconds())/1000000000.0 << std::endl;
	}
	else {
  	   tracefile << nodeid << ", sent, " << "/power/TypeI/data/phy" << nodeid << "/"
		   << InetSocketAddress::ConvertFrom (address).GetIpv4 () << "/" << GetNodeFromIP(str_ip_address)
                   << "/" << seqNo << ", " << packetSize << ", " << std::fixed << std::setprecision(9)
                   << (Simulator::Now().GetNanoSeconds())/1000000000.0 << std::endl;

	}

}

void ReceivedPacketCallbackPhy(uint32_t nodeid, Ptr<Packet> packet, const Address &address, uint32_t localport, uint32_t size, uint32_t subscription, Ipv4Address local_ip) {

	int packetSize = packet->GetSize();

          uint8_t pData[packetSize];
          packet->CopyData (pData, sizeof(pData));
          std::vector<uint8_t> payloadVector( &pData[0], &pData[packetSize] );
	  std::string payload( payloadVector.begin(), payloadVector.end() );
          std::vector<std::string> pl  = SplitString(payload);

	//Extra 4 bytes already removed at COM app layer while checking if sequence number is greater than 100

	//At this point "subscription" parameter contains sequence number of packet
	if (localport == 6000) {
           std::string str = /*std::to_string( nodeid )*/std::to_string(  packetSize) + " " + pl[0] + " " 
		   + std::to_string( ( Simulator::Now( ).GetSeconds( ) ) ) + " OpenDSS "+pl[2] ;           

 	   std::stringstream ss;   std::string str_ip_address;
	   ss << InetSocketAddress::ConvertFrom (address).GetIpv4();
	   ss >> str_ip_address;

	   tracefile << nodeid << ", recv, " << "/power/TypeI/data"  << GetNodeFromIP(str_ip_address) << "/" << local_ip 
		   << "/" << nodeid << "/" << subscription << ", " << packetSize << ", " 
		   << std::fixed << std::setprecision(9) << (Simulator::Now().GetNanoSeconds())/1000000000.0 << std::endl;
	}

	else {
           std::string str = std::to_string( packetSize ) + " " + pl[0]
                   + " " + std::to_string( ( Simulator::Now( ).GetSeconds( ) ) ) + " RedisPV " + payload;


  	   std::stringstream ss;   std::string str_ip_address;
           ss << InetSocketAddress::ConvertFrom (address).GetIpv4();
           ss >> str_ip_address;

	   tracefile << nodeid << ", recv, " << "/power/TypeI/data/phy"  << GetNodeFromIP(str_ip_address) << "/" << local_ip
                   << "/" << nodeid << "/" << subscription << ", " << packetSize << ", "
                   << std::fixed << std::setprecision(9) << (Simulator::Now().GetNanoSeconds())/1000000000.0 << std::endl;


	}
}

void SentPacketCallbackCom(uint32_t nodeid, Ptr<Packet> packet, const Address &address, uint32_t localport) {

	if (localport !=8000) {
		//Do not log data for error, PMU ACK, AMI ACK from compute
	}
	else {
		tracefile << nodeid << ", sent, " << "/overlay/com/subscription/" << "0" << ", " << packet->GetSize () << ", " << std::fixed
        		<< std::setprecision(9) << (Simulator::Now().GetNanoSeconds())/1000000000.0 << std::endl;
	}
}

void ReceivedPacketCallbackCom(uint32_t nodeid, Ptr<Packet> packet, const Address &address, uint32_t localport, 
		uint32_t size, uint32_t subscription, Ipv4Address local_ip) {
        
  int packetSize = packet->GetSize();
  uint8_t pData[packetSize];
  packet->CopyData (pData, sizeof(pData));
  std::vector<uint8_t> payloadVector( &pData[0], &pData[packetSize] );
  std::string payload( payloadVector.begin(), payloadVector.end() );
  std::vector<std::string> pl  = SplitString(payload);
 
  std::string str = std::to_string( packetSize) + " " + pl[0] + " " + std::to_string( ( Simulator::Now( ).GetSeconds( ) ) ) 
	  + " OpenDSS "+ pl[1];


  //Only log flows that are permitted in config file ( eliminates redundant interests received from multiple interfaces and to other nodes )
  //if ( FlowPermitted( ( int )nodeid, ( int )GetSourceNodeID( iname ) ) == true ) {
  std::stringstream ss;   std::string str_ip_address;
  ss << InetSocketAddress::ConvertFrom (address).GetIpv4();
  ss >> str_ip_address;

  tracefile << nodeid << ", recv, " << "/power/TypeI/data"  << GetNodeFromIP(str_ip_address) << "/" << local_ip
	  << "/" << nodeid << "/" << subscription << ", " << packetSize << ", "
	  << std::fixed << std::setprecision(9) << (Simulator::Now().GetNanoSeconds())/1000000000.0 << std::endl;
}


bool NodeInComm(int nodeid) {
        for (int i=0; i<(int)com_nodes.size(); i++) {
                if (com_nodes[i] == nodeid) {
                        return true;
                }
        }
        return false;
}

bool NodeInAgg(int nodeid) {
        for (int i=0; i<(int)agg_nodes.size(); i++) {
                if (agg_nodes[i] == nodeid) {
                        return true;
                }
        }
        return false;
}

bool NodeInPhy(int nodeid) {
        for (int i=0; i<(int)phy_nodes.size(); i++) {
                if (phy_nodes[i] == nodeid) {
                        return true;
                }
        }
        return false;
}


int GetNodeFromIP(std::string str_ip) {
	for (int i=0; i<(int)node_ip_map.size(); i++) {
		if (node_ip_map[i].second == str_ip) {
			return node_ip_map[i].first;
		}
	}
	std::cout << "IP address " << str_ip << " not mapped to any node!!! Cannot write correct entry in tracefile" << std::endl;
	exit(1);
}


std::string GetAggIP(int nodeid) {
	for (int i=0; i<(int)phy_to_agg_map.size(); i++) {
		if (nodeid == phy_to_agg_map[i].first) {
			return phy_to_agg_map[i].second;
		}
	}
	std::cout << "Node " << nodeid << ", not connected to any aggregation IP!!!!!" << std::endl;
	exit(1);
}

std::string GetLCIP(int pdcid) {
        for (int i=0; i<(int)lc_ip_map.size(); i++) {
                if (pdcid == lc_ip_map[i].first) {
                        return lc_ip_map[i].second;
                }
        }
        std::cout << "PDC " << pdcid << ", has no corresponding IP!!!!!" << std::endl;
        
	exit(1);
}

std::string GetIP(int nodeid) {
        for (int i=0; i<(int)node_ip_map.size(); i++) {
                if (nodeid == node_ip_map[i].first) {
                        return node_ip_map[i].second;
                }
        }
        std::cout << "Node " << nodeid << ", has no corresponding IP!!!!!" << std::endl;

        exit(1);
}

bool HasIP(int nodeid) {
        for (int i=0; i<(int)node_ip_map.size(); i++) {
                if (nodeid == node_ip_map[i].first) {
                        return true;
                }
        }
        return false;
}


} //namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}

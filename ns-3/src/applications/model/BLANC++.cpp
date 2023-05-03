/* -*- Mode:C  ; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
 /*
  * Copyright 2012
  *
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
 
 #include "ns3/log.h"
 #include "ns3/ipv4-address.h"
 #include "ns3/ipv6-address.h"
 #include "ns3/address-utils.h"
 #include "ns3/nstime.h"
 #include "ns3/inet-socket-address.h"
 #include "ns3/inet6-socket-address.h"
 #include "ns3/simulator.h"
 #include "ns3/socket-factory.h"
 #include "ns3/uinteger.h"
 #include "ns3/string.h"
 #include "ns3/trace-source-accessor.h"
 #include "ns3/ipv4.h"
 #include "BLANC++.hpp"
 #define DEBUG1 0
 #define DEBUG2 0
 #define DEBUG3 0
 namespace ns3 {
 
 NS_LOG_COMPONENT_DEFINE ("ns3.BLANCpp");
 NS_OBJECT_ENSURE_REGISTERED (BLANCpp);
 
 TypeId
 BLANCpp::GetTypeId (void)
 {
   static TypeId tid = TypeId ("ns3::BLANCpp")
     .SetParent<Application> ()
     .AddConstructor<BLANCpp> ()
     .AddAttribute ("Port", "Port on which we listen for incoming connections.",
                    UintegerValue (7),
                    MakeUintegerAccessor (&BLANCpp::m_local_port),
                    MakeUintegerChecker<uint16_t> ())
     .AddAttribute ("PacketSize",
                   "The size of outbound packet, typically acknowledgement packets from server application. 536 not fragmented on 1500 MTU",
                   UintegerValue (536),
                   MakeUintegerAccessor (&BLANCpp::m_packet_size),
		   MakeUintegerChecker<uint32_t> ())
     .AddAttribute ("RouterHelper",
                   "True if application will run in Router helper mode.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&BLANCpp::m_route_helper),
                   MakeUintegerChecker<uint32_t> ())
     .AddAttribute ("Name",
                   "Name of node.",
                   StringValue ("0"),
                   MakeStringAccessor (&BLANCpp::m_name),
                   MakeStringChecker())     
     .AddTraceSource ("ReceivedPacketS",
                     "A packet has been received",
                     MakeTraceSourceAccessor (&BLANCpp::m_receivedPacket),
                     "ns3::BLANCpp::ReceivedPacketTraceCallback")
     .AddTraceSource ("SentPacketS",
                     "A packet has been sent",
                     MakeTraceSourceAccessor (&BLANCpp::m_sentPacket),
                     "ns3::BLANCpp::SentPacketTraceCallback")
     .AddTraceSource ("OnFindReply",
                     "A findReply packet has been received",
                     MakeTraceSourceAccessor (&BLANCpp::m_onFindReply),
                     "ns3::BLANCpp::OnFindReplyTraceCallback")
     .AddTraceSource ("OnHold",
                     "A hold packet has been recived",
                     MakeTraceSourceAccessor (&BLANCpp::m_onHold),
                     "ns3::BLANCpp::OnHoldTraceCallback")
     .AddTraceSource ("OnPay",
                     "A pay packet has been received",
                     MakeTraceSourceAccessor (&BLANCpp::m_onPay),
                     "ns3::BLANCpp::OnPayTraceCallback")
     .AddTraceSource ("OnPayPath",
                     "A pay packet has been received",
                     MakeTraceSourceAccessor (&BLANCpp::m_onPayPath),
                     "ns3::BLANCpp::OnPayPathTraceCallback")                     
     .AddTraceSource ("OnTx",
                     "A transaction has started",
                     MakeTraceSourceAccessor (&BLANCpp::m_onTx),
                     "ns3::BLANCpp::OnTxTraceCallback")  
     .AddTraceSource ("OnTxFail",
                     "A transaction has started",
                     MakeTraceSourceAccessor (&BLANCpp::m_onTxFail),
                     "ns3::BLANCpp::OnTxFailTraceCallback")   
     .AddTraceSource ("OnPathUpdate",
                     "A transaction has started",
                     MakeTraceSourceAccessor (&BLANCpp::m_onPathUpdate),
                     "ns3::BLANCpp::OnPathUpdateTraceCallback") 
     .AddTraceSource ("OnTxRetry",
                     "A transaction has started",
                     MakeTraceSourceAccessor (&BLANCpp::m_onTxRetry),
                     "ns3::BLANCpp::OnTxRetryTraceCallback")                                                                                     
   ;
   return tid;
 }
 
 
BLANCpp::BLANCpp ()
 {
   NS_LOG_FUNCTION_NOARGS ();
   m_socket = 0;
   m_running = false;
 }
 
BLANCpp::~BLANCpp()
 {
   NS_LOG_FUNCTION_NOARGS ();
   m_socket = 0;
 }
 
void
BLANCpp::DoDispose (void)
 {
   NS_LOG_FUNCTION_NOARGS ();
   Application::DoDispose ();
 }
 
void
BLANCpp::StartApplication (void)
 {
   NS_LOG_FUNCTION_NOARGS ();
 
   m_running = true;

   if (m_socket == 0)
     {
       TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
       m_socket = Socket::CreateSocket (GetNode (), tid);
       InetSocketAddress listenAddress = InetSocketAddress (Ipv4Address::GetAny (), m_peerPort);
       m_socket->Bind (listenAddress);
       m_socket->Listen();
     }
   m_socket->SetAcceptCallback (
         MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
         MakeCallback (&BLANCpp::HandleAccept, this));
   if (m_route_helper){
      Simulator::Schedule(Seconds(0.0), &BLANCpp::sendAdvertPacket, this);
   }

   checkTimeout();

 }
 
void
BLANCpp::StopApplication ()
 {
   NS_LOG_FUNCTION_NOARGS ();
 
   m_running = false;
 
   if (m_socket != 0)
     {
       m_socket->Close ();
       m_socket->SetAcceptCallback (
             MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
             MakeNullCallback<void, Ptr<Socket>, const Address &> () );
     }
 }
 
void
BLANCpp::ReceivePacket (Ptr<Socket> s)
 {
   NS_LOG_FUNCTION (this << s);
   Ptr<Packet> packet;
   Address from;
   while (packet = s->RecvFrom (from)) {
      if (packet->GetSize () > 0) {

	      //Get the first interface IP attached to this node (this is where socket is bound, true nodes that have only 1 IP)
    	   //Ptr<NetDevice> PtrNetDevice = PtrNode->GetDevice(0);
    	   Ptr <Node> PtrNode = this->GetNode();
    	   Ptr<Ipv4> ipv4 = PtrNode->GetObject<Ipv4> (); 
    	   Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0);  
    	   m_local_ip = iaddr.GetLocal (); 

     	   NS_LOG_INFO ("Server Received " << packet->GetSize () << " bytes from " << InetSocketAddress::ConvertFrom (from).GetIpv4 ()
		      << ":" << InetSocketAddress::ConvertFrom (from).GetPort ());

	      packet->RemoveAllPacketTags ();
     	   packet->RemoveAllByteTags ();
	      processPacket(packet, s);
	   } 
	
      //TODO: Alter
      if (false){ 
     	  NS_LOG_LOGIC ("Sending reply packet " << packet->GetSize ());
	  //Keep original packet to send to trace file and get correct size
          Ptr<Packet> packet_copy = Create<Packet> (m_packet_size);
     	  s->Send (packet_copy);
      }
          // Callback for received packet
    	  m_receivedPacket (GetNode()->GetId(), packet, from, m_local_port, 0, m_local_ip);
      
     }
 }
 
void BLANCpp::HandleAccept (Ptr<Socket> s, const Address& from)
 {
   NS_LOG_FUNCTION (this << s << from);
   s->SetRecvCallback (MakeCallback (&BLANCpp::ReceivePacket, this));
   s->SetCloseCallbacks(MakeCallback (&BLANCpp::HandleSuccessClose, this),
     MakeNullCallback<void, Ptr<Socket> > () );
 }
 
void BLANCpp::HandleSuccessClose(Ptr<Socket> s)
 {
   NS_LOG_FUNCTION (this << s);
   NS_LOG_LOGIC ("Client close received");
   s->Close();
   s->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > () );
   s->SetCloseCallbacks(MakeNullCallback<void, Ptr<Socket> > (),
       MakeNullCallback<void, Ptr<Socket> > () );
 }

//SECTION: Incoming Packet handling 
void 
BLANCpp::processPacket(Ptr<Packet> p, Ptr<Socket> s){
   blancHeader packetHeader;
   p->RemoveHeader(packetHeader);
   uint32_t pType = packetHeader.GetPacketType();
   uint32_t extraSize = p->GetSize() - packetHeader.GetPayloadSize();
   
 
   Ptr<Packet> pkt  = p->Copy();
   p->RemoveAtEnd(extraSize);
   	  
   switch(pType){
      case Find:
      case FindRecv:
      case 10:
         onFindPacket(p, packetHeader, s);
         break;
      case HoldReply:
	      onHoldReply(p, packetHeader);
         break;
      case Hold:
         onHoldPacket(p, packetHeader, s);
         break;
      case HoldRecv:
         onHoldRecvPacket(p, packetHeader, s);
         break;
      case Pay:
         onPayPacket(p, packetHeader, s);
         break;
      case PayReply:
         onPayReply(p, packetHeader);
         break;
      case Advert:
         onAdvertPacket(p, packetHeader, s);
         break;  
      case AdvertReply:
         onAdvertReply(p, packetHeader, s);
         break;            
      case Reg:
         onRegPacket(p, packetHeader, s);
         break;      
      case Nack:
         onNack(p, packetHeader, s);
      default:
         break;
   }

   if (extraSize > 0){
      pkt->RemoveAtStart( packetHeader.GetPayloadSize() );
      processPacket(pkt, s);
   }
}

void 
BLANCpp::onHoldPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s)
 {
   std::string payload = readPayload(p);
   //std::cout << payload <<"  payload "<<std::endl;

   std::vector<std::string> items = SplitString(payload, '|');
   uint32_t txID = std::stoi(items[0]); std::string path = items[1]; double amount = std::stod(items[2]);
   uint64_t hopMax = std::stoi(items[3]); double te1 = std::stod(items[4]), te2 = std::stod(items[5]);

   if(DEBUG2){
      std::cout << "Holds----- "<<m_name <<"  NodeID\n"<< txID <<" TxID\n" << path <<" path "<<
         amount <<" amount\n"<< hopMax <<" hop max\n"<< te1 <<" te1\n"<< te2 <<" te2\n"<<std::endl;
   }
   std::string dest = SplitString(path, ',')[0];
   std::string src = findSource(s), nextHop = "";

   bool needOverlap = false;
   
   if ( txidTable.find(txID) != txidTable.end() && txidTable[txID].pending != 0) {
      if(m_name == path && m_route_helper) {
         updateBCHold(txID); sendProceedPay(txID); sendHoldReply(txID); 
         txidTable[txID].src = s; txidTable[txID].nextHop = src; 
         txidTable[txID].dest = dest; txidTable[txID].replied = true;
         return;
      }
      needOverlap = checkOverlap(txID, dest, true);
      if (!needOverlap) return;
   }

   TransactionInfo* entry = &txidTable[txID];  
   if (needOverlap){
      TransactionInfo overlap;
      overlapTable[txID].push_back(overlap);
      entry = &overlapTable[txID].back();
   }

   entry->src = s;
   entry->nextHop = src; 
   entry->dest = dest;

   //Create next hold packet
   //Ptr<Packet> packet_copy = Create<Packet> (m_packet_size);
   //TODO: Create return setting
   if(!m_route_helper && m_name == dest){
      path = SplitString(path, ',', 1)[1];
      dest = SplitString(path, ',')[0];
   }
   else if(m_route_helper && m_name == dest){
      //Update next hop and Router Helper Destination
      sendHoldReply(txID); 
      if (SplitString(path, ',').size() == 1) {
         updateBCHold(txID);
         insertTimeout(txID, nextHop, payload, te1, te2, true);
         entry->replied = true;
         return;
      }

      //TODO add path via Onion encryption to protect
      path = SplitString(path, ',', 1)[1];
      dest = SplitString(path, ',')[0];
      if(RoutingTable.find(dest) == RoutingTable.end()){
         path = createPath(dest, amount) + path;
         dest = SplitString(path, ',')[0];
      }
      entry->nextDest = dest;
      entry->dest = dest;
      entry->onPath = true;
      updateRHRTableWeight(dest, amount);

      //Send out BC message, affirming hold.
      updateBCHold(txID);
   }

   std::string result = findNextHop(dest, amount, true);
   nextHop = SplitString(result, '|')[0];

   if (nextHop == "" ) {      
   //std::cout<<result<<"  "<<dest<<std::endl;
      sendNack(txID, te1, dest);
      return; 
   }  
   double hopCount = std::stod(SplitString(result, '|')[1]);   

   entry->pending = amount;

   blancHeader packetHeader;
   packetHeader.SetPacketType(Hold);

   payload = std::to_string(txID) + "|";
   payload += path + "|";
   payload += std::to_string(amount) + "|";
   payload += "5|";
   payload += std::to_string(te1) + "|" +std::to_string(te2);
   //std::cout<<"Sending to " <<nextHop<<std::endl;
   updatePathWeight(nextHop, amount, true);
   insertTimeout(txID, nextHop, payload, te1*hopCount, te2*hopCount, true);

   //Forward next hold packet
   forwardPacket(packetHeader, nextHop, payload);
 }

void 
BLANCpp::onFindPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s){

   
}

void 
BLANCpp::onHoldRecvPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s)
 { 
   std::string payload = readPayload(p);

   //std::cout << payload <<"  payload "<<std::endl;

   std::vector<std::string> items = SplitString(payload, '|');
   uint32_t txID = std::stoi(items[0]); std::string path = items[1]; double amount = std::stod(items[2]);
   uint64_t hopMax = std::stoi(items[3]); double te1 = std::stoi(items[4]); double te2 = std::stoi(items[5]);

   if(DEBUG2){
      std::cout << "HoldR----- "<<m_name <<" NodeID\n"<<txID<<" TxID\n"<<path <<" path\n" << 
         amount <<"  amount\n"<<hopMax<<" hop max\n"<<te1<<" te1\n"<<te2<<" te2"<<
         ns3::Simulator::Now().GetSeconds()<<"\n"<<std::endl;
   }

   std::string dest = SplitString(path, ',')[0];
   if ( txidTable.find(txID) != txidTable.end() && txidTable[txID].pending != 0) {
      if(!checkOverlap(txID, dest, false)){
         return;
      }
   }

   if(overlapTable.find(txID) == overlapTable.end()){
      overlapTable[txID] = {};
   }
   std::string nextHop = findSource(s);
   txidTable[txID].src = s;
   txidTable[txID].dest = "RH2";   

   txidTable[txID].pending = amount;

   //Create next hold packet
   if(!m_route_helper || m_name != dest){  

      blancHeader packetHeader;
      packetHeader.SetPacketType(HoldRecv);

      payload = std::to_string(txID) + "|";
      payload += path + "|";
      payload += std::to_string(amount) + "|";
      payload += "5|";
      payload += std::to_string(te1) + "|" +std::to_string(te2);
      
      std::string origin = nextHop;
      std::string result =  findNextHop(dest, amount, true);
      nextHop = SplitString(result, '|')[0];
      if (nextHop == "") {
         std::cout<<"No where to go\n";
         sendNack(txID, te1, dest);
         return; 
      }  
      double hopCount = std::stod(SplitString(result, '|')[1]);       
      txidTable[txID].nextHop = nextHop;       
    
      updatePathWeight(origin, amount, false);
      insertTimeout(txID, origin, payload, te1*hopCount, te2*hopCount, false);    

      //Forward next hold packet
      forwardPacket(packetHeader, nextHop, payload);
   }
   else if (m_route_helper && m_name == dest){
      //Send out BC message, affirming hold. 
      updateBCHoldRecv(txID);
      //m_onHold(std::stoi(m_name), txID, true);
      txidTable[txID].onPath = true;
      txidTable[txID].replied = true;
      insertTimeout(txID, nextHop, payload, te1, te2, false);    

      sendHoldReply(txID);
   }
 }

void 
BLANCpp::onHoldReply(Ptr<Packet> p, blancHeader ph)
 {

   std::string payload = readPayload(p);

   std::vector<std::string> items = SplitString(payload, '|');
   uint32_t txID = std::stoi(items[0]);

   std::string src = "";//REPLACE

   if (items.size() == 1){
      if (txidTable[txID].replied) return;
      if(DEBUG2) std::cout << "Hold reply ";
   }
   else if(DEBUG2)
      std::cout << "ProceedPay ";      
      
   if(DEBUG2) std::cout<< m_name  << " "<<ns3::Simulator::Now().GetSeconds() <<"\n"<<txID <<"  TxID\n"<<std::endl;

   txidTable[txID].replied = true;

   blancHeader packetHeader;
   packetHeader.SetPacketType(HoldReply);

   if (txidTable[txID].src != NULL ){
      //Forward next pay packet
      forwardPacket(packetHeader, txidTable[txID].src, payload);
   }

   else if (m_txID == txID && items.size() > 1){
      m_onHold(std::stoi(m_name), txID, !m_payer);	 
   }
 }

void 
BLANCpp::onPayReply(Ptr<Packet> p, blancHeader ph){

   std::string payload = readPayload(p);


   std::vector<std::string> items = SplitString(payload, '|');
   uint32_t txID = std::stoi(items[0]);

   if (txidTable.find(txID) == txidTable.end()) return;

   std::string src = "";//REPLACE
   if(DEBUG3)
      std::cout << "Pay reply "<< m_name  <<"\n"<<txID <<"  TxID\n"<<std::endl;

   txidTable[txID].payReplied = true;
   blancHeader packetHeader;
   packetHeader.SetPacketType(PayReply);

   //if (txidTable[txID].src != NULL ){
if (srcTrail[txID].size() > 0 ){      
      //Forward next pay packet
      //forwardPacket(packetHeader, txidTable[txID].src, payload);
      forwardPacket(packetHeader, srcTrail[txID].front(), payload);
      srcTrail[txID].erase(srcTrail[txID].begin());
      //txidTable[txID].src = NULL;
      if (srcTrail[txID].size() == 0 ) txidTable.erase(txID);
   }
}

void 
BLANCpp::onPayPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s)
 {

   std::string payload = readPayload(p);

   std::vector<std::string> items = SplitString(payload, '|');
   uint32_t txID = std::stoi(items[0]);
   double amount = std::stod(items[1]);
   uint64_t timeout = std::stoi(items[2]);
      std::string src = "";//REPLACE
   txidTable[txID].src = s;
   srcTrail[txID].push_back(s);
   if(DEBUG3){   
      std::cout << "Pay packet------ "<<m_name <<" NodeID "<<std::endl;
      std::cout << txID <<"  TXiD "<<std::endl;
      std::cout << amount <<"  amount\n"<<std::endl;
   }
   m_onPayPath(std::stoi(m_name), txID);

   if ( txidTable.find(txID) == txidTable.end() || txidTable[txID].pending == 0) return;


   if(m_txID == txID && m_payer){
      //Update next hop and Router Helper Destination
      sendPayReply(txID);
      m_onPay(std::stoi(m_name), txID);
      //Send out BC message, affirming hold.
      updateBCPay(txID);  

     return; 
   }


   //Create next pay packet

   std::string nextHop = txidTable[txID].nextHop;
   if( nextHop == "" && overlapTable.find(txID) != overlapTable.end() && overlapTable[txID].size() > 0){
      txidTable[txID] = overlapTable[txID].back();
      txidTable[txID].replied = true;
      overlapTable[txID].pop_back();
      nextHop = txidTable[txID].nextHop;
   }   

   //Forward next pay packet
   blancHeader packetHeader;
   packetHeader.SetPacketType(Pay);

   forwardPacket(packetHeader, nextHop, payload);

   //TODO verify with algo
   if (m_route_helper && txidTable[txID].onPath ){
      //Send out BC message, affirming hold. 
      sendPayReply(txID);
   }

   if(overlapTable.find(txID) != overlapTable.end() && overlapTable[txID].size() > 0){
      txidTable[txID] = overlapTable[txID].back();
      txidTable[txID].replied = true;
      overlapTable[txID].pop_back();
   }
 }

void 
BLANCpp::onAdvertPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s)
{
   std::string payload = readPayload(p);

   if(DEBUG1) std::cout << m_name<<" "<<payload <<"  payload "<<std::endl;

   std::vector<std::string> items = SplitString(payload, '|');
   std::string src = items[0];
   double sendMax = std::stod(items[1]);
   double recvMax = std::stod(items[2]);
   double hopCount = std::stod(items[3])+1;
   double hopMax = std::stod(items[4]);
   double band = 0;
   std::string route = "";
   if(method2) {
      band = std::stod(items[5]);
      route = items[6];   
   }

   //uint64_t timestamp = std::stoi(items[2]);

   if( m_name == src)
      return;
   std::string nextHop = findSource(s);

   RoutingEntry* entry = NULL;
   int minHop = 10000;
   if (RoutingTable.find(src) == RoutingTable.end()){
      RoutingTable[src].push_back(RoutingEntry());
      entry = &RoutingTable[src][0];
      entry->nextHop = nextHop;
   }
   else {
      for (int i = 0; i < RoutingTable[src].size(); i++){
         if (RoutingTable[src][i].nextHop == nextHop){
            entry = &RoutingTable[src][i];
            if(entry->hopCount <= hopCount){
               return;
            }
         }
         if(RoutingTable[src][i].hopCount < minHop)
            minHop = RoutingTable[src][i].hopCount;
      }
      if (entry == NULL){
         RoutingTable[src].push_back(RoutingEntry());
         entry = &RoutingTable[src][RoutingTable[src].size()-1];
         entry->nextHop = nextHop;
      }
   }

   if(DEBUG1 ){
      std::cout << "*****Advertisment packet*****\n" <<m_name <<"  NodeID "<<ns3::Simulator::Now().GetSeconds()<< std::endl;
      std::cout << src <<"  RH-Advert Source "<<std::endl;
      std::cout << nextHop <<"  Recieved from "<<std::endl;
      std::cout << sendMax <<"  send max"<<std::endl;
      std::cout << recvMax <<"  recv max"<<std::endl;
      std::cout << hopCount <<"  hop count"<<std::endl;
      std::cout << hopMax <<"  hop max\n"<<std::endl;
      if(method2) {
      std::cout << band <<"  band"<<std::endl;
      std::cout << route <<"  route\n"<<std::endl;

         band = std::stod(items[5]);
         route = items[6];   
      }      
    }

   /*if (m_route_helper){
      std::cout << " Routing helper received\n"<<std::endl;
      if (reachable.find(src) == reachable.end())
         reachable.emplace(src);
   }*/


   //hopCount++;

   entry->sendMax = neighborTable[nextHop].cost;
   if (sendMax < entry->sendMax)
      entry->sendMax = sendMax;
   entry->recvMax = recvMax;
   entry->hopCount = hopCount;

   //entry->expireTime = ;
   //TODO: Add signature verification
   //TODO: Add keyGen
 
   if (method2 && hopCount >= (hopMax-band) &&  hopCount <= hopMax && !m_route_helper){
      sendAdvertReply(items);
   }
   if (hopCount >= hopMax || hopCount >= minHop)
      return;  
   for (auto it = neighborTable.begin(); it != neighborTable.end(); it++){
      std::string payload = src;
      payload += "|"+ std::to_string(entry->sendMax) + "|";
      payload += std::to_string(std::min(entry->recvMax, neighborTable[it->first].cost));
      payload += "|" + std::to_string(hopCount) + "|";
      payload += std::to_string(hopMax) + "|";
      if(method2){
         payload += std::to_string(band) + "|";
         payload += route + "," + m_name + "|";      
      }
      //TODO: Add timestamp
      payload += "now";
      
      blancHeader packetHeader;
      packetHeader.SetPacketType(Advert);
      //TODO: Add signature
      if(it->first != nextHop && SplitString(it->first, '|').size()==1){
         forwardPacket(packetHeader, it->first, payload);
         if (DEBUG1 ) std::cout<<m_name<<" sending to " <<it->first<<" from "<<nextHop<<std::endl;
      }
   }
}

void 
BLANCpp::onAdvertReply(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s)
{
   std::string payload = readPayload(p);
   std::vector<std::string> items = SplitString(payload, '|');
   //std::cout << payload <<"  payload "<<std::endl;
   if(!method2) return;

   std::string src = items[0];
   double sendMax = std::stod(items[1]);
   double recvMax = std::stod(items[2]);
   double hopCount = std::stod(items[3])+1;
   std::string route = items[4];
   std::string nextRoute = SplitString(route, ',')[ SplitString(route, ',').size()-1 ];

   //uint64_t timestamp = std::stoi(items[2]);
    
   if(DEBUG2 || src == "143"){ 
      std::cout << "Advert Reply------ "<<m_name <<"  NodeID "<<std::endl;
      std::cout << src <<" Nonce "<<std::endl;
      std::cout << sendMax <<" send max"<<std::endl;
      std::cout << recvMax <<" recv max"<<std::endl;
      std::cout << route <<" route"<<std::endl;
      std::cout << hopCount <<" hop count\n"<<std::endl;
   }

   std::string nextHop = findSource(s);
   RoutingEntry* entry = NULL;
   int minHop = 10000;
   if (RoutingTable.find(src) == RoutingTable.end()){
      RoutingTable[src].push_back(RoutingEntry());
      entry = &RoutingTable[src][0];
      entry->nextHop = nextHop;
   }
   else {
      /*for (int i = 0; i < RoutingTable[src].size(); i++){
         if (RoutingTable[src][i].nextHop == nextHop){
            entry = &RoutingTable[src][i];
            if(entry->hopCount <= hopCount){
             //  return;
            }
         }
         if(RoutingTable[src][i].hopCount < minHop)
            minHop = RoutingTable[src][i].hopCount;
      }*/
      if (entry == NULL){
         RoutingTable[src].push_back(RoutingEntry());
         entry = &RoutingTable[src][RoutingTable[src].size()-1];
         entry->nextHop = nextHop;
      }
   }
   //std::cout<<"Gonna go to "<<nextRoute<<"  "<< ns3::Simulator::Now().GetSeconds()<<std::endl;
   entry->sendMax = std::min(sendMax, neighborTable[entry->nextHop].cost);
   entry->recvMax = recvMax;
   entry->hopCount = hopCount;
   entry->nonce = true;
   //entry->expireTime = ;
   //TODO: Add signature verification
   //TODO: Add keyGen

   if (nextRoute.size()>0){
      std::string payload = src;
      payload += "|"+ std::to_string(entry->sendMax) + "|";
      payload += std::to_string(std::min(entry->recvMax, neighborTable[nextRoute].cost));
      payload += "|" + std::to_string(hopCount) + "|";
      payload += route.substr(0, (route.size() - nextRoute.size()-1)  ) + "|";
      //TODO: Add timestamp
      payload += "now";
      
      blancHeader packetHeader;
      packetHeader.SetPacketType(AdvertReply);
      //TODO: Add signature

      forwardPacket(packetHeader, nextRoute, payload);
   }
}

void 
BLANCpp::onRegPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s){
   std::string payload = readPayload(p);

   if(DEBUG1) std::cout<<"Node "<<m_name<<" registering socket for "<<payload<<std::endl;
    
   if (neighborTable[payload].socket == 0)
      neighborTable[payload].socket = s;
   else 
      neighborTable[payload+"|T"].socket = s;

}

void 
BLANCpp::forwardPacket(blancHeader packetHeader, std::string nextHop, std::string payload)
{

  Ptr<Packet> p;
  p = Create<Packet> (reinterpret_cast<const uint8_t*> (payload.c_str()), payload.size());  
  //p = Create<Packet> (100);
    packetHeader.setPayloadSize(payload.size());

  uint64_t m_bytesSent = 100;
  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well

  //Add Blanc information to packet header and send
  Address peerAddress = neighborTable[nextHop].address;
  Ptr<Socket> socket = getSocket(nextHop);

  p->AddHeader(packetHeader);

  double seconds = 0;
  if(lastSent >= Simulator::Now().GetSeconds())
      seconds = 0.001 + (lastSent - Simulator::Now().GetSeconds());
   lastSent = Simulator::Now().GetSeconds() + seconds;

  Simulator::Schedule(Seconds(seconds), &BLANCpp::send, this, socket, p);

  //socket->Send (p);
  NS_LOG_INFO ("Sent " << 100 << " bytes to " << peerAddress << ":" << m_peerPort);

  //Increase sequence number for next packet
  m_seq = m_seq + 1;
}

void
BLANCpp::forwardPacket(blancHeader packetHeader, Ptr<Socket> socket, std::string payload)
{

  Ptr<Packet> p;
  p = Create<Packet> (reinterpret_cast<const uint8_t*> (payload.c_str()), payload.size());
  packetHeader.setPayloadSize(payload.size());
  //p = Create<Packet> (100);
  uint64_t m_bytesSent = 100;
  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  //m_txTrace (p);

  //Add Blanc information to packet header and send

  p->AddHeader(packetHeader);

  double seconds = 0;
  if(lastSent >= Simulator::Now().GetSeconds())
      seconds = 0.001 + (lastSent - Simulator::Now().GetSeconds());
   lastSent = Simulator::Now().GetSeconds() + seconds;

  Simulator::Schedule(Seconds(seconds), &BLANCpp::send, this, socket, p);
  //socket->Send (p);

  //Increase sequence number for next packet
  m_seq = m_seq + 1;
}

void 
BLANCpp::setNeighborCredit(std::string name, double amount){
   neighborTable[name].cost = amount;
}

void
BLANCpp::setNeighbor(std::string name, Ipv4Address address){
   neighborTable[name].address = address;
}

Ptr<Socket> 
BLANCpp::getSocket(std::string dest)
{
 Ptr<Socket> socket = neighborTable[dest].socket;
	
 if (socket == 0)
 {
    TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");   
    socket = Socket::CreateSocket (GetNode (), tid);
    socket->Bind();
    socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom( neighborTable[dest].address ), m_peerPort));

    socket->SetRecvCallback (MakeCallback (&BLANCpp::ReceivePacket, this));
    
    std::string payload = m_name;     
    blancHeader packetHeader;
    packetHeader.SetPacketType(Reg);
    neighborTable[dest].socket = socket;

    forwardPacket(packetHeader, socket, payload);
 }

 return socket;
}

bool
BLANCpp::hasHoldRecv(uint32_t txID)
{
   uint32_t txIDprime = txidTable[txID].nextTxID;
      std::cout << txIDprime<< ' ' << txID <<"  NodeID "<<std::endl;

   if ( txidTable.find(txIDprime) != txidTable.end() ){
      //txidTable.erase(txIDprime);
      //txidTable.erase(txID);
      return true;
   }
   return false;
}

//Transaction Functions
void 
BLANCpp::startTransaction(uint32_t txID, uint32_t secret, std::vector<std::string> peerlist, bool payer, double amount)
{
   //Determine number of segments and create the needed txIDs.

   m_payer = payer;
   m_txID = txID;
   txidTable[txID].nextDest = peerlist[0];
   txidTable[txID].dest = peerlist[0];
   std::string path = m_name+"|" +peerlist[0];
   if (payer) {
      nextRH = peerlist[1];
      path += "|" +peerlist[1];
   }
   else    txidTable[txID].dest = "RH2";
   m_onTx(path, txID, payer);
   sendHoldPacket(txID, amount);
   TiP = true;
}

void
BLANCpp::reset(uint32_t txID)
{
   //std::cout<<"Good bye tx "<<m_name<<std::endl;
   //Determine number of segments and create the needed txIDs.
   m_amount = 0;
   m_payer = false;
   txidTable.erase(txID);
   m_txID = 0;
   nextRH = "";
   TiP = false;
}

uint32_t
BLANCpp::createTxID(uint32_t txID){
   return txID + rand()%10;
}

void 
BLANCpp::sendAdvertPacket()
{
   AdvertSetup();

   //TODO: Add as a variable
   int hopMax = 6;
   if(method2) hopMax = 4;
   int band = 3;
   for (auto it = neighborTable.begin(); it != neighborTable.end(); it++){
      if(SplitString(it->first, '|').size()!=1) continue;
      std::string payload = m_name;
      payload += "|10000000000|";
      payload += std::to_string(neighborTable[it->first].cost) + "|0|";
      payload += std::to_string(hopMax) + "|";
      if(method2) {
         payload += std::to_string(band) + "|";
         payload += m_name + "|";      
      }
      //TODO: Add timestamp
      payload += "now";
      
      blancHeader packetHeader;
      packetHeader.SetPacketType(Advert);
      //TODO: Add signature
      if (DEBUG1) std::cout<<"Node "<<m_name<<" sending to"<<it->first<<std::endl;
      forwardPacket(packetHeader, it->first, payload);
   }
}

void 
BLANCpp::sendAdvertReply(std::vector<std::string> items)
{
   if(!method2) return;
   AdvertSetup();

   std::string recvMax = items[1];
   std::string sendMax = items[2];
   std::string route = items[6];
   std::string nextRoute = SplitString(route, ',')[ SplitString(route, ',').size()-1 ];

   if (nextRoute.size()>0){
      std::string payload = m_name;
      payload += "|100000000000|";
      payload += std::to_string(neighborTable[nextRoute].cost) + "|0|";
      payload += route.substr(0, (route.size() - nextRoute.size()-1)  ) + "|";

      //TODO: Add timestamp
      payload += "now";

      blancHeader packetHeader;
      packetHeader.SetPacketType(AdvertReply);
      //TODO: Add signature

      forwardPacket(packetHeader, nextRoute, payload);
   }
}

void 
BLANCpp::sendRegPacket(std::string dest){

}

void
BLANCpp::sendHoldPacket(uint32_t txID, double amount)
{
   //TODO Add preimage to packet and check it at onHold
   //TODO add path via Onion encryption to protect
   m_amount = amount;
   std::string dest = txidTable[txID].nextDest;
   txidTable[txID].pending = amount;
   std::string nextHop, result = findNextHop(dest, amount, true);
   nextHop = SplitString(result, '|')[0];
   if (nextHop == "") {
         std::cout<<"Node "<<m_name<<" to RH "<< txidTable[txID].nextDest <<" failed sending out Hold packet\n";
      reset(txID);
      return; 
   }
   double hopCount = std::stod(SplitString(result, '|')[1]);

   std::string te1 = "30", te2 = "100"; //TODO:: replace with class variable

      if(DEBUG1) std::cout<<"Node "<<m_name<<" "<< nextHop <<" sending out Hold packet\n";

   blancHeader packetHeader;
   packetHeader.SetPacketType(HoldRecv);

   if(m_payer){
      packetHeader.SetPacketType(Hold);
      updatePathWeight(nextHop, amount, true);
      m_txID = txID;
   }
   else {
      txidTable[txID].nextHop = nextHop;
   if(overlapTable.find(txID) == overlapTable.end()){
      overlapTable[txID] = {};
   }      
   }

   std::string payload = std::to_string(txID) + "|";
   payload += dest;
   if(m_payer)
      payload += "," + nextRH;//Fix this
   payload += "|";
   payload += std::to_string(amount) + "|";
   payload += "5|";
   payload += te1 + "|" + te2;


   int timeout = 5;
   insertTimeout(txID, nextHop, payload, std::stod(te1)*hopCount, std::stod(te2)*hopCount, true);

   forwardPacket(packetHeader, nextHop, payload);
   if (DEBUG2) std::cout<<"Hold phase\n\n\n";
}

void 
BLANCpp::sendHoldReply(uint32_t txID)
{
   blancHeader packetHeader;
   packetHeader.SetPacketType(HoldReply);

   std::string payload = std::to_string(txID);
   forwardPacket(packetHeader, txidTable[txID].src, payload);
}

void 
BLANCpp::sendProceedPay(uint32_t txID)
{
   blancHeader packetHeader;
   packetHeader.SetPacketType(HoldReply);

   std::string payload = std::to_string(txID)+"|ProceedPay";
   forwardPacket(packetHeader, txidTable[txID].src, payload);
}

void 
BLANCpp::sendPayReply(uint32_t txID)
{
   //TODO Add preimage to packet and check it at onHold
   //TODO add destinations
   if(DEBUG3) std::cout<<"Pay Reply: "<<m_name<<" "<<txidTable[txID].dest<<"\n\n";

   blancHeader packetHeader;
   packetHeader.SetPacketType(PayReply);

   std::string payload = std::to_string(txID);

   forwardPacket(packetHeader, txidTable[txID].src, payload);
   //txidTable[txID].src = NULL;
}

void 
BLANCpp::sendPayPacket(uint32_t txID)
{
   if(DEBUG3) std::cout<<"Pay phase: "<<m_name<<" "<<txidTable[txID].nextHop<<"\n\n\n";

   std::string nextHop = txidTable[txID].nextHop;
   //txidTable.erase(txID);

   blancHeader packetHeader;
   packetHeader.SetPacketType(Pay);
   std::string payload = std::to_string(txID) + "|";
   payload += std::to_string(m_amount) + "|";
   payload += "5";

   forwardPacket(packetHeader, nextHop, payload);
   if(overlapTable.find(txID) != overlapTable.end() && overlapTable[txID].size() > 0){
      txidTable[txID] = overlapTable[txID].back();
      txidTable[txID].replied = true;
      overlapTable[txID].pop_back();
   }   
}

void 
BLANCpp::sendNack(uint32_t txID, double te1, std::string dest)
{
   blancHeader packetHeader;
   packetHeader.SetPacketType(Nack);

   std::string payload = std::to_string(txID)+"|No Route|"+std::to_string(te1)+"|"+dest;
   forwardPacket(packetHeader, txidTable[txID].src, payload);
   txidTable.erase(txID);
}

void 
BLANCpp::insertTimeout(uint32_t txID, std::string src, std::string payload, double te1, double te2, bool sender){
   TransactionInfo* entry = NULL;
   if (txidTable[txID].payload == "" && txidTable[txID].dest != "") { 
      entry = &txidTable[txID];
   }
   
   else if(overlapTable.find(txID) != overlapTable.end()){
      for (auto each = overlapTable[txID].begin(); each != overlapTable[txID].end(); each++){
         if(each->payload == "") {
            entry = &(*each);
         }
      }     
   }   
   //std::cout<<txID<<"  " <<m_name<<"   "<<txidTable[txID].payload<<std::endl;
   entry->payload = payload; 
   entry->sender = sender;
   entry->te1 = ns3::Simulator::Now().GetSeconds() + te1;
   entry->te2 = ns3::Simulator::Now().GetSeconds() + te2;
}

void 
BLANCpp::checkTimeout(){
   for (auto each = txidTable.begin(); each != txidTable.end(); each++ ){
      if (each->second.te1 <= ns3::Simulator::Now().GetSeconds() && !each->second.replied ) {
         //std::cout<<m_name<<"  "<<each->second.te1<<"  Bruh\n";
         //checkTe1(each->first, each->second.nextHop, each->second.payload, 5, each->second.sender);
      }
      if (each->second.te2 <= ns3::Simulator::Now().GetSeconds() && !each->second.payReplied) {
         //std::cout<<m_name<<"  "<<each->second.te1<<"  .... really?\n";
         //checkTe2(each->first, each->second.nextHop, each->second.sender);
      }
   }
   ns3::Simulator::Schedule(Seconds(0.001), &BLANCpp::checkTimeout, this);
}

void 
BLANCpp::checkTe1(uint32_t txID, std::string src, std::string payload, double te1, bool sender){
   if (txidTable[txID].replied) return;

   updatePathWeight(src, (-1 * txidTable[txID].pending), sender);
   updateRoutingTable(src, txidTable[txID].pending, sender);
   if(txidTable[txID].onPath){
      updateRHRTableWeight(txidTable[txID].nextDest, (-1 * txidTable[txID].pending));
      updateRHRTable(txidTable[txID].nextDest, txidTable[txID].pending);
   }

   std::string nextHop = findNextHop(txidTable[txID].nextDest, txidTable[txID].pending, sender); 
   
   if (nextHop == "" || txidTable[txID].retries >= m_maxRetires) {
      sendNack(txID, te1, txidTable[txID].nextDest);
      return; 
   }
   txidTable[txID].retries++;
   txidTable[txID].nextHop = nextHop; 

   blancHeader packetHeader;
   packetHeader.SetPacketType(Hold);
 
   updatePathWeight(nextHop, txidTable[txID].pending, sender);
   txidTable[txID].te1 = ns3::Simulator::Now().GetSeconds() + te1;

   //Forward next hold packet
   forwardPacket(packetHeader, nextHop, payload);
}

void 
BLANCpp::onNack(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s){
   std::string payload = readPayload(p);

   std::cout << payload <<"  payload "<<std::endl;

   std::vector<std::string> items = SplitString(payload, '|');
   uint32_t txID = std::stoi(items[0]);
   double te1 = std::stoi(items[2]);
   std::string dest = items[3];


   std::cout << "Nack----- "<<m_name <<"  NodeID "<<std::endl;

   if (txidTable[txID].replied) return;
   std::cout << txID <<"  TxID\n"<<std::endl;
   std::cout << dest <<"  Dest\n"<<std::endl;

   std::string src = "";
   txidTable[txID].sender;
   for (auto it = neighborTable.begin(); it != neighborTable.end(); it++){
      if (it->second.socket == s)
         src = it->first; 
   }

   //TODO update from nack values
   updatePathWeight(src, (-1 * txidTable[txID].pending), txidTable[txID].sender);
   updateRoutingTable(src, txidTable[txID].pending, txidTable[txID].sender);
   if(txidTable[txID].onPath){
      updateRHRTableWeight(dest, (-1 * txidTable[txID].pending));
      updateRHRTable(dest, txidTable[txID].pending);
   }

   std::string result = findNextHop(dest, txidTable[txID].pending, txidTable[txID].sender); 
   std::string nextHop = SplitString(result, '|')[0];
         std::cout<<result<< "  nextDest: " << txidTable[txID].nextDest << " dest: " << txidTable[txID].dest <<std::endl;;

   if (nextHop == "" || txidTable[txID].retries >= m_maxRetires) {
      if (dest == m_name) {
         dest = txidTable[txID].dest;
      }
      if(m_txID == txID) {
         m_onTxFail(txID);
         reset(txID);
      }
      else {
         sendNack(txID, te1, dest);
         txidTable.erase(txID);
      }
      return; 
   }

   std::cout<<nextHop<<" changed from "<<src<<" "<<txidTable[txID].dest<<std::endl;

   txidTable[txID].retries++;
   m_onTxRetry(txID);
   //txidTable[txID].nextHop = nextHop; 

   blancHeader packetHeader;
   packetHeader.SetPacketType(Hold);
 
   updatePathWeight(nextHop, txidTable[txID].pending, txidTable[txID].sender);
   txidTable[txID].te1 = ns3::Simulator::Now().GetSeconds() + te1;
     std::cout<<txidTable[txID].dest<<std::endl;

   if (txidTable[txID].dest == dest) { 
      payload = txidTable[txID].payload;
   }
   
   else if(overlapTable.find(txID) != overlapTable.end()){
      for (auto each = overlapTable[txID].begin(); each != overlapTable[txID].end(); each++){
         if(each->dest == dest) {
            payload = each->payload;
         }
     //std::cout<<each->dest<<" dest "<<dest<<std::endl;

      }     
   } 

     std::cout<<payload<<" dddd "<<dest<<std::endl;

   //Forward next hold packet
   forwardPacket(packetHeader, nextHop, payload);
}

void 
BLANCpp::checkTe2(uint32_t txID, std::string src, bool sender){
   if (txidTable.find(txID) == txidTable.end() ) return;

   updatePathWeight(src, (-1 * txidTable[txID].pending), sender);
   txidTable.erase(txID);
}

void 
BLANCpp::send(Ptr<Socket> s, Ptr<Packet> p){
   if(m_name == "3") {
     // std::cout<<"Lets go "<<ns3::Simulator::Now()<<std::endl;

Ptr<Packet> pkt  = p->Copy();   
  blancHeader packetHeader;
   pkt->RemoveHeader(packetHeader);
   uint32_t pType = packetHeader.GetPacketType();   
    uint8_t buffer[p->GetSize()];
   pkt->CopyData (buffer,  pkt->GetSize());

   std::ostringstream convert;
   for (int a = 0; a < pkt->GetSize(); a++) {
      convert << (char)buffer[a];
   }


   std::string payload = convert.str();
   //std::cout << pType <<"  "<<payload <<"  payload "<<std::endl;

   }
   s->Send(p);
};

std::string 
BLANCpp::readPayload(Ptr<Packet> p){
   uint8_t buffer[p->GetSize()];
   p->CopyData (buffer,  p->GetSize());

   std::ostringstream convert;
   for (int a = 0; a < p->GetSize(); a++) {
      convert << (char)buffer[a];
   }
   return convert.str();
  }

std::string 
BLANCpp::findSource(Ptr<Socket> s){
   std::string src = "";
   for (auto it = neighborTable.begin(); it != neighborTable.end(); it++){
      if (it->second.socket == s)
         src = it->first; 
   }
   if(SplitString(src, '|').size()>1) {
      src = SplitString(src, '|')[0];
   }
   return src;
}

bool
BLANCpp::checkOverlap(uint32_t txID, std::string dest, bool sender){
   if (txidTable[txID].dest == dest) { 
      return false;
   }
   
   if(overlapTable.find(txID) == overlapTable.end()){
      TransactionInfo tailEntry = txidTable[txID];
      overlapTable[txID].push_back(tailEntry);
      txidTable[txID].dest = "";
      txidTable[txID].nextHop = "";
   }
   if (sender){
      //Ensure this is not a duplicate by looping through overlap table entry.
      for (auto each = overlapTable[txID].begin(); each != overlapTable[txID].end(); each++){
         if(each->dest == dest) return false;
      }      
   }
   return true;
  }

std::string 
BLANCpp::getRH(){
   std::string RH = "";
   int minHops = 10000;
   for (auto i = RoutingTable.begin(); i != RoutingTable.end(); i++){
      double sendMax = 0, recvMax = 0, hops = 10000;
      for (auto each = i->second.begin(); each != i->second.end(); each++){
         if (each->nonce) continue;
         if(sendMax < each->sendMax) sendMax = each->sendMax;
         if(recvMax < each->recvMax) recvMax = each->recvMax;
         if(hops > each->hopCount) hops = each->hopCount;
      }
      if (recvMax > 0 && sendMax > 0 && hops < minHops ) {
         minHops = hops;
         RH =  i->first;
      }
   }
   //std::cout<<m_name<<":: "<<RH<<"  Hops:" <<minHops<<std::endl;
   return RH;
}

std::vector<std::string> 
BLANCpp::getReachableRHs(){
   std::vector<std::string> RHs;
   for (auto i = RoutingTable.begin(); i != RoutingTable.end(); i++){
      std::string entry = i->first + "|";
      double sendMax = 0, recvMax = 0;
      bool nonce = false;
      for (auto each = i->second.begin(); each != i->second.end(); each++){
         if(each->nonce) nonce = true;
         if(sendMax < each->sendMax) sendMax = each->sendMax;
         if(recvMax < each->recvMax) recvMax = each->recvMax;
      }
      if(nonce) entry = "N"+ entry;
      entry += std::to_string(sendMax) + "|" + std::to_string(recvMax);
      //std::cout<<"Node " <<m_name<<" "<<i->first <<" creating table...\n";
      RHs.push_back(entry);
   }
   return RHs;
}

void 
BLANCpp::setRHTable(std::unordered_map<std::string, std::vector<std::string>> RHlist){
   std::cout<<"Node " <<m_name<<" creating table...\n";
   for (auto i = RHlist.begin(); i != RHlist.end(); i++){
      if (RoutingTable.find(i->first) != RoutingTable.end() || i->first == m_name) {
         if (DEBUG1) std::cout<<"\tReaches routing helper " <<i->first<<" directly\n";
         continue;
      }
      if (DEBUG1) std::cout<<"\tFor router helper " <<i->first<<".\n";
      for (auto each = i->second.begin(); each != i->second.end(); each++){
         RHRoutingEntry entry;
         bool nonce = false;
         entry.path = SplitString(*each, '|')[0];

         if(entry.path[0] == 'N' ) {
            nonce = true;
            entry.path = entry.path.substr(1);
         }
         entry.maxSend =  std::stod(SplitString(*each, '|')[1]);
         entry.maxRecv = std::stod(SplitString(*each, '|')[2]);

         if(RoutingTable.find(entry.path) != RoutingTable.end()){
            entry.complete = true;
            if(nonce)
               NonceTable[i->first].push_back(entry.path);
         }
         else if (nonce) continue;
         RHRoutingTable[i->first].push_back(entry);
         std::cout<<"\t\tGo to " <<entry.path<<" with sizes " <<entry.maxSend<<" and " <<entry.maxRecv<<".\n";
      }
   }
}

std::vector<std::string>  
BLANCpp::matchUpNonces(std::unordered_map<std::string, std::vector<std::string>> RHlist){
   for (auto i = RHlist.begin(); i != RHlist.end(); i++){
      if (RoutingTable.find(i->first) != RoutingTable.end() || i->first == m_name) {
         continue;
      }
      for (auto each = i->second.begin(); each != i->second.end(); each++){
         bool nonce = false;
         std::string path = SplitString(*each, '|')[0];
         std::string send =  SplitString(*each, '|')[1];
         std::string recv = SplitString(*each, '|')[2];
         if(path[0] == 'N' ) {
            nonce = true;
            path = path.substr(1);
         }
         if(RoutingTable.find(path) != RoutingTable.end()){
            if(nonce) {
               std::string rh = i->first +"|" +send + "|" + recv;
               RHlist[m_name].push_back(rh);
               break;
            }
         }
      }
   }
   return RHlist[m_name];
  }

std::string 
BLANCpp::findNextHop(std::string dest, double amount, bool send){
   std::vector<RoutingEntry> options; 
   for (auto i = RoutingTable[dest].begin(); i != RoutingTable[dest].end(); i++){
      if (send && i->sendMax >= amount)
         options.push_back(*i);
      else if (!send &&  i->recvMax >= amount)
         options.push_back(*i);
   }
   int leastHops = 10000;
   std::string bestNextHop = "";
   for (auto i = options.begin(); i != options.end(); i++){
      if (i->hopCount < leastHops){
         leastHops = i->hopCount;
         bestNextHop = i->nextHop;
      }
   }
   return bestNextHop + "|" + std::to_string(leastHops);
}

//TODO:: Revist this function for optimization
std::string 
BLANCpp::createPath(std::string dest, double amount){
   bool complete = false;
   std::string path = "";
   while (!complete){
      if(NonceTable[dest].size() > 0){
         complete = true;
         path = NonceTable[dest][0] + "," + path;
         continue;
      }
      std::string newDest = dest;
      for (auto i = RHRoutingTable[dest].begin(); i != RHRoutingTable[dest].end(); i++){
         if ( i->maxSend >= amount){
            path = i->path + "," + path;
            newDest = i->path;
            if (i->complete) complete = true; 
            break;
         }
      }
      dest = newDest;
   }
      if(DEBUG3) std::cout<<"This is the Path I found "<<path<<std::endl;
   return path;
}

} // Namespace ns3



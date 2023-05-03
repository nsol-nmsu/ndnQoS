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
 #include "BLANC++V2.hpp"
 
 namespace ns3 {
 
 NS_LOG_COMPONENT_DEFINE ("ns3.BLANCppV2");
 NS_OBJECT_ENSURE_REGISTERED (BLANCppV2);
 
 TypeId
 BLANCppV2::GetTypeId (void)
 {
   static TypeId tid = TypeId ("ns3::BLANCppV2")
     .SetParent<Application> ()
     .AddConstructor<BLANCppV2> ()
     .AddAttribute ("Port", "Port on which we listen for incoming connections.",
                    UintegerValue (7),
                    MakeUintegerAccessor (&BLANCppV2::m_local_port),
                    MakeUintegerChecker<uint16_t> ())
     .AddAttribute ("PacketSize",
                   "The size of outbound packet, typically acknowledgement packets from server application. 536 not fragmented on 1500 MTU",
                   UintegerValue (536),
                   MakeUintegerAccessor (&BLANCppV2::m_packet_size),
		   MakeUintegerChecker<uint32_t> ())
     .AddAttribute ("RouterHelper",
                   "True if application will run in Router helper mode.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&BLANCppV2::m_route_helper),
                   MakeUintegerChecker<uint32_t> ())
     .AddAttribute ("Name",
                   "Name of node.",
                   StringValue ("0"),
                   MakeStringAccessor (&BLANCppV2::m_name),
                   MakeStringChecker())     
     .AddTraceSource ("ReceivedPacketS",
                     "A packet has been received",
                     MakeTraceSourceAccessor (&BLANCppV2::m_receivedPacket),
                     "ns3::BLANCppV2::ReceivedPacketTraceCallback")
     .AddTraceSource ("SentPacketS",
                     "A packet has been sent",
                     MakeTraceSourceAccessor (&BLANCppV2::m_sentPacket),
                     "ns3::BLANCppV2::SentPacketTraceCallback")
     .AddTraceSource ("OnFindReply",
                     "A findReply packet has been received",
                     MakeTraceSourceAccessor (&BLANCppV2::m_onFindReply),
                     "ns3::BLANCppV2::OnFindReplyTraceCallback")
     .AddTraceSource ("OnHold",
                     "A hold packet has been recived",
                     MakeTraceSourceAccessor (&BLANCppV2::m_onHold),
                     "ns3::BLANCppV2::OnHoldTraceCallback")
     .AddTraceSource ("OnPay",
                     "A pay packet has been received",
                     MakeTraceSourceAccessor (&BLANCppV2::m_onPay),
                     "ns3::BLANCppV2::OnPayTraceCallback")
   ;
   return tid;
 }
 
 
BLANCppV2::BLANCppV2 ()
 {
   NS_LOG_FUNCTION_NOARGS ();
   m_socket = 0;
   m_running = false;
 }
 
BLANCppV2::~BLANCppV2()
 {
   NS_LOG_FUNCTION_NOARGS ();
   m_socket = 0;
 }
 
void
BLANCppV2::DoDispose (void)
 {
   NS_LOG_FUNCTION_NOARGS ();
   Application::DoDispose ();
 }
 
void
BLANCppV2::StartApplication (void)
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
         MakeCallback (&BLANCppV2::HandleAccept, this));

 }
 
void
BLANCppV2::StopApplication ()
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
BLANCppV2::ReceivePacket (Ptr<Socket> s)
 {

   NS_LOG_FUNCTION (this << s);
   Ptr<Packet> packet;
   Address from;

   while (packet = s->RecvFrom (from))
     {
       if (packet->GetSize () > 0)
         {

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
 
void BLANCppV2::HandleAccept (Ptr<Socket> s, const Address& from)
 {
   NS_LOG_FUNCTION (this << s << from);
   s->SetRecvCallback (MakeCallback (&BLANCppV2::ReceivePacket, this));
   s->SetCloseCallbacks(MakeCallback (&BLANCppV2::HandleSuccessClose, this),
     MakeNullCallback<void, Ptr<Socket> > () );
 }
 
void BLANCppV2::HandleSuccessClose(Ptr<Socket> s)
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
BLANCppV2::processPacket(Ptr<Packet> p, Ptr<Socket> s){
   blancHeader packetHeader;
   p->RemoveHeader(packetHeader);
   uint32_t pType = packetHeader.GetPacketType();
	  
   switch(pType){
      case Find:
      case FindRecv:
      case 10:
         onFindPacket(p, packetHeader, s);
         break;
      case FindReply:
	      onFindReply(p, packetHeader);
         break;
      case Hold:
         onHoldPacket(p, packetHeader);
         break;
      case HoldRecv:
         onHoldRecvPacket(p, packetHeader, s);
         break;
      case Pay:
         onPayPacket(p, packetHeader);
         break;
      default:
         break;
   }
}

void 
BLANCppV2::onHoldPacket(Ptr<Packet> p, blancHeader ph)
 {

   uint8_t buffer[p->GetSize()];
   p->CopyData (buffer,  p->GetSize());

   std::ostringstream convert;
   for (int a = 0; a < p->GetSize(); a++) {
      convert << (char)buffer[a];
   }


   std::string payload = convert.str();

   std::vector<std::string> items = SplitString(payload, '|');
   uint32_t txID = std::stoi(items[0]);
   uint32_t oldTxID = std::stoi(items[1]);
   std::string dest = txidTable[oldTxID].dest;
   double amount = std::stod(items[2]);
   uint64_t timeout = std::stoi(items[4]);

   std::cout << m_name <<"  NodeID "<<std::endl;
   std::cout << txID <<"  TXiD "<<std::endl;
   std::cout << oldTxID <<" TXiDPrime "<<std::endl;   
   std::cout << dest <<std::endl;
   std::cout << amount <<"  amount\n"<<std::endl;

   //Get all values from packet
   std::string src = "";//REPLACE	 
   std::string nextHop = "";

   if ( txidTable.find(txID) != txidTable.end() && txidTable[txID].pending != 0) {

      if(hasHoldRecv(oldTxID)) {
	      updateBCHold(txID);
         m_onHold(std::stoi(m_name), txID, false);	 
      }
      return;
   }


   uint32_t nextTxID = oldTxID;
   //Create next hold packet
   //Ptr<Packet> packet_copy = Create<Packet> (m_packet_size);
   //TODO: Create return setting
   if(m_route_helper && m_name == dest){

      //Update Transaction Table
      nextTxID = txidTable[oldTxID].nextTxID;
      TransactionInfo entry = txidTable[oldTxID];
      txidTable.erase(oldTxID);
      txidTable[txID] = entry;      

      //Update next hop and Router Helper Destination
      nextHop = txidTable[txID].nextHop;
      dest = txidTable[txID].nextDest;

      //Send out BC message, affirming hold.
      updateBCHold(txID);
   }
   else if(m_name != dest){
      nextHop = txidTable[oldTxID].nextHop;
      TransactionInfo entry = txidTable[oldTxID];
      txidTable.erase(oldTxID);
      txidTable[txID] = entry;
   }

   txidTable[txID].pending = amount;

   blancHeader packetHeader;
   packetHeader.SetPacketType(Hold);

   payload = std::to_string(txID) + "|";
   payload += std::to_string(nextTxID) + "|";
   payload += std::to_string(amount) + "|";
   payload += "en|";
   payload += "5";
   
   updatePathWeight(nextHop, amount);
   insertTimeout(txID, nextHop, timeout);

   //Forward next hold packet
   forwardPacket(packetHeader, nextHop, payload);
   

   //Send out BC message, all only do in case of timeout. 
   //updateBCHold(txID);
   
 }

void 
BLANCppV2::onFindPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s){

   //Get all values from packet
   std::string src = "";//REPLACE
   uint32_t type = ph.GetPacketType();
   std::string nextHop = "";

   uint8_t buffer[p->GetSize()];   
   p->CopyData (buffer,  p->GetSize());
   
   std::ostringstream convert;
   for (int a = 0; a < p->GetSize(); a++) {
      convert << (char)buffer[a];
   }
   

   std::string payload = convert.str();

   std::vector<std::string> items = SplitString(payload, '|');
   uint32_t txID = std::stoi(items[0]);
   std::string path = items[1];
   std::string dest = SplitString(path, ',')[0];
   txidTable[txID].dest = dest;
   double amount = std::stod(items[2]);
   uint32_t secret = std::stoi(items[3]);
   uint64_t timeout = std::stoi(items[4]);

   txidTable[txID].src = s;

   std::cout << "Node "<<m_name<<std::endl;
   std::cout << txID <<"  TXiD "<<std::endl;
   std::cout << path <<"  dest "<<std::endl;
   std::cout << amount <<"  amount "<<std::endl;
   std::cout << secret <<"  secret "<<std::endl;
   std::cout << timeout <<"  timeout\n"<<std::endl;
   
   if(m_name != dest){
      nextHop = findRHTable[dest];
      txidTable[txID].nextHop = nextHop;
   }    
      
   insertTimeout(txID, src, timeout);

   //Create next find packet
   //Ptr<Packet> packet_copy = Create<Packet> (m_packet_size);
   if(m_route_helper && m_name == dest){

      if ( txidTable.find(secret) != txidTable.end() ){
         txidTable[txID].nextTxID = txidTable[secret].nextTxID;
	      txidTable[ txidTable[secret].nextTxID ].nextTxID = txID;
	      txidTable.erase(secret);
      }	    
      else {
         txidTable[secret].nextTxID = txID;
      }
   //txidTable[txID].src = s;

      if (type == Find){

         //Update Transaction Table
         uint32_t nextTxID = createTxID(txID);
   	 txidTable[txID].nextTxID = nextTxID;
    	 //txID = nextTxID;

   	 type = 10;

   	 //Update next hop and Router Helper Destination  
	 dest = SplitString(path, ',')[1];
	 nextHop = findRHTable[dest];
      txidTable[txID].nextHop = nextHop;
      txidTable[txID].nextDest = dest;
      
      txID = nextTxID;
	 path = dest;
      }
      else {
         //Send out BC message, affirming hold. TODO: Confirm if all do this or only RHs.
         updateBCFind(txID);	     
	 if (type == 10){
	    sendFindReply(txID, secret, amount);		 
	 }
	 else{
            sendFindReply(txID, secret, amount);		 
	 }

         return;
      }
   }

   blancHeader packetHeader;
   packetHeader.SetPacketType(type);

   payload = std::to_string(txID) + "|";
   payload += path;
   payload += "|";

   std::cout<<dest<<std::endl;
   if(amount > neighborTable[nextHop].cost) amount = neighborTable[nextHop].cost;
      payload += std::to_string(amount) + "|";

   payload += std::to_string(secret) + "|";
   payload += "5";

   //Forward next hold packet
   forwardPacket(packetHeader, nextHop, payload);


   //Send out BC message, affirming hold. TODO: Confirm if all do this or only RHs.
   updateBCFind(txID);
}

void 
BLANCppV2::onHoldRecvPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s)
 { 

   uint8_t buffer[p->GetSize()];
   p->CopyData (buffer,  p->GetSize());

   std::ostringstream convert;
   for (int a = 0; a < p->GetSize(); a++) {
      convert << (char)buffer[a];
   }


   std::string payload = convert.str();

   std::vector<std::string> items = SplitString(payload, '|');
   uint32_t txID = std::stoi(items[0]);
   uint32_t oldTxID = std::stoi(items[1]);
   std::string dest = txidTable[oldTxID].dest;
   double amount = std::stod(items[2]);
   uint64_t timeout = std::stoi(items[4]);

   txidTable[txID].src = s;

   std::cout << m_name <<"  NodeID "<<std::endl;

   std::cout << txID <<"  TXiD "<<std::endl;
   std::cout << oldTxID <<" TXiDPrime "<<std::endl;
   std::cout << amount <<"  amount\n"<<std::endl;

   //Get all values from packet
   std::string src = "";//REPLACE
   std::string nextHop = "";

   if ( txidTable.find(txID) != txidTable.end() && txidTable[txID].pending != 0) return;

   if(m_name != dest){
      nextHop = txidTable[oldTxID].nextHop;
      txidTable.erase(oldTxID);
   }

   txidTable[txID].pending = amount;

   //Create next hold packet
   if(!m_route_helper || m_name != dest){

      blancHeader packetHeader;
      packetHeader.SetPacketType(HoldRecv);

      payload = std::to_string(txID) + "|";
      payload += std::to_string(oldTxID) + "|";
      payload += std::to_string(amount) + "|";
      payload += "en|";
      payload += "5";
      
      updatePathWeight(nextHop, amount);
      insertTimeout(txID, nextHop, timeout);

      //Forward next hold packet
      forwardPacket(packetHeader, nextHop, payload);
   }
   else if (m_route_helper && m_name == dest){
      //Send out BC message, affirming hold. 
      updateBCHoldRecv(txID);
      m_onHold(std::stoi(m_name), txID, true);
   }
 }

void 
BLANCppV2::onFindReply(Ptr<Packet> p, blancHeader ph)
 {

   uint8_t buffer[p->GetSize()];
   p->CopyData (buffer,  p->GetSize());

   std::ostringstream convert;
   for (int a = 0; a < p->GetSize(); a++) {
      convert << (char)buffer[a];
   }


   std::string payload = convert.str();

   std::vector<std::string> items = SplitString(payload, '|');
   uint32_t txID = std::stoi(items[0]);
   double amount = std::stod(items[1]);
   uint32_t secret = std::stoi(items[2]);
   
      std::string src = "";//REPLACE


   std::cout << "Find reply "<< m_name  <<"\n"<<txID <<"  TXiD "<<std::endl;
   std::cout << amount <<"  amount"<<std::endl;
   std::cout << secret <<"  Secret\n"<<std::endl;

   blancHeader packetHeader;
   packetHeader.SetPacketType(FindReply);

   if (txidTable[txID].src != NULL ){
      //Forward next pay packet
      forwardPacket(packetHeader, txidTable[txID].src, payload);
      txidTable[txID].src = NULL;
   }
   else if (m_route_helper){
      for (auto i = txidTable.begin(); i != txidTable.end(); i++){
         if (i->second.nextTxID == txID){
            sendFindReply(i->first, secret, amount);
	    return;
	 }
      }
   }
   else {
      //Update next hop and Router Helper Destination
      m_onFindReply(std::stoi(m_name), secret, amount);
   }
 }

void 
BLANCppV2::onPayPacket(Ptr<Packet> p, blancHeader ph)
 {

   uint8_t buffer[p->GetSize()];
   p->CopyData (buffer,  p->GetSize());

   std::ostringstream convert;
   for (int a = 0; a < p->GetSize(); a++) {
      convert << (char)buffer[a];
   }


   std::string payload = convert.str();

   std::vector<std::string> items = SplitString(payload, '|');
   uint32_t txID = std::stoi(items[0]);
   double amount = std::stod(items[1]);
   uint64_t timeout = std::stoi(items[2]);
      std::string src = "";//REPLACE

   std::cout << m_name <<"  NodeID "<<std::endl;

   std::cout << txID <<"  TXiD "<<std::endl;
   std::cout << amount <<"  amount\n"<<std::endl;
   if ( txidTable.find(txID) == txidTable.end() || txidTable[txID].pending == 0) return;

   insertTimeout(txID, src, timeout);

   if(m_txID == txID ){
      //Update next hop and Router Helper Destination
      m_onPay(std::stoi(m_name), txID);
      //Send out BC message, affirming hold.
      updateBCPay(txID);  
        
     return; 
   }

   if (txidTable[txID].src == NULL ){
      //Create next pay packet

      std::string nextHop = txidTable[txID].nextHop;

      //Forward next pay packet
      blancHeader packetHeader;
      packetHeader.SetPacketType(Pay);

      forwardPacket(packetHeader, nextHop, payload);
   }
   else if (txidTable[txID].src != NULL ){
      //Forward next pay packet
      blancHeader packetHeader;
      packetHeader.SetPacketType(Pay);

      forwardPacket(packetHeader, txidTable[txID].src, payload);
   }
   txidTable.erase(txID);//TODO:Move to after BC confirmation

 }

void 
BLANCppV2::onAdvertPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s)
{
   uint8_t buffer[p->GetSize()];
   p->CopyData (buffer,  p->GetSize());

   std::ostringstream convert;
   for (int a = 0; a < p->GetSize(); a++) {
      convert << (char)buffer[a];
   }

   std::string payload = convert.str();

   std::vector<std::string> items = SplitString(payload, '|');
   std::string src = items[0];
   double sendMax = std::stod(items[1]);
   double recvMax = std::stod(items[2]);
   double hopCount = std::stod(items[3]);
   double hopMax = std::stod(items[4]);
   double band = std::stod(items[5]);
   std::string route = items[6];

   //uint64_t timestamp = std::stoi(items[2]);
    
   std::cout << m_name <<"  NodeID "<<std::endl;
   std::cout << src <<"  RH-Advert Source "<<std::endl;
   std::cout << sendMax <<"  send max\n"<<std::endl;
   std::cout << recvMax <<"  recv max\n"<<std::endl;
   std::cout << hopCount <<"  hop count\n"<<std::endl;
   std::cout << hopMax <<"  hop max\n"<<std::endl;

   hopCount++;
   std::string nextHop;
   RoutingEntry* entry;

   for (auto it = neighborTable.begin(); it != neighborTable.end(); it++){
      if (it->second.socket == s)
         nextHop = it->first; 
   }

   if (RoutingTable.find(src) == RoutingTable.end()){
      RoutingTable[src].push_back(RoutingEntry());
      entry = &RoutingTable[src][0];
   }
   else {
      for (int i = 0; i < RoutingTable[src].size(); i++){
         if (RoutingTable[src][i].nextHop == nextHop)
            entry = &RoutingTable[src][i];
      }
      if (entry == NULL){
         RoutingTable[src].push_back(RoutingEntry());
         entry = &RoutingTable[src][RoutingTable[src].size()];
      }

   }

   entry->sendMax = neighborTable[entry->nextHop].cost;
   entry->recvMax = recvMax;
   entry->hopCount = hopCount;
   //entry->expireTime = ;
   //TODO: Add signature verification
   //TODO: Add keyGen

   if (hopCount > (hopMax-band) &&  hopCount <= hopMax)
      sendAdvertReply(items);
   if (hopCount>= hopMax)
      return;
   for (auto it = neighborTable.begin(); it != neighborTable.end(); it++){
      std::string payload = m_name;
      payload += "|"+ std::to_string(entry->sendMax) + "|";
      payload += std::to_string(std::min(entry->recvMax, neighborTable[it->first].cost));
      payload += "|" + std::to_string(hopCount) + "|";
      payload += std::to_string(hopMax) + "|";
      payload += std::to_string(band) + "|";
      payload += route + "," + m_name + "|";

      //TODO: Add timestamp
      payload += "now";
      
      blancHeader packetHeader;
      packetHeader.SetPacketType(Advert);
      //TODO: Add signature

      forwardPacket(packetHeader, it->first, payload);
   }

   
}

void 
BLANCppV2::onAdvertReply(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s)
{
   uint8_t buffer[p->GetSize()];
   p->CopyData (buffer,  p->GetSize());

   std::ostringstream convert;
   for (int a = 0; a < p->GetSize(); a++) {
      convert << (char)buffer[a];
   }
   std::string payload = convert.str();
   std::vector<std::string> items = SplitString(payload, '|');

   std::string src = items[0];
   double sendMax = std::stod(items[1]);
   double recvMax = std::stod(items[2]);
   double hopCount = std::stod(items[3]);
   std::string route = items[4];
   std::string nextRoute = SplitString(route, ',')[ SplitString(route, ',').size()-1 ];

   //uint64_t timestamp = std::stoi(items[2]);
    
   std::cout << m_name <<"  NodeID "<<std::endl;
   std::cout << src <<"  Nonce "<<std::endl;
   std::cout << sendMax <<"  send max\n"<<std::endl;
   std::cout << recvMax <<"  recv max\n"<<std::endl;
   std::cout << hopCount <<"  hop count\n"<<std::endl;

   hopCount++;
   std::string nextHop;
   RoutingEntry* entry;

   for (auto it = neighborTable.begin(); it != neighborTable.end(); it++){
      if (it->second.socket == s)
         nextHop = it->first; 
   }

   if (RoutingTable.find(src) == RoutingTable.end()){
      RoutingTable[src].push_back(RoutingEntry());
      entry = &RoutingTable[src][0];
   }
   else {
      for (int i = 0; i < RoutingTable[src].size(); i++){
         if (RoutingTable[src][i].nextHop == nextHop)
            entry = &RoutingTable[src][i];
      }
      if (entry == NULL){
         RoutingTable[src].push_back(RoutingEntry());
         entry = &RoutingTable[src][RoutingTable[src].size()];
      }

   }

   entry->sendMax = neighborTable[entry->nextHop].cost;
   entry->recvMax = recvMax;
   entry->hopCount = hopCount;
   //entry->expireTime = ;
   //TODO: Add signature verification
   //TODO: Add keyGen

   if (nextRoute.size()>0){
      std::string payload = m_name;
      payload += "|"+ std::to_string(entry->sendMax) + "|";
      payload += std::to_string(std::min(entry->recvMax, neighborTable[nextRoute].cost));
      payload += "|" + std::to_string(hopCount) + "|";
      payload += route.substr(0, (route.size() - nextRoute.size())  ) + "|";
      //TODO: Add timestamp
      payload += "now";
      
      blancHeader packetHeader;
      packetHeader.SetPacketType(Advert);
      //TODO: Add signature

      forwardPacket(packetHeader, nextRoute, payload);
   }
}

void 
BLANCppV2::forwardPacket(blancHeader packetHeader, std::string nextHop, std::string payload)
{

  Ptr<Packet> p;
  p = Create<Packet> (reinterpret_cast<const uint8_t*> (payload.c_str()), payload.size());  
  //p = Create<Packet> (100);
  uint64_t m_bytesSent = 100;
  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well

  //Add Blanc information to packet header and send
  Address peerAddress = neighborTable[nextHop].address;
  Ptr<Socket> socket = getSocket(nextHop);

  p->AddHeader(packetHeader);

  
  socket->Send (p);
  NS_LOG_INFO ("Sent " << 100 << " bytes to " << peerAddress << ":" << m_peerPort);

  //Increase sequence number for next packet
  m_seq = m_seq + 1;
}

void
BLANCppV2::forwardPacket(blancHeader packetHeader, Ptr<Socket> socket, std::string payload)
{

  Ptr<Packet> p;
  p = Create<Packet> (reinterpret_cast<const uint8_t*> (payload.c_str()), payload.size());
  //p = Create<Packet> (100);
  uint64_t m_bytesSent = 100;
  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  //m_txTrace (p);

  //Add Blanc information to packet header and send

  p->AddHeader(packetHeader);

  socket->Send (p);

  //Increase sequence number for next packet
  m_seq = m_seq + 1;
}

void 
BLANCppV2::setNeighborCredit(std::string name, double amount){
   neighborTable[name].cost = amount;
}

void
BLANCppV2::setNeighbor(std::string name, Ipv4Address address){
   neighborTable[name].address = address;
}

Ptr<Socket> 
BLANCppV2::getSocket(std::string dest)
{
 Ptr<Socket> socket = neighborTable[dest].socket;
	
 if (socket == 0)
 {
    TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");   
    socket = Socket::CreateSocket (GetNode (), tid);
    socket->Bind();
    socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom( neighborTable[dest].address ), m_peerPort));

    socket->SetRecvCallback (MakeCallback (&BLANCppV2::ReceivePacket, this));
 }
 return socket;
}


bool
BLANCppV2::hasHoldRecv(uint32_t txID)
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
BLANCppV2::startTransaction(uint32_t txID, uint32_t secret, std::vector<std::string> peerlist, bool payer)
{
   //Determine number of segments and create the needed txIDs.
   std::string NH = findRHTable[peerlist[0]];
   m_amount = neighborTable[NH].cost;
   m_payer = payer;
   txidTable[txID].nextTxID = createTxID(txID);
   m_txID = txidTable[txID].nextTxID;
   txidTable[txID].nextDest = peerlist[0];
   if (payer) nextRH = peerlist[1];
   sendFindPacket(txID, secret);
   TiP = true;
}

void
BLANCppV2::reset(uint32_t txID)
{
   //Determine number of segments and create the needed txIDs.
   m_amount = 0;
   m_payer = false;
   txidTable.erase(txID);
   m_txID = 0;
   nextRH = "";
   TiP = false;
}

uint32_t
BLANCppV2::createTxID(uint32_t txID){
   return txID + rand()%10;
}

void 
BLANCppV2::sendAdvertPacket()
{
   AdvertSetup();

   //TODO: Add as a variable
   int hopMax = 10;
   int band = 2;


   for (auto it = neighborTable.begin(); it != neighborTable.end(); it++){
      std::string payload = m_name;
      payload += "|10000000000|";
      payload += std::to_string(neighborTable[it->first].cost) + "|0|";
      payload += std::to_string(hopMax) + "|";
      payload += std::to_string(band) + "|";
      payload += m_name + "|";
      //TODO: Add timestamp
      payload += "now";
      
      blancHeader packetHeader;
      packetHeader.SetPacketType(Advert);
      //TODO: Add signature

      forwardPacket(packetHeader, it->first, payload);
   }
}

void 
BLANCppV2::sendAdvertReply(std::vector<std::string> items)
{
   AdvertSetup();

   std::string recvMax = items[1];
   std::string sendMax = items[2];
   std::string route = items[4];
   std::string nextRoute = SplitString(route, ',')[ SplitString(route, ',').size()-1 ];

   if (nextRoute.size()>0){
      std::string payload = m_name;
      payload += "|" + sendMax + "|";
      payload += std::to_string(neighborTable[nextRoute].cost) + "|0|";
      payload += route.substr(0, (route.size() - nextRoute.size())  ) + "|";

      //TODO: Add timestamp
      payload += "now";
      
      blancHeader packetHeader;
      packetHeader.SetPacketType(Advert);
      //TODO: Add signature

      forwardPacket(packetHeader, nextRoute, payload);
   }
}

void 
BLANCppV2::sendFindReply(uint32_t txID, uint32_t secret, double amount)
{
   blancHeader packetHeader;
   packetHeader.SetPacketType(FindReply);

   std::string payload = std::to_string(txID) + "|";
   payload += std::to_string(amount) + "|";
   payload += std::to_string(secret);

   forwardPacket(packetHeader, txidTable[txID].src, payload);
   txidTable[txID].src = NULL;
}

void
BLANCppV2::sendFindPacket(uint32_t txID, uint32_t secret)
{

   uint32_t txIDPrime = txidTable[txID].nextTxID;
   std::string dest = txidTable[txID].nextDest;
   std::string nextHop = findRHTable[dest]; //Update based on Some table base
   txidTable[txID].nextHop = nextHop;

   blancHeader packetHeader;
   packetHeader.SetPacketType(FindRecv);
   if(m_payer)
      packetHeader.SetPacketType(Find);

   std::string payload = std::to_string(txIDPrime) + "|";
   payload += dest;
   if(m_payer)
      payload += "," + nextRH;
   payload += "|";
   payload += std::to_string(m_amount) + "|";
   payload += std::to_string(secret) + "|";
   payload += "5";

   forwardPacket(packetHeader, nextHop, payload);

}

void 
BLANCppV2::sendHoldPacket(uint32_t txID, double amount)
{
   uint32_t txIDPrime = txidTable[txID].nextTxID;
   std::cout<<txIDPrime<<" Let's see the issue.\n";
   std::string dest = txidTable[txID].nextDest;
   std::string nextHop = txidTable[txID].nextHop;

   m_amount = amount;

   blancHeader packetHeader;
   if(m_payer){
      packetHeader.SetPacketType(Hold);
   }
   else { 
      packetHeader.SetPacketType(HoldRecv);
      txidTable[txID].pending = amount;
      m_txID = txID;
   }

   std::string payload = std::to_string(txID) + "|";
   payload += std::to_string(txIDPrime) + "|";
   payload += std::to_string(amount) + "|";
   payload += "ENC|";
   payload += "5";

   int timeout = 5;
   updatePathWeight(nextHop, amount);
   insertTimeout(txID, nextHop, timeout);

   forwardPacket(packetHeader, nextHop, payload);
   std::cout<<"Hold phase\n\n\n";
}

void 
BLANCppV2::sendPayPacket(uint32_t txID)
{
   std::cout<<"Pay phase \n\n\n";

   std::string nextHop = txidTable[txID].nextHop;
   txidTable.erase(txID);

   blancHeader packetHeader;
   packetHeader.SetPacketType(Pay);
   std::string payload = std::to_string(txID) + "|";
   payload += std::to_string(m_amount) + "|";
   payload += "5";

   forwardPacket(packetHeader, nextHop, payload);
}

} // Namespace ns3



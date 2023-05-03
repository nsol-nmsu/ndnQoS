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
 #include "BLANC.hpp" 
 
 namespace ns3 {
 
 NS_LOG_COMPONENT_DEFINE ("ns3.BLANC");
 NS_OBJECT_ENSURE_REGISTERED (Blanc);
 
 TypeId
 Blanc::GetTypeId (void)
 {
   static TypeId tid = TypeId ("ns3::BLANC")
     .SetParent<Application> ()
     .AddConstructor<Blanc> ()
     .AddAttribute ("Port", "Port on which we listen for incoming connections.",
                    UintegerValue (7),
                    MakeUintegerAccessor (&Blanc::m_local_port),
                    MakeUintegerChecker<uint16_t> ())
     .AddAttribute ("PacketSize",
                   "The size of outbound packet, typically acknowledgement packets from server application. 536 not fragmented on 1500 MTU",
                   UintegerValue (536),
                   MakeUintegerAccessor (&Blanc::m_packet_size),
		   MakeUintegerChecker<uint32_t> ())
     .AddAttribute ("RouterHelper",
                   "True if application will run in Router helper mode.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&Blanc::m_route_helper),
                   MakeUintegerChecker<uint32_t> ())
     .AddAttribute ("Name",
                   "Name of node.",
                   StringValue ("0"),
                   MakeStringAccessor (&Blanc::m_name),
                   MakeStringChecker())     
     .AddTraceSource ("ReceivedPacketS",
                     "A packet has been received",
                     MakeTraceSourceAccessor (&Blanc::m_receivedPacket),
                     "ns3::Blanc::ReceivedPacketTraceCallback")
     .AddTraceSource ("SentPacketS",
                     "A packet has been sent",
                     MakeTraceSourceAccessor (&Blanc::m_sentPacket),
                     "ns3::Blanc::SentPacketTraceCallback")
     .AddTraceSource ("OnFindReply",
                     "A findReply packet has been received",
                     MakeTraceSourceAccessor (&Blanc::m_onFindReply),
                     "ns3::Blanc::OnFindReplyTraceCallback")
     .AddTraceSource ("OnHold",
                     "A hold packet has been recived",
                     MakeTraceSourceAccessor (&Blanc::m_onHold),
                     "ns3::Blanc::OnHoldTraceCallback")
     .AddTraceSource ("OnPay",
                     "A pay packet has been received",
                     MakeTraceSourceAccessor (&Blanc::m_onPay),
                     "ns3::Blanc::OnPayTraceCallback")
   ;
   return tid;
 }
 
 
Blanc::Blanc ()
 {
   NS_LOG_FUNCTION_NOARGS ();
   m_socket = 0;
   m_running = false;
 }
 
Blanc::~Blanc()
 {
   NS_LOG_FUNCTION_NOARGS ();
   m_socket = 0;
 }
 
void
Blanc::DoDispose (void)
 {
   NS_LOG_FUNCTION_NOARGS ();
   Application::DoDispose ();
 }
 
void
Blanc::StartApplication (void)
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
         MakeCallback (&Blanc::HandleAccept, this));

 }
 
void
Blanc::StopApplication ()
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
Blanc::ReceivePacket (Ptr<Socket> s)
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
 
void Blanc::HandleAccept (Ptr<Socket> s, const Address& from)
 {
   NS_LOG_FUNCTION (this << s << from);
   s->SetRecvCallback (MakeCallback (&Blanc::ReceivePacket, this));
   s->SetCloseCallbacks(MakeCallback (&Blanc::HandleSuccessClose, this),
     MakeNullCallback<void, Ptr<Socket> > () );
 }
 
void Blanc::HandleSuccessClose(Ptr<Socket> s)
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
Blanc::processPacket(Ptr<Packet> p, Ptr<Socket> s){
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
      case FindReply:
	      onFindReply(p, packetHeader, s);
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
      case Reg:
         onRegPacket(p, packetHeader, s);
         break;           
      default:
         break;
   }
     if (extraSize > 0){
      pkt->RemoveAtStart( packetHeader.GetPayloadSize() );
      processPacket(pkt, s);
   }
}

void 
Blanc::onHoldPacket(Ptr<Packet> p, blancHeader ph)
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

   TransactionInfo entry;  


   if ( txidTable.find(txID) != txidTable.end() && txidTable[txID].pending != 0) {
      TransactionInfo* overlap = checkTransaction(txID, oldTxID, dest);
      if (overlap == NULL) return; 
      entry = txidTable[oldTxID];
      entry.overlap = overlap;
      entry.nextTxID = oldTxID;
   }
   else entry = txidTable[oldTxID];

   uint32_t nextTxID = oldTxID;
   //Create next hold packet
   //TODO: Create return setting

   //Update Transaction Table
   nextHop = txidTable[oldTxID].nextHop;
   txidTable[txID] = entry;  

   if(m_route_helper && m_name == dest){   
      //Update next hop and Router Helper Destination
      nextTxID = txidTable[oldTxID].nextTxID;
      nextHop = txidTable[txID].nextHop;
      dest = txidTable[txID].nextDest;

      //Send out BC message, affirming hold.
      updateBCHold(txID);
   }

   txidTable.erase(oldTxID);
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
   std::cout<<"sent "<<nextHop<<std::endl;
   

   //Send out BC message, all only do in case of timeout. 
   //updateBCHold(txID);
   
 }

void 
Blanc::onFindPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s){

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
   if ( txidTable.find(txID) != txidTable.end() ){
      return;
   }	       
   std::string path = items[1];
   std::string dest = SplitString(path, ',')[0];
   txidTable[txID].dest = dest;
   double amount = std::stod(items[2]);
   uint32_t secret = std::stoi(items[3]);
   int hopsLeft = std::stoi(items[4]);
   uint64_t timeout = 5;

   std::cout << "Node "<<m_name<<std::endl;
   std::cout << txID <<"  TXiD "<<std::endl;
   std::cout << path <<"  dest "<<std::endl;
   std::cout << amount <<"  amount "<<std::endl;
   std::cout << secret <<"  secret "<<std::endl;
   std::cout << timeout <<"  timeout\n"<<std::endl;

   txidTable[txID].src = s;
   
   if(m_name != dest){
      //nextHop = findRHTable[dest];
      //txidTable[txID].nextHop = nextHop;
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

      if (type == Find){

         //Update Transaction Table
         uint32_t nextTxID = createTxID(txID);
   	   txidTable[txID].nextTxID = nextTxID;
    	   //txID = nextTxID;
   	   type = 10;
   	   //Update next hop and Router Helper Destination  
	      dest = SplitString(path, ',')[1];
	      //nextHop = findRHTable[dest];
         //txidTable[txID].nextHop = nextHop;
         txidTable[txID].nextDest = dest;
         txidTable[nextTxID].dest = dest;

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

   //Send out BC message, affirming hold. TODO: Confirm if all do this or only RHs.
   updateBCFind(txID);
   if (hopsLeft == 0) return;

   for (auto it = neighborTable.begin(); it != neighborTable.end(); it++){
      blancHeader packetHeader;
      packetHeader.SetPacketType(type);

      payload = std::to_string(txID) + "|";
      payload += path;
      payload += "|";

      std::cout<<dest<<std::endl;
      if(amount > it->second.cost) amount = it->second.cost;
         payload += std::to_string(amount) + "|";

      payload += std::to_string(secret) + "|";
      payload += std::to_string(hopsLeft+1);

      //Forward next hold packet
      forwardPacket(packetHeader, it->first, payload);
   }

}

void 
Blanc::onHoldRecvPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s)
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
   txidTable[txID].dest = dest;

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
Blanc::onFindReply(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s)
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
   std::cout << secret <<"  Secret"<<std::endl;

   std::string nextHop;
   for (auto it = neighborTable.begin(); it != neighborTable.end(); it++){
      if (it->second.socket == s)
         nextHop = it->first; 
   }
   std::cout << nextHop <<"  Next Hop\n"<<std::endl;


   blancHeader packetHeader;
   packetHeader.SetPacketType(FindReply);
   txidTable[txID].nextHop = nextHop;
   if (txidTable[txID].src != NULL ){
      //Forward next pay packet
      forwardPacket(packetHeader, txidTable[txID].src, payload);
      txidTable[txID].src = NULL;
   }
   else if (m_route_helper){
      for (auto i = txidTable.begin(); i != txidTable.end(); i++){
         std::cout<<"One of these should be it "<<txID<<"  "<<i->second.nextTxID<<std::endl;
         if (i->second.nextTxID == txID){
            sendFindReply(i->first, secret, amount);
            txidTable[i->first].nextHop = nextHop;
            txidTable.erase(txID);
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
Blanc::onPayPacket(Ptr<Packet> p, blancHeader ph)
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
Blanc::onAdvertPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s)
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

   if (hopCount >= hopMax)
      return;
   for (auto it = neighborTable.begin(); it != neighborTable.end(); it++){
      std::string payload = m_name;
      payload += "|"+ std::to_string(entry->sendMax) + "|";
      payload += std::to_string(std::min(entry->recvMax, neighborTable[it->first].cost));
      payload += "|" + std::to_string(hopCount) + "|";
      payload += std::to_string(hopMax) + "|";
      //TODO: Add timestamp
      payload += "now";
      
      blancHeader packetHeader;
      packetHeader.SetPacketType(Advert);
      //TODO: Add signature

      forwardPacket(packetHeader, it->first, payload);
   }

   
}

void 
Blanc::onRegPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s){
   uint8_t buffer[p->GetSize()];
   p->CopyData (buffer,  p->GetSize());

   std::ostringstream convert;
   for (int a = 0; a < p->GetSize(); a++) {
      convert << (char)buffer[a];
   }

   std::string payload = convert.str();
    
   neighborTable[payload].socket = s;

}

void 
Blanc::forwardPacket(blancHeader packetHeader, std::string nextHop, std::string payload)
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

  
  socket->Send (p);
  NS_LOG_INFO ("Sent " << 100 << " bytes to " << peerAddress << ":" << m_peerPort);

  //Increase sequence number for next packet
  m_seq = m_seq + 1;
}

void
Blanc::forwardPacket(blancHeader packetHeader, Ptr<Socket> socket, std::string payload)
{

  Ptr<Packet> p;
  p = Create<Packet> (reinterpret_cast<const uint8_t*> (payload.c_str()), payload.size());
  //p = Create<Packet> (100);
    packetHeader.setPayloadSize(payload.size());

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
Blanc::setNeighborCredit(std::string name, double amount){
   neighborTable[name].cost = amount;
}

void
Blanc::setNeighbor(std::string name, Ipv4Address address){
   neighborTable[name].address = address;
}

Ptr<Socket> 
Blanc::getSocket(std::string dest)
{
 Ptr<Socket> socket = neighborTable[dest].socket;
	
 if (socket == 0)
 {
    TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");   
    socket = Socket::CreateSocket (GetNode (), tid);
    socket->Bind();
    socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom( neighborTable[dest].address ), m_peerPort));

    socket->SetRecvCallback (MakeCallback (&Blanc::ReceivePacket, this));

    std::string payload = m_name;     
    blancHeader packetHeader;
    packetHeader.SetPacketType(Reg);
    neighborTable[dest].socket = socket;

    forwardPacket(packetHeader, socket, payload);    
 }
 return socket;
}


bool
Blanc::hasHoldRecv(uint32_t txID)
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
Blanc::startTransaction(uint32_t txID, uint32_t secret, std::vector<std::string> peerlist, bool payer)
{
   //Determine number of segments and create the needed txIDs.
   //std::string NH = findRHTable[peerlist[0]];
   m_amount = 10000;//neighborTable[NH].cost;
   m_payer = payer;
   txidTable[txID].nextTxID = createTxID(txID);
   m_txID = txidTable[txID].nextTxID;
   txidTable[txID].nextDest = peerlist[0];
   if (payer) nextRH = peerlist[1];
   sendFindPacket(txID, secret);
   TiP = true;
}

void
Blanc::reset(uint32_t txID)
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
Blanc::createTxID(uint32_t txID){
   return txID + rand()%10;
}

void 
Blanc::sendAdvertPacket()
{
   AdvertSetup();

   //TODO: Add as a variable
   int hopMax = 10;

   for (auto it = neighborTable.begin(); it != neighborTable.end(); it++){
      std::string payload = m_name;
      payload += "|10000000000|";
      payload += std::to_string(neighborTable[it->first].cost) + "|0|";
      payload += std::to_string(hopMax) + "|";
      //TODO: Add timestamp
      payload += "now";
      
      blancHeader packetHeader;
      packetHeader.SetPacketType(Advert);
      //TODO: Add signature

      forwardPacket(packetHeader, it->first, payload);
   }
}


void 
Blanc::sendFindReply(uint32_t txID, uint32_t secret, double amount)
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
Blanc::sendFindPacket(uint32_t txID, uint32_t secret)
{
   uint32_t txIDPrime = txidTable[txID].nextTxID;
   std::string dest = txidTable[txID].nextDest;
   std::string nextHop = findRHTable[dest]; //Update based on Some table base
   txidTable[txID].nextHop = nextHop;
   txidTable[txIDPrime];
   
   int hopMax = 10;
   for (auto it = neighborTable.begin(); it != neighborTable.end(); it++){
      blancHeader packetHeader;
      packetHeader.SetPacketType(FindRecv);
      if(m_payer)
         packetHeader.SetPacketType(Find);
   std::cout<<"Node "<<m_name<<" "<< it->first << " " <<dest<<" sending out find packet\n";

      std::string payload = std::to_string(txIDPrime) + "|";
      payload += dest;
      if(m_payer)
         payload += "," + nextRH;
      payload += "|";
      payload += std::to_string(it->second.cost) + "|";
      payload += std::to_string(secret) + "|";
      payload += std::to_string(hopMax);

      forwardPacket(packetHeader, it->first, payload);
   }
}

void 
Blanc::sendHoldPacket(uint32_t txID, double amount)
{
   uint32_t txIDPrime = txidTable[txID].nextTxID;
   txidTable[txID].nextHop = txidTable[txIDPrime].nextHop;
   std::cout<<txIDPrime<<" " <<txidTable[txID].nextDest<<" "<< txidTable[txIDPrime].nextHop << "Let's see the issue.\n";
   std::string dest = txidTable[txID].nextDest;
   std::string nextHop = txidTable[txIDPrime].nextHop;
   txidTable.erase(txIDPrime);

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
Blanc::sendPayPacket(uint32_t txID)
{

   std::string nextHop = txidTable[txID].nextHop;
   txidTable.erase(txID);
   std::cout<<"Pay phase "<<nextHop<<" " << txID <<"\n\n\n";

   blancHeader packetHeader;
   packetHeader.SetPacketType(Pay);
   std::string payload = std::to_string(txID) + "|";
   payload += std::to_string(m_amount) + "|";
   payload += "5";

   forwardPacket(packetHeader, nextHop, payload);
}

} // Namespace ns3



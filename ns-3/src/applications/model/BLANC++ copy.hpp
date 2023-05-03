/* -*- Mode:C; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef BLANC
#define BLANC

#include <unordered_map>
#include <unordered_set>

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/blanc-header.h"
#include "ns3/socket.h"
#include "ns3/packet.h"
#include "BLANC-BB.hpp"

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup applications
 * \defgroup TcpEcho 
 */

/**
 * \ingroup tcpecho
 * \brief A Tcp Echo server
 *
 * Every packet received is sent back to the client.
 */
class BLANCpp : public Application
{
public:

  typedef void (* ReceivedPacketTraceCallback) (uint32_t nodeid, Ptr<Packet> packet, const Address &address, 
		  uint32_t localport, uint32_t packetSize, uint32_t subscription, Ipv4Address localip);
  typedef void (* SentPacketTraceCallback) (uint32_t nodeid, Ptr<Packet> packet, const Address &address, uint32_t localport);

  typedef void (* OnFindReplyTraceCallback) (uint32_t nodeid, uint32_t txID, double amount);
  typedef void (* OnHoldTraceCallback) (uint32_t nodeid, uint32_t txID, bool received);
  typedef void (* OnPayTraceCallback) (uint32_t nodeid, uint32_t txID);

  
   struct RoutingEntry {            // Structure declaration
      std::string nextHop;
      double sendMax;
      double recvMax;
      int hopCount;
      double expireTime;
      RoutingEntry() :
         nextHop(""),
         sendMax(0),
         recvMax(0),
         hopCount(0),
         expireTime(0) {}
   }; 

   struct Neighbor {            // Structure declaration
      Ipv4Address address;
      Ptr<Socket> socket;
      double cost;
      Neighbor() :
         socket(NULL),
         cost(0) {}
   }; 

   struct TransactionInfo {             // Structure declaration
      uint32_t nextTxID;         // Member (int variable)
      std::string nextHop;   // Member (string variable)
      std::string nextDest;
      double pending;
      Ptr<Socket> src;
      std::string dest;
      TransactionInfo() :
         nextTxID(0),
         nextHop(""),
         nextDest(""),
         pending(0),
         src(NULL),
         dest("") {}
   };    

  static TypeId GetTypeId (void);
  BLANCpp ();
  virtual ~BLANCpp ();

  /**
   *
   * Receive the packet from client echo on socket level (layer 4), 
   * handle the packet and return to the client.
   *
   * \param socket TCP socket.
   *
   */
  void ReceivePacket(Ptr<Socket> socket);
  
  /**
  *
  * Handle packet from accept connections.
  *
  * \parm s TCP socket.
  * \parm from Address from client echo.
  */
  void HandleAccept (Ptr<Socket> s, const Address& from);
  
  /**
  *
  * Handle successful closing connections.
  *
  * \parm s TCP socket.
  *
  */
  void HandleSuccessClose(Ptr<Socket> s);
  
  //Incoming Packet handling 
  enum PacketType { 
      Find =      0, 
      FindRecv =  1,
      FindReply = 2,
      Hold =      3,
      HoldRecv =  4,
      Pay =       5,
      Advert =    6,
      Reg =       7
  };

  void processPacket(Ptr<Packet> p, Ptr<Socket> s);

  virtual void onHoldPacket(Ptr<Packet> p, blancHeader ph) = 0;

  virtual void onHoldRecvPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s)  = 0;

  virtual void onFindPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s)  = 0;

  virtual void onFindReply(Ptr<Packet> p, blancHeader ph)  = 0;

  virtual void onPayPacket(Ptr<Packet> p, blancHeader ph)  = 0;
  
  void onRegPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s);
  
  virtual void onAdvertPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s) = 0;

  //Transaction Functions
  bool getTiP(){ return TiP; }; 

  void startTransaction(uint32_t txID, uint32_t secret, std::vector<std::string> peerlist, bool payer);

  void reset(uint32_t txID);
 

  void send(Ptr<Socket> s, Ptr<Packet> p){
   s->Send(p);
  };


  //Packet fowarding
  virtual void sendAdvertPacket();

  virtual void sendRegPacket(std::string dest);

  virtual void sendFindPacket(uint32_t txID, uint32_t secret);

  virtual void sendHoldPacket(uint32_t txID, double amount);

  virtual void sendPayPacket(uint32_t txID);

  virtual void sendFindReply(uint32_t txID, uint32_t secret, double amount);

  void forwardPacket(blancHeader packetHeader, std::string nextHop){
     forwardPacket(packetHeader,nextHop,"");
  };

  void forwardPacket(blancHeader packetHeader, std::string nextHop, std::string payload);

  void forwardPacket(blancHeader packetHeader, Ptr<Socket> socket, std::string payload);  


  void updatePathWeight(std::string name, double amount){
      neighborTable[name].cost -= amount;
  };
  
  //Routing Functions 
  void setFindTable(std::string RH, std::string nextHop){
     findRHTable[RH] = nextHop;
  };

  void setNeighborCredit(std::string name, double amount);

  void setNeighbor(std::string name, Ipv4Address address);

  std::unordered_set<std::string> getReachable(){
   return reachable;
  };


  //Utility Functions
  bool hasHoldRecv(uint32_t txID);

  void insertTimeout(uint32_t txID, std::string src, uint64_t timeout){};

  Ptr<Socket> getSocket(std::string dest);

  std::vector<std::string> SplitString( std::string strLine, char delimiter ) {
   std::string str = strLine;
   std::vector<std::string> result;
   uint32_t i =0;
   std::string buildStr = "";

   for ( i = 0; i<str.size(); i++) {
      if ( str[i]== delimiter ) {
         result.push_back( buildStr );
	 buildStr = "";
      }
      else {
   	      buildStr += str[i];
      }
   }

   if(buildStr!="")
      result.push_back( buildStr );

   return result;
};

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void HandleRead (Ptr<Socket> socket);

protected:

  uint16_t m_local_port;
  bool m_running;
  Ipv4Address m_local_ip;
  Ptr<Socket> m_socket;
  uint32_t m_packet_size;
  uint16_t m_peerPort = 5017;
  uint32_t m_seq;
  double m_amount;
  bool m_payer;


  //BLANCpp Atributes
  bool m_route_helper;
  std::string m_name;
  std::string nextRH;
  uint32_t m_txID = 0;
  std::unordered_set<std::string> reachable;

  std::unordered_map<std::string, std::string> findRHTable;
  std::unordered_map<std::string, std::vector<RoutingEntry>> RoutingTable;

  std::unordered_map<uint32_t, TransactionInfo> txidTable; 
  std::unordered_map<std::string, Neighbor>  neighborTable; 

  //Transaction Pair atributes
  bool TiP = false;


  //Callbacks
  TracedCallback<uint32_t, Ptr<Packet>, const Address &, uint32_t, uint32_t, Ipv4Address> m_receivedPacket;
  TracedCallback<uint32_t, Ptr<Packet>, const Address &, uint32_t> m_sentPacket;
  TracedCallback<uint32_t, uint32_t, double> m_onFindReply;
  TracedCallback<uint32_t, uint32_t, bool> m_onHold;
  TracedCallback<uint32_t, uint32_t> m_onPay;
  

};

} // namespace ns3

#endif /* ICENS_TCP_SERVER */


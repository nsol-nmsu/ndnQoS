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
  typedef void (* OnPayPathTraceCallback) (uint32_t nodeid, uint32_t txID);
  typedef void (* OnTxTraceCallback) (std::string nodeid, uint32_t txID, bool payer);
  typedef void (* OnPathUpdateTraceCallback) (std::string nodeid1, std::string nodeid2, double amount);
  typedef void (* OnTxFailTraceCallback) (uint32_t txID);
  typedef void (* OnTxRetryTraceCallback) (uint32_t txID);
  
   struct RoutingEntry {            // Structure declaration
      std::string nextHop;
      double sendMax;
      double recvMax;
      int hopCount;
      double expireTime;
      bool nonce;
      RoutingEntry() :
         nextHop(""),
         sendMax(0),
         recvMax(0),
         hopCount(0),
         expireTime(0),
         nonce(false) {}
   }; 

   struct RHRoutingEntry {            // Structure declaration
      std::string path;
      double maxSend;
      double maxRecv;
      bool complete;
      RHRoutingEntry() :
         path(""),
         maxSend(0),
         maxRecv(0),
         complete(false) {}
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
      std::string payload;
      double te1;
      double te2;
      Ptr<Socket> src;
      std::string dest;
      int retries;
      bool onPath;
      bool sender;
      bool replied;
      bool payReplied;
      TransactionInfo() :
         nextTxID(0),
         nextHop(""),
         nextDest(""),
         pending(0),
         payload(""),
         te1(0),
         te2(0),
         src(NULL),
         dest(""),
         retries(0),
         onPath(false),
         sender(false),
         replied(false),
         payReplied(false){}
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
      Reg =       7,
      HoldReply = 8,
      PayReply =  9, 
      Nack =      11,
      AdvertReply =    12
  };

  void processPacket(Ptr<Packet> p, Ptr<Socket> s);

  void onHoldPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s);

  void onHoldRecvPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s);

  void onFindPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s);

  void onHoldReply(Ptr<Packet> p, blancHeader ph);

  void onPayPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s);

  void onPayReply(Ptr<Packet> p, blancHeader ph);
  
  void onRegPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s);
  
  void onNack(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s);

  void onAdvertPacket(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s);

  void 
  onAdvertReply(Ptr<Packet> p, blancHeader ph, Ptr<Socket> s);

  //Transaction Functions
  bool getTiP(){ return TiP; }; 

  void startTransaction(uint32_t txID, uint32_t secret, std::vector<std::string> peerlist, bool payer, double amount);

  void reset(uint32_t txID);

  uint32_t createTxID(uint32_t txID);
  
  void AdvertSetup(){};

  void send(Ptr<Socket> s, Ptr<Packet> p);


  //Packet fowarding
  void sendAdvertPacket();

  void 
  sendAdvertReply(std::vector<std::string> items);

  void 
  sendRegPacket(std::string dest);

  void 
  sendHoldPacket(uint32_t txID, double amount);

  void 
  sendHoldReply(uint32_t txID);

  void 
  sendProceedPay(uint32_t txID);

  void 
  sendPayPacket(uint32_t txID);

  void 
  sendPayReply(uint32_t txID);

  void 
  sendNack(uint32_t txID, double te1, std::string dest);

  void 
  forwardPacket(blancHeader packetHeader, std::string nextHop){
     forwardPacket(packetHeader,nextHop,"");
  };

  void 
  forwardPacket(blancHeader packetHeader, std::string nextHop, std::string payload);

  void 
  forwardPacket(blancHeader packetHeader, Ptr<Socket> socket, std::string payload);  

  //Block Chain Functions
  void 
  updateBCFind(uint32_t txID){};

  void 
  updateBCHold(uint32_t txID){};

  void 
  updateBCHoldRecv(uint32_t txID){};

  void 
  updateBCPay(uint32_t txID){};  

  void 
  updatePathWeight(std::string name, double amount, bool sender){
      neighborTable[name].cost -= amount;
      for (auto i = RoutingTable.begin(); i != RoutingTable.end(); i++){
         for (auto each = i->second.begin(); each != i->second.end(); each++){
            if (each->nextHop == name){
               if (sender) each->sendMax -= amount;
               else each->recvMax -= amount;
            }
         }
      }
      m_onPathUpdate(m_name, name, amount);
  };
  
  void 
  updateRoutingTable(std::string name, double amount, bool sender){
      for (auto i = RoutingTable.begin(); i != RoutingTable.end(); i++){
         for (auto each = i->second.begin(); each != i->second.end(); each++){
            if (each->nextHop == name){
               if (sender) each->sendMax = amount-1;
               else each->recvMax = amount-1;
            }
         }
      }
  }

 void 
 updateRHRTableWeight(std::string name, double amount){
      for (auto i = RHRoutingTable.begin(); i != RHRoutingTable.end(); i++){
         for (auto each = i->second.begin(); each != i->second.end(); each++){
            if (each->path == name){
               each->maxSend -= amount;
            }
         }
      }
  };

 void 
 updateRHRTable(std::string name, double amount){
      for (auto i = RHRoutingTable.begin(); i != RHRoutingTable.end(); i++){
         for (auto each = i->second.begin(); each != i->second.end(); each++){
            if (each->path == name){
               each->maxSend = amount-1;
            }
         }
      }
  };

  //Routing Functions
  void 
  sendRoutingInfo();

  void 
  setFindTable(std::string RH, std::string nextHop){
     findRHTable[RH] = nextHop;
  };

  void 
  setNeighborCredit(std::string name, double amount);

  void 
  setNeighbor(std::string name, Ipv4Address address);

  //Utility Functions
  bool 
  hasHoldRecv(uint32_t txID);

  void 
  insertTimeout(uint32_t txID, std::string src, std::string payload, double te1, double te2, bool sender);

  void 
  checkTimeout();
  
  void 
  checkTe1(uint32_t txID, std::string src, std::string payload, double te1, bool sender);

  void 
  checkTe2(uint32_t txID, std::string src, bool sender);

  Ptr<Socket>
  getSocket(std::string dest);

  uint32_t 
  getHighestAmount(std::string dest);

  std::string 
  getRH();

  std::vector<std::string> 
  getReachableRHs();

  std::vector<std::string>  
  matchUpNonces(std::unordered_map<std::string, std::vector<std::string>> RHlist);

  void 
  setRHTable(std::unordered_map<std::string, std::vector<std::string>> RHlist);

  std::string 
  findNextHop(std::string dest, double amount, bool send);

   //TODO:: Revist this function for optimization
  std::string 
  createPath(std::string dest, double amount);

  std::string 
  readPayload(Ptr<Packet> p);

  std::string 
  findSource(Ptr<Socket> s);

  bool 
  checkOverlap(uint32_t txID, std::string dest, bool sender);

  std::vector<std::string> 
  SplitString( std::string strLine, char delimiter, int max=0 ) {
   std::string str = strLine;
   std::vector<std::string> result;
   uint32_t i =0;
   std::string buildStr = "";
   int total = 0;

   for ( i = 0; i<str.size(); i++) {
      if ( str[i]== delimiter && (total != max || max == 0)) {
         result.push_back( buildStr );
	      buildStr = "";
         total++;
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

  uint16_t m_local_port;
  bool m_running;
  Ipv4Address m_local_ip;
  Ptr<Socket> m_socket;
  uint32_t m_packet_size;
  uint16_t m_peerPort = 5017;
  uint32_t m_seq;
  double m_amount;
  bool m_payer;
  int m_maxRetires = 4;//TODO:: add as varaible


  //BLANCpp Atributes
  bool m_route_helper;
  bool method2 = false;
  std::string m_name;
  std::string nextRH;
  uint32_t m_txID = 0;
  double lastSent = 0;

  std::unordered_map<std::string, std::string> findRHTable;
  std::unordered_map<std::string, std::vector<RoutingEntry>> RoutingTable;
  std::unordered_map<std::string, std::vector<std::string>> NonceTable;
  std::unordered_map<std::string, std::vector<RHRoutingEntry>> RHRoutingTable;

  std::unordered_map<uint32_t, TransactionInfo> txidTable; 
  std::unordered_map<uint32_t, std::vector<TransactionInfo>> overlapTable; 
  std::unordered_map<uint32_t, std::vector<Ptr<Socket>>> srcTrail; 
  std::unordered_map<std::string, Neighbor>  neighborTable; 

  //Transaction Pair atributes
  bool TiP = false;


  //Callbacks
  TracedCallback<uint32_t, Ptr<Packet>, const Address &, uint32_t, uint32_t, Ipv4Address> m_receivedPacket;
  TracedCallback<uint32_t, Ptr<Packet>, const Address &, uint32_t> m_sentPacket;
  TracedCallback<uint32_t, uint32_t, double> m_onFindReply;
  TracedCallback<uint32_t, uint32_t, bool> m_onHold;
  TracedCallback<uint32_t, uint32_t> m_onPay;
  TracedCallback<uint32_t, uint32_t> m_onPayPath;
  TracedCallback<std::string, uint32_t, bool> m_onTx;
  TracedCallback<std::string, std::string, double> m_onPathUpdate;
  TracedCallback<uint32_t> m_onTxFail;
  TracedCallback<uint32_t> m_onTxRetry;


};

} // namespace ns3

#endif /* ICENS_TCP_SERVER */


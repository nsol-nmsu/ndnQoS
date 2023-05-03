/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
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

#include "blanc-header.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ns3.blancHeader");

NS_OBJECT_ENSURE_REGISTERED (blancHeader);

TypeId
blancHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::blancHeader")
    .SetParent<Header> ()
    ;
  return tid;
}

TypeId
blancHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
blancHeader::SetPacketType (uint32_t packetType)
{
  m_packetType = packetType;
}
uint32_t
blancHeader::GetPacketType (void)
{
  return m_packetType;
}



void 
blancHeader::SetDest (uint32_t dest)
{
  m_dest = dest;
}
  
uint32_t 
blancHeader::GetDest (void)
{
  return m_dest;
}

void
blancHeader::SetNxtDest (uint32_t dest)
{
  m_nxtDest = dest;
}

uint32_t
blancHeader::GetNxtDest (void)
{
  return m_nxtDest;
}


void 
blancHeader::SetTIDPrime (uint32_t TID)
{
  m_TID_prime = TID;
}
  
uint32_t 
blancHeader::GetTIDPrime (void)
{
  return m_TID_prime;
}

  
void 
blancHeader::SetTID (uint32_t TID)
{
  m_TID = TID;
}
  
uint32_t 
blancHeader::GetTID (void)
{
  return m_TID;
}

   
void 
blancHeader::SetAmount (double amount)
{
  m_amount = amount;
}

double  
blancHeader::GetAmount (void){
  return m_amount;
}


uint32_t
blancHeader::GetSerializedSize (void) const
{
  //Four bytes of data to store
  return 32;
}
void
blancHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteHtonU32 (m_packetType);
  start.WriteHtonU32 (m_payloadSize);
  start.WriteHtonU32 (m_dest);
  start.WriteHtonU32 (m_nxtDest);
  start.WriteHtonU32 (m_TID_prime);
  start.WriteHtonU32 (m_TID); 
  start.WriteHtonU64 (m_amount);  
}
uint32_t
blancHeader::Deserialize (Buffer::Iterator start)
{
  m_packetType = start.ReadNtohU32 ();
  m_payloadSize = start.ReadNtohU32 ();
  m_dest = start.ReadNtohU32 ();
  m_nxtDest = start.ReadNtohU32 ();
  m_TID_prime = start.ReadNtohU32 ();
  m_TID = start.ReadNtohU32 ();
  m_amount = start.ReadNtohU64 ();
  return 32;
}
void
blancHeader::Print (std::ostream &os) const
{
  os << m_packetType;
}

}//namespace

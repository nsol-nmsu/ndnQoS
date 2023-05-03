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

#ifndef ICENS_HEADER_H
#define ICENS_HEADER_H

#include "ns3/header.h"
#include <stdint.h>

namespace ns3 {

/**
 * \brief A custom header class to add a 2-byte blanc message type information to packets
 *
 */

class blancHeader : public Header
{
public:

  // New methods needed
  static TypeId GetTypeId (void);

  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * \brief Set packet type value of the packet
   *
   * Types are defined as follows.
   *   1: hold
   *   2: hold receive
   *   3: pay
   */
  void SetPacketType (uint32_t data);

  /**
   * \brief Get packet type value of the packet
   *
   */
  uint32_t GetPacketType (void);


  void SetDest (uint32_t data);

  /**
   * \brief Set subscription value of the packet
   *
   */
  uint32_t GetDest (void);

  void SetNxtDest (uint32_t data);

  /**
   * \brief Set subscription value of the packet
   *
   */
  uint32_t GetNxtDest (void);


 
  void SetTIDPrime (uint32_t data);

  /**
   * \brief Set subscription value of the packet
   *
   */
  uint32_t GetTIDPrime (void);

  void SetTID (uint32_t data);

  /**
   * \brief Set subscription value of the packet
   *
   */

  uint32_t GetTID (void);

   void SetAmount (double data);

  /**
   * \brief Set subscription value of the packet
   *
   */
  double  GetAmount (void);

  void setPayloadSize(uint32_t s){
    m_payloadSize = s;
  };

  uint32_t  GetPayloadSize (void){
    return m_payloadSize;
  }; 


  // Overridden methods from NS3 Header Class
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

private:
  uint32_t m_packetType;
  uint32_t m_payloadSize;
  uint32_t m_dest;//REPLACE WITH STRING
  uint32_t m_nxtDest;
  uint32_t m_TID_prime;
  uint32_t m_TID;
  double m_amount;
  
};

} // namespace ns3

#endif /* ICENS_HEADER_H */

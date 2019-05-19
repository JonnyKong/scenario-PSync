/* -*- Mode:C++; c-file-style:"google"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Zhaoning Kong <jonnykong@cs.ucla.edu>
 */

#include "pure-forwarder-node.hpp"

namespace ndn {

PureForwarderNode::PureForwarderNode(uint64_t nid)
  : m_nid(nid)
  , m_scheduler(m_face.getIoService())
  , m_forward_prob(100)
  , m_random_generator(nid)
  , m_collision_random(m_random_generator, boost::uniform_int<>(0, 100000))
  , m_forward_rand(m_random_generator, boost::uniform_int<>(0, 100))
{
}

void
PureForwarderNode::Start()
{
  m_face.setInterestFilter(Name("/ndn/syncNotify"),
                           std::bind(&PureForwarderNode::onInterest, this, _2),
                           [this](const Name&, const std::string& reason) {});
  m_face.setInterestFilter(Name("/ndn/vsyncData"),
                           std::bind(&PureForwarderNode::onInterest, this, _2),
                           [this](const Name&, const std::string& reason) {});
}

void
PureForwarderNode::Stop()
{
}

void
PureForwarderNode::onInterest(const Interest& interest)
{
  if (m_forward_rand() >= m_forward_prob)
    return;

  Interest interest_new(interest.getName(), time::milliseconds(5000));
  interest_new.refreshNonce();
  
  // Add random delay
  int delay = m_collision_random();
  m_scheduler.scheduleEvent(time::microseconds(delay), [this, interest_new] {
    int64_t now = ns3::Simulator::Now().GetMicroSeconds();\
    std::cout << now << " microseconds forwarder(" << m_nid << ") forward interest: "
              << interest_new.getName().toUri() << std::endl;
    m_face.expressInterest(interest_new,
                           std::bind(&PureForwarderNode::onData, this, _1, _2),
                           [](const Interest& interest, const lp::Nack& nack){},
                           [](const Interest& interest){});
  });
}

void
PureForwarderNode::onData(const Interest& interest, const Data& data)
{
  return;
  if (m_forward_rand() >= m_forward_prob)
    return;

  // Add random delay
  int delay = m_collision_random();
  m_scheduler.scheduleEvent(time::microseconds(delay), [this, data] {
    int64_t now = ns3::Simulator::Now().GetMicroSeconds();
    std::cout << now << " microseconds forwarder(" << m_nid << ") forward data: "
              << data.getName().toUri() << std::endl;
    m_face.put(data); 
  });
}


} // namespace ndn
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

#pragma once

#include <iostream>
#include <boost/random.hpp>

#include <ndn-cxx/data.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>
#include <ndn-cxx/security/signing-info.hpp>
#include <ndn-cxx/util/backports.hpp>
#include <ndn-cxx/util/scheduler-scoped-event-id.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <ndn-cxx/util/signal.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>

namespace ndn {

class PureForwarderNode {
public:
  PureForwarderNode(uint64_t nid);

  // Set interest filter to all
  void
  Start();

  void
  Stop();

  // Forward interests with probability.
  void
  onInterest(const Interest& interest);

  // Forward data with probability.
  // Data can be forwarded either by NFD directly, or through the application,
  //  depending on whether a random delay is needed. Corresponding changes have
  //  to be made for the forwarder.
  void
  onData(const Interest& interest, const Data& data);


private:
  uint64_t m_nid;
  Face m_face;
  Scheduler m_scheduler;
  const int m_forward_prob;
  
  boost::mt19937 m_random_generator;
  boost::variate_generator<boost::mt19937&, boost::uniform_int<>> m_collision_random;
  boost::variate_generator<boost::mt19937&, boost::uniform_int<>> m_forward_rand;
};

} // namespace ndn
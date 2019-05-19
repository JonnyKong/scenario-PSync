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

#include "ns3/ndnSIM-module.h"
#include "ns3/integer.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"

#include "ns3/application.h"
#include "ns3/ptr.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"

#include "pure-forwarder-node.hpp"

namespace ns3 {
namespace ndn {

class PureForwarderApp : public Application {
public:
  static TypeId
  GetTypeId()
  {
    static TypeId tid = TypeId("PureForwarderApp")
      .SetParent<Application>()
      .AddConstructor<PureForwarderApp>()
      .AddAttribute("NodeID", "NodeID fsor sync node", UintegerValue(0),
                    MakeUintegerAccessor(&PureForwarderApp::m_nid), MakeUintegerChecker<uint64_t>());
    return tid;
  }

protected:
  // inherited from Application base class.
  virtual void
  StartApplication()
  {
    m_instance.reset(new ::ndn::PureForwarderNode(m_nid));
    m_instance->Start();
  }

  virtual void
  StopApplication()
  {
    m_instance->Stop();
    m_instance.reset();
  }

private:
  std::unique_ptr<::ndn::PureForwarderNode> m_instance;
  uint64_t m_nid;
};

} // namespace ndn
} // namespace ns3

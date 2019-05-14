/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#include "ns3/ndnSIM-module.h"
#include "ns3/integer.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/boolean.h"

#include "psync.hpp"

namespace ns3 {
namespace ndn {


class PSyncApp : public Application
{
public:
  static TypeId
  GetTypeId()
  {
    static TypeId tid = TypeId("PSyncApp")
      .SetParent<Application>()
      .AddConstructor<PSyncApp>()
      .AddAttribute("NodeID", "NodeID for sync node", UintegerValue(0),
                    MakeUintegerAccessor(&PSyncApp::m_nid), MakeUintegerChecker<uint64_t>())
      .AddAttribute("SyncPrefix", "Sync Prefix", StringValue("/"),
                    MakeNameAccessor(&PSyncApp::m_syncPrefix), MakeNameChecker())
      .AddAttribute("DataPrefix", "Data Prefix", StringValue("/"),
                    MakeNameAccessor(&PSyncApp::m_dataPrefix), MakeNameChecker())
      .AddAttribute("UserPrefix", "User Prefix", StringValue("/"),
                    MakeNameAccessor(&PSyncApp::m_userPrefix), MakeNameChecker())
      .AddAttribute("DataGenerationDuration", "Data generation duration", IntegerValue(3),
                    MakeIntegerAccessor(&PSyncApp::m_dataGenerationDuration), MakeIntegerChecker<int>());
    return tid;
  }

protected:
  // inherited from Application base class.
  virtual void
  StartApplication()
  {
    m_instance.reset(new ::ndn::PSync(m_nid, m_syncPrefix, m_dataPrefix,
                                      std::string(m_userPrefix.toUri())));
    m_instance->setDataGenerationDuration(m_dataGenerationDuration);
    m_instance->run();
  }

  virtual void
  StopApplication()
  {
    m_instance.reset();
  }

private:
  std::unique_ptr<::ndn::PSync> m_instance;
  uint64_t m_nid;
  int m_nDataStreams;
  Name m_syncPrefix;
  Name m_dataPrefix;
  Name m_userPrefix;
  int m_dataGenerationDuration;
};


} // namespace ndn
} // namespace ns3

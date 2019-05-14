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

#include <iostream>

#include <ndn-cxx/face.hpp>
#include <boost/random.hpp>
#include <boost/asio/io_service.hpp>
#include <unordered_map>

#include "PSync/full-producer.hpp"


namespace ndn {

class PSync
{
public:
  /**
   * @brief constructor
   *
   * Initialize PSync node for simulation. Each node contains a consumer and
   * producer instance.
   *
   * Default values:
   * Consumer bloom filter expected subscriptions: 80
   * Sync interest lifetime: 1600ms
   * Sync reply freshness: 1600ms
   */
  PSync(uint64_t nid, Name& syncPrefix, Name& dataPrefix,
        const std::string& userPrefix);

  void
  setDataGenerationDuration(const int dataGenerationDuration);

  void
  run();

  void
  processSyncUpdate(const std::vector<psync::MissingDataInfo>& updates);

  void
  sendDataInterest(const Name& data_name);

  void
  onDataInterest(const Name& prefix, const Interest& interest);

  void
  onDataTimeout(const Interest& interest, int nRetries,
                int64_t last, const std::string& reason);

  void
  onDataReply(const Interest& interest, const Data& data);

  void
  doUpdate(const ndn::Name& updateName);

  void
  printNFDTraffic();

private:
  // Logging
  void
  logStateStore(const Name& prefix, uint64_t seqNo);

  void
  logDataStore(const Name& data_name);
  
  uint64_t m_nid;
  Name m_syncPrefix;
  Name m_dataPrefix;
  Name m_userPrefix;
  boost::asio::io_service m_ioService;
  DummyIoService m_dummy_ioService;
  ndn::KeyChain m_keyChain;
  std::unordered_map<Name, std::shared_ptr<Data>> m_data_store;

  Face m_face;
  Scheduler m_scheduler;
  
  int m_dataGenerationDuration;

  boost::mt19937 m_random_generator;
  boost::variate_generator<boost::mt19937&, boost::uniform_int<> > m_data_generation_random;
  boost::variate_generator<boost::mt19937&, boost::uniform_int<> > m_collision_random; 
  boost::variate_generator<boost::mt19937&, boost::uniform_int<> > m_forward_random; 
  
  psync::FullProducer m_fullProducer;
};

} // namespace ndn

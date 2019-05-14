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

#include "psync.hpp"

#include <climits>
#include <assert.h>

namespace ndn {

PSync::PSync(uint64_t nid, Name& syncPrefix, Name& dataPrefix,
             const std::string& userPrefix)
  : m_nid(nid)
  , m_syncPrefix(syncPrefix)
  , m_dataPrefix(dataPrefix)
  , m_userPrefix(userPrefix)
  , m_face(m_dummy_ioService)
  , m_scheduler(m_dummy_ioService)
  , m_random_generator(nid)
  , m_data_generation_random(m_random_generator,
                             boost::uniform_int<>(40000 * 0.9, 40000 * 1.1))
  // , m_data_generation_random(m_random_generator,
  //                            boost::uniform_int<>(10000 * 0.9, 10000 * 1.1))
  , m_collision_random(m_random_generator,
                       boost::uniform_int<>(0, 1000))
  , m_forward_random(m_random_generator,
                     boost::uniform_int<>(0, 100))
  , m_fullProducer(80, m_face, syncPrefix, userPrefix,
                   std::bind(&PSync::processSyncUpdate, this, _1),
                   8000_ms, 1000_ms)
{
  // Init full producer component. Each node will only publish one data
  // stream 0.
  m_fullProducer.setNodeId(m_nid);
  ndn::Name prefix(userPrefix + "-" + ndn::to_string(0));
  m_fullProducer.addUserNode(prefix);
  m_scheduler.scheduleEvent(ndn::time::milliseconds(100 * m_nid),
                            [this, prefix] {
                              doUpdate(prefix);
                            });

  // Application layer is responsible for data fetching
  m_face.setInterestFilter(m_dataPrefix,
                           std::bind(&PSync::onDataInterest, this, _1, _2),
                           [] (const Name& prefix, const std::string& msg) {});

  // Print NFD traffic at the end of simulation
  m_scheduler.scheduleEvent(time::seconds(2395), [this] {
    printNFDTraffic();
  });
}

void
PSync::setDataGenerationDuration(const int dataGenerationDuration)
{
  m_dataGenerationDuration = dataGenerationDuration;
}

void
PSync::run()
{
  int64_t now = ns3::Simulator::Now().GetMicroSeconds();
  std::cout << now << " microseconds node(" << m_nid << ") start running\n";
  m_face.processEvents();
}

void
PSync::processSyncUpdate(const std::vector<psync::MissingDataInfo>& updates) {
  for (const auto& update : updates) {
    for (uint64_t i = update.lowSeq; i <= update.highSeq; i++) {
      // Data can now be fetched using the prefix and sequence
      logStateStore(update.prefix, i);
      
      // Add random delay before fetch data to avoid collision
      int delay = m_collision_random();
      m_scheduler.scheduleEvent(time::milliseconds(delay),
                                [this, update, i] {
        Name data_name(m_dataPrefix);
        data_name.append(update.prefix).append(ndn::to_string(i));
        sendDataInterest(data_name); 
      });
    }
  }
}

void
PSync::sendDataInterest(const Name& data_name)
{
  int64_t now = ns3::Simulator::Now().GetMicroSeconds();
  std::cout << now << " microseconds node(" << m_nid << ") Send Data Interest: "
            << data_name << std::endl;
  
  Interest interest(data_name, time::milliseconds(5000));

  int nRetries = 9;
  m_face.expressInterest(interest,
                         std::bind(&PSync::onDataReply, this, _1, _2),
                         bind(&PSync::onDataTimeout, this, _1, nRetries,
                              now, "NACK"), // Nack
                         bind(&PSync::onDataTimeout, this, _1, nRetries,
                              now, "TIMEOUT"));
}

void
PSync::onDataInterest(const Name& prefix, const Interest& interest)
{
  int64_t now = ns3::Simulator::Now().GetMicroSeconds();
  if (m_data_store.find(interest.getName()) == m_data_store.end()) {
    std::cout << now << " microseconds node(" << m_nid << ") onDataInterest(): "
              << interest.getName().toUri() << " Doesn't have data\n"; 
    
    // Forward data interest with 50% probability
    if (m_forward_random() > 50)
      return;

    int delay = m_collision_random();
    m_scheduler.scheduleEvent(time::milliseconds(delay),
                              [this, interest] {
      int64_t now = ns3::Simulator::Now().GetMicroSeconds();
      std::cout << now << " microseconds node(" << m_nid << ") Send Data Interest "
                << "(Forward): " << interest.getName() << std::endl;
      Interest interest_new_nonce(interest);
      interest_new_nonce.refreshNonce();
      m_face.expressInterest(interest_new_nonce,
                             std::bind(&PSync::onDataReply, this, _1, _2),
                             bind(&PSync::onDataTimeout, this, _1, 0,
                                  now, "NACK"), // Nack
                             bind(&PSync::onDataTimeout, this, _1, 0,
                                  now, "TIMEOUT"));
    });

  } else {
    std::cout << now << " microseconds node(" << m_nid << ") onDataInterest(): "
              << interest.getName().toUri() << " Have data\n"; 
  
    // Add random delay before replying data to avoid collision
    int delay = m_collision_random(); 
    m_scheduler.scheduleEvent(time::milliseconds(delay),
                              [this, interest] {
      int64_t now = ns3::Simulator::Now().GetMicroSeconds();
      m_face.put(*m_data_store[interest.getName()]);
      std::cout << now << " microseconds node(" << m_nid <<  ") Send Data Reply: "
                << m_data_store[interest.getName()]->getName() << std::endl;
    });
  }
}

void
PSync::onDataTimeout(const Interest& interest, int nRetries,
                     int64_t last, const std::string& reason)
{
  int64_t now = ns3::Simulator::Now().GetMicroSeconds();
  if (nRetries <= 0)
    return;

  Interest interest_new_nonce(interest);
  interest_new_nonce.refreshNonce();
  interest_new_nonce.setInterestLifetime(time::milliseconds(5000));

  int delay = m_collision_random();
  
  // Should wait for longer time if received NACK
  if (reason == "NACK" && now < last + 5 * 1000 * 1000)
    delay += 5 * 1000 - (now - last) / 1000;
 
  m_scheduler.scheduleEvent(time::milliseconds(delay),
                            [this, interest_new_nonce, nRetries, reason] {
    
    int64_t now = ns3::Simulator::Now().GetMicroSeconds();
    std::cout << now << " microseconds node(" << m_nid << ") Send Data Interest ("
              << reason << "): " << interest_new_nonce.getName() << std::endl;
    
    m_face.expressInterest(interest_new_nonce,
                           std::bind(&PSync::onDataReply, this, _1, _2),
                           std::bind(&PSync::onDataTimeout, this, _1, nRetries - 1,
                                     now, "NACK"),
                           std::bind(&PSync::onDataTimeout, this, _1, nRetries - 1,
                                     now, "TIMEOUT"));
  });
}

void
PSync::onDataReply(const Interest& interest, const Data& data)
{
  int64_t now = ns3::Simulator::Now().GetMicroSeconds();
  std::cout << now << " microseconds node(" << m_nid << ") onDataReply(): "
            << data.getName() << std::endl;
  
  // Drop duplicate data
  if (m_data_store.find(data.getName()) != m_data_store.end())
    return;
  
  // Store data
  m_data_store[data.getName()] = std::make_shared<Data>(interest.getName());
  m_data_store[data.getName()]->setContent(data.getContent());
  m_keyChain.sign(*m_data_store[data.getName()]);
  logDataStore(data.getName());
}

void
PSync::doUpdate(const ndn::Name& prefix)
{
  int64_t now = ns3::Simulator::Now().GetMicroSeconds();
  if (now > m_dataGenerationDuration * 1000 * 1000) {
    std::cout << now << " microseconds node(" << m_nid << ") Stops data generation\n";
    return;
  }
 
  // Publish an update to this user prefix
  m_fullProducer.publishName(prefix);
  uint64_t seqNo = m_fullProducer.getSeqNo(prefix).value();
    
  // Save data to data store 
  ndn::Name data_name(m_dataPrefix);
  data_name.append(prefix).append(ndn::to_string(seqNo));
  std::string gibberish = std::string(100, '*');
  std::shared_ptr<Data> data = std::make_shared<Data>(data_name);
  data->setFreshnessPeriod(time::seconds(3600)); 
  data->setContent(reinterpret_cast<const uint8_t*>(gibberish.data()),
                   gibberish.size());
  m_keyChain.sign(*data);
  
  m_data_store[data_name] = data;
  logStateStore(prefix, seqNo);
  logDataStore(data_name);
  std::cout << now << " microseconds node(" << m_nid << ") Publish: " 
            << data_name << std::endl;
  
  // Schedule for next data publishing
  m_scheduler.scheduleEvent(ndn::time::milliseconds(m_data_generation_random()),
                            [this, prefix] {
                              doUpdate(prefix);
                            });
}

void
PSync::logStateStore(const Name& prefix, uint64_t seqNo)
{
  int64_t now = ns3::Simulator::Now().GetMicroSeconds();
  std::cout << now << " microseconds node(" << m_nid << ") Update New Seq: "
            << prefix << "/" << seqNo << std::endl;
}

void
PSync::logDataStore(const Name& data_name)
{
  int64_t now = ns3::Simulator::Now().GetMicroSeconds();
  std::cout << now << " microseconds node(" << m_nid << ") Store New Data: "
            << data_name << std::endl;
}

void
PSync::printNFDTraffic()
{
  Interest i("/ndn/getNDNTraffic", time::milliseconds(5));
  m_face.expressInterest(i, [](const Interest&, const Data&) {},
                         [](const Interest&, const lp::Nack&) {},
                         [](const Interest&) {});
}


} // namespace ndn

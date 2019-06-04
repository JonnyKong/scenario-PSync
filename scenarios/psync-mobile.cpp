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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"

#include <cstdio>

namespace ns3 {
namespace ndn {

int
main(int argc, char* argv[])
{
  std::string phyMode("DsssRate11Mbps");
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

  // Set params
  double loss_rate = 0.0;
  CommandLine cmd;
  cmd.AddValue("lossRate", "loss rate", loss_rate);
  cmd.Parse(argc, argv);

  // Default params
  int node_num = 30;
  int range = 60;
  int sim_time = 2400;

  // Wifi
  RngSeedManager::SetRun(0);

  WifiHelper wifi = WifiHelper();
  wifi.SetStandard(WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                               "DataMode", StringValue(phyMode),
                               "ControlMode", StringValue(phyMode));
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel",
                                  "MaxRange", DoubleValue(range));

  YansWifiPhyHelper wifiPhyHelper = YansWifiPhyHelper::Default();
  wifiPhyHelper.SetChannel(wifiChannel.Create());

  WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");

  // Nodes
  NodeContainer nodes;
  nodes.Create(node_num);

  // 1. Install wifi
  NetDeviceContainer wifiNetDevices = wifi.Install(wifiPhyHelper, wifiMac, nodes);
  wifi.AssignStreams(wifiNetDevices, 0);

  // 2. Install mobility
  auto traceFile = "trace/scenario-" + to_string(node_num) + ".ns_movements";
  Ns2MobilityHelper ns2 = Ns2MobilityHelper(traceFile);
  ns2.Install();

  // 3. Install NDN stack
  StackHelper ndnHelper;
  ndnHelper.InstallAll();

  // 4. Set multicast for prefix /psync
  StrategyChoiceHelper::InstallAll("/psyncState", "/localhost/nfd/strategy/multicast");
  StrategyChoiceHelper::InstallAll("/psyncData", "/localhost/nfd/strategy/multicast");

  // 5. Install sync app
  uint64_t idx = 0;
  for (NodeContainer::Iterator i = nodes.Begin(); i != nodes.End(); ++i) {
    Ptr<Node> object = *i;
    Ptr<MobilityModel> position = object->GetObject<MobilityModel>();
    Vector pos = position->GetPosition();
    std::cout << "node " << idx << " x position: " << pos.x << " " << pos.y << std::endl;

    if (idx < 20) {
      AppHelper syncAppHelper("PSyncApp");
      syncAppHelper.SetAttribute("NodeID", UintegerValue(idx));
      syncAppHelper.SetAttribute("SyncPrefix", StringValue("/psyncState"));
      syncAppHelper.SetAttribute("DataPrefix", StringValue("/psyncData"));
      syncAppHelper.SetAttribute("UserPrefix", StringValue(std::string("/peer") + std::to_string(idx)));
      syncAppHelper.SetAttribute("DataGenerationDuration", IntegerValue(800));
      syncAppHelper.Install(object).Start(Seconds(2));
    } else {
      AppHelper pureForwarderAppHelper("PureForwarderApp");
      pureForwarderAppHelper.SetAttribute("NodeID", UintegerValue(idx));
      pureForwarderAppHelper.Install(object).Start(Seconds(2));
    }

    StackHelper::setLossRate(loss_rate, object);
    StackHelper::setNodeID(idx, object);

    // Add route to all nodes
    FibHelper::AddRoute(object, "/psyncState", std::numeric_limits<int32_t>::max());
    FibHelper::AddRoute(object, "/psyncData", std::numeric_limits<int32_t>::max());
    idx++;
  }

  Simulator::Stop(Seconds(sim_time));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}



} // namespace ndn
} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::ndn::main(argc, argv);
}

/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 *
 * Author: Jaume Nin <jaume.nin@cttc.cat>
 */


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/csma-helper.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/epc-helper.h"
//#include "ns3/gtk-config-store.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("EpcGtpUTunnelExample");
int
main (int argc, char *argv[])
{
  Packet::EnablePrinting ();

  // Command line arguments
  CommandLine cmd;
  cmd.Parse (argc, argv);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // parse again so you can override default values from the command line
  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create nodes.");
  NodeContainer c;
  c.Create (2);

  InternetStackHelper internet;
  internet.Install (c);

  // Create the channels first
  NS_LOG_INFO ("Create channels.");

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
  NetDeviceContainer nc = csma.Install (c);

  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ic = ipv4.Assign (nc);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("epc-gtp.tr"));
  csma.EnablePcapAll ("epc-gtp");

  EpcHelper epcHelper;

  epcHelper.InstallGtpu (c.Get (0));
  epcHelper.InstallGtpu (c.Get (1));

  epcHelper.CreateGtpuTunnel (c.Get (0), ic.GetAddress (0),  c.Get (1), ic.GetAddress (1));

  NS_LOG_INFO ("Create Applications.");
  // Set up some default values for the simulation.
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (200));
  Config::SetDefault ("ns3::OnOffApplication::OnTime", RandomVariableValue (ConstantVariable (1)));
  Config::SetDefault ("ns3::OnOffApplication::OffTime", RandomVariableValue (ConstantVariable (0)));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("2kbps"));
  OnOffHelper onoffAB = OnOffHelper ("ns3::Ipv4RawSocketFactory", InetSocketAddress (Ipv4Address ("100.0.0.2")));
  OnOffHelper onoffBA = OnOffHelper ("ns3::Ipv4RawSocketFactory", InetSocketAddress (Ipv4Address ("100.0.0.1")));
  PacketSinkHelper sinkA ("ns3::Ipv4RawSocketFactory", InetSocketAddress (Ipv4Address ("100.0.0.1")));
  PacketSinkHelper sinkB ("ns3::Ipv4RawSocketFactory", InetSocketAddress (Ipv4Address ("100.0.0.2")));

  ApplicationContainer apps = onoffAB.Install (c.Get (0));
  apps.Add (onoffBA.Install (c.Get (1)));
  apps.Add (sinkA.Install (c.Get (0)));
  apps.Add (sinkB.Install (c.Get (1)));

  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();

  /*GtkConfigStore config;
  config.ConfigureAttributes ();*/

  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
}

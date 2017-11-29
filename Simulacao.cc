#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

// Default Network Topology
//  Refer to Topologia.png
//    Dashed simple line indicates WiFi connection
//    Dashed double line indicates cabled p2p connection
//    Solid simple line indicates LAN Ethernet connection
//    Blue node is server, all others (but border nodes in WiFi LANs) are client
//    Server is just an echo

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("simulacao");

int main (int argc, char* argv[]) {
	bool verbose = true;

	CommandLine cmd;
	cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

	cmd.Parse (argc,argv);

	if (verbose) {
		LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
		LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
	}

	NodeContainer p2pNodes;
	p2pNodes.Create (6); // 6 LANs

	// Como se trata de WAN, vamos usar valores maiores
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("300Mbps"));
	pointToPoint.SetChannelAttribute ("Delay", StringValue ("500us"));

	NetDeviceContainer p2pDevices;
	p2pDevices = pointToPoint.Install (p2pNodes.Get (0), p2pNodes.Get (1));
    p2pDevices = pointToPoint.Install (p2pNodes.Get (1), p2pNodes.Get (2));
    p2pDevices = pointToPoint.Install (p2pNodes.Get (2), p2pNodes.Get (3));
    p2pDevices = pointToPoint.Install (p2pNodes.Get (3), p2pNodes.Get (4));
    p2pDevices = pointToPoint.Install (p2pNodes.Get (4), p2pNodes.Get (5));

	// Fazendo todos os nos de LAN Ethernet
	NodeContainer csmaNodes[4];
	for (int i = 0; i < 4; i++ ) {
		// Adicionando nos de borda (1 por rede)
		csmaNodes[i].Add (p2pNodes.Get (i));
		// Adicionando nos restantes
		csmaNodes[i].Create (9);
	}

	// 4 LANs Ethernet
	CsmaHelper csma[4];
	csma[0].SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
	csma[0].SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
	csma[1].SetChannelAttribute ("DataRate", StringValue ("120Mbps"));
	csma[1].SetChannelAttribute ("Delay", TimeValue (NanoSeconds (2370)));
	csma[2].SetChannelAttribute ("DataRate", StringValue ("10Mbps"));
	csma[2].SetChannelAttribute ("Delay", StringValue("1ms"));
	csma[3].SetChannelAttribute ("DataRate", StringValue ("300Mbps"));
	csma[3].SetChannelAttribute ("Delay", TimeValue (NanoSeconds (280)));

	NetDeviceContainer csmaDevices[4];
	for (int i = 0; i < 4; i++ ) {
		csmaDevices[i] = csma[i].Install (csmaNodes[i]);
	}

	// 2 LANs Wifi
	NodeContainer wifiStaNodes[2];
	NodeContainer wifiApNode[2];

	for (int i = 0; i < 2; i++ ) {
		// Adicionando nos de borda (1 por rede)
		wifiApNode[i] = p2pNodes.Get (i+4);
		// Adicionando nos restantes
		wifiStaNodes[i].Create (9);
	}

	YansWifiChannelHelper channel[2];
	YansWifiPhyHelper phy[2];
	WifiHelper wifi[2];
	NqosWifiMacHelper mac[2];
	Ssid ssid[2];
	NetDeviceContainer staDevices[2];
	NetDeviceContainer apDevices[2];
	MobilityHelper mobility[2];

	for (int i = 0; i < 2; i++) {
		channel[i] = YansWifiChannelHelper::Default ();
		phy[i] = YansWifiPhyHelper::Default ();

		phy[i].SetChannel (channel[i].Create ());

		wifi[i] = WifiHelper();
		wifi[i].SetRemoteStationManager ("ns3::AarfWifiManager");

		mac[i] = NqosWifiMacHelper::Default ();

		ssid[i] = Ssid ("ns-3-ssid");
		mac[i].SetType ("ns3::StaWifiMac",
			"Ssid", SsidValue (ssid[i]),
			"ActiveProbing", BooleanValue (false)
		);

		staDevices[i] = wifi[i].Install (phy[i], mac[i], wifiStaNodes[i]);

		mac[i].SetType ("ns3::ApWifiMac",
			"Ssid", SsidValue (ssid[i]),
			"BeaconGeneration", BooleanValue (true),
			"BeaconInterval", TimeValue (Seconds (2.5))
		);

		apDevices[i] = wifi[i].Install (phy[i], mac[i], wifiApNode[i]);

		mobility[i].SetPositionAllocator ("ns3::GridPositionAllocator",
			"MinX", DoubleValue (0.0),
			"MinY", DoubleValue (0.0),
			"DeltaX", DoubleValue (5.0),
			"DeltaY", DoubleValue (10.0),
			"GridWidth", UintegerValue (3),
			"LayoutType", StringValue ("RowFirst")
		);

		mobility[i].SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
			"Bounds", RectangleValue (Rectangle (-50, 50, -50, 50))
		);

		mobility[i].Install (wifiStaNodes[i]);

		mobility[i].SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		mobility[i].Install (wifiApNode[i]);
	}

	InternetStackHelper stack;
	stack.Install (csmaNodes[0]);
	stack.Install (csmaNodes[1]);
	stack.Install (csmaNodes[2]);
	stack.Install (csmaNodes[3]);
	stack.Install (wifiApNode[0]);
	stack.Install (wifiApNode[1]);
	stack.Install (wifiStaNodes[0]);
	stack.Install (wifiStaNodes[1]);

	// Configurando IPv4
	Ipv4AddressHelper address;

	address.SetBase ("10.2.1.0", "255.255.255.0");
	Ipv4InterfaceContainer p2pInterfaces;
	p2pInterfaces = address.Assign (p2pDevices);

	Ipv4InterfaceContainer csmaInterfaces[4];
	address.SetBase ("10.1.1.0", "255.255.255.0");
	csmaInterfaces[0] = address.Assign (csmaDevices[0]);

	address.SetBase ("10.1.2.0", "255.255.255.0");
	csmaInterfaces[1] = address.Assign (csmaDevices[1]);

	address.SetBase ("10.1.3.0", "255.255.255.0");
	csmaInterfaces[2] = address.Assign (csmaDevices[2]);

	address.SetBase ("10.1.4.0", "255.255.255.0");
	csmaInterfaces[3] = address.Assign (csmaDevices[3]);

	address.SetBase ("10.1.5.0", "255.255.255.0");
	address.Assign (staDevices[0]);
	address.Assign (apDevices[0]);

	address.SetBase ("10.1.6.0", "255.255.255.0");
	address.Assign (staDevices[1]);
	address.Assign (apDevices[1]);

	// Configurando aplicação
	UdpEchoServerHelper echoServer (9);

	ApplicationContainer serverApps = echoServer.Install (csmaNodes[3].Get (0));
	serverApps.Start (Seconds (1.0));
	serverApps.Stop (Seconds (10.0));

	UdpEchoClientHelper echoClient (csmaInterfaces[3].GetAddress (0), 9);
	echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
	echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.)));
	echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

	ApplicationContainer clientApps[57];
	int x = 0;
	for (int i = 0; i < 4; i++) {
		for (int n = 0; n < 10; n++) {
			if (3 == i && n == 0) {
				continue;
			}

			clientApps[x] = echoClient.Install (csmaNodes[i].Get (n));
			clientApps[x].Start (Seconds (2.0));
			clientApps[x++].Stop (Seconds (10.0));
		}
	}

	for (int i = 0; i < 2; i++) {
		for (int n = 1; n < 9; n++) {
			clientApps[x] = echoClient.Install (wifiStaNodes[i].Get (n));
			clientApps[x].Start (Seconds (2.0));
			clientApps[x++].Stop (Seconds (10.0));
		}
	}

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	Simulator::Stop (Seconds (10.0));

	// Dados para analise no Wireshark sao tudo que passar pela rede P2P e pelos nos de borda
	pointToPoint.EnablePcapAll ("simulacao");
	phy[0].EnablePcap ("simulacao", apDevices[0].Get (0));
	phy[1].EnablePcap ("simulacao", apDevices[1].Get (0));
	csma[0].EnablePcap ("simulacao", csmaDevices[0].Get (0), true);
	csma[1].EnablePcap ("simulacao", csmaDevices[1].Get (0), true);
	csma[2].EnablePcap ("simulacao", csmaDevices[2].Get (0), true);
	csma[3].EnablePcap ("simulacao", csmaDevices[3].Get (0), true);

	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}

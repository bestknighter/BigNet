#include "ns3/core-module.h"
#include "ns3/simulator-module.h"
#include "ns3/node-module.h"
#include "ns3/helper-module.h"
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
//    Blue node is server, all others are client
//    Server us just an echo

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiExample");

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
	p2pDevices = pointToPoint.Install (p2pNodes);

	// Fazendo todos os nos de LAN Ethernet
	NodeContainer csmaNodes[4];
	for (int i = 0; i < 4; i++ ) {
		// Adicionando nos de borda (1 por rede)
		csmaNodes[i].Add (p2pNodes.Get (i));
		// Adicionando nos restantes
		csmaNodes[i].Create (9);
	}

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

	// TODO: resto do projeto XD


	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}
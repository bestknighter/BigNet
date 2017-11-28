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
	uint32_t nCsma = 3;
	uint32_t nWifi = 3;

	CommandLine cmd;
	cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
	cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
	cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

	cmd.Parse (argc,argv);

	if (verbose) {
		LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
		LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
	}


	// TODO: resto do projeto XD


	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("VehicleDemo");

int main(int argc,char *argv[]){
	LogComponentEnable("VehicleDemo",LOG_LEVEL_ALL);
	//LogComponentEnable("VehicleApplication",LOG_LEVEL_ALL);
	LogComponentEnable("Controller",LOG_LEVEL_ALL);
	Address serverAddress;
	NS_LOG_INFO("Create Nodes");
	NodeContainer n;
	n.Create(4);

	InternetStackHelper internet;
	internet.Install(n);
	NS_LOG_INFO("Create channels");
	CsmaHelper csma;
	csma.SetChannelAttribute("DataRate",DataRateValue(DataRate("5Mbps")));
	csma.SetChannelAttribute("Delay",TimeValue(MilliSeconds(2)));
	csma.SetDeviceAttribute("Mtu",UintegerValue(1400));
	NetDeviceContainer d= csma.Install(n);

	Ipv4AddressHelper ipv4;
	ipv4.SetBase("10.1.1.0","255.255.255.0");
	Ipv4InterfaceContainer i = ipv4.Assign(d);

	serverAddress = Address(i.GetAddress(3));
	NS_LOG_INFO("Create Applications");
	uint16_t port = 80;
	UdpEchoServerHelper server(port);
	/*ApplicationContainer apps =server.Install(n.Get(3));
	apps.Start(Seconds(1.0));
	apps.Stop(Seconds(10.0));*/
	ApplicationContainer apps;
	uint32_t packetSize = 1024;
	uint32_t maxPacketCount = 1;
	Time interPacketInterval = Seconds(1.0);
	VehicleHelper vehicle(Ipv4Address("255.255.255.255"),port);
	vehicle.SetAttribute("MaxPackets",UintegerValue(maxPacketCount));
	vehicle.SetAttribute("Interval",TimeValue(interPacketInterval));
	vehicle.SetAttribute("PacketSize",UintegerValue(packetSize));
	NodeContainer client;
	client.Add(n.Get(0));
	client.Add(n.Get(1));
	client.Add(n.Get(2));
	apps = vehicle.Install(client);
	vehicle.SetLane(apps.Get(0),0);
	vehicle.SetLane(apps.Get(1),4);
	vehicle.SetLane(apps.Get(2),7);
	apps.Start(Seconds(2.0));
	apps.Stop(Seconds(20));
	NodeContainer controller_node;
	controller_node.Add(n.Get(3));
	ControllerHelper controller(port);
	apps = controller.Install(controller_node);
	apps.Start(Seconds(2.0));
	apps.Stop(Seconds(20));
	Simulator::Run();
	Simulator::Destroy();
	NS_LOG_INFO("Done");

}

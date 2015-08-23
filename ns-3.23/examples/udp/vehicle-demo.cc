#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"

#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/config-store-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("VehicleDemo");


int main(int argc,char *argv[]){
	LogComponentEnable("VehicleDemo",LOG_LEVEL_ALL);

	uint16_t totalVehicles = 100;
	uint16_t vehiclesPerMinute = 8;
	CommandLine cmd;
	bool verboseVehicle=false,verboseController=false;
	cmd.AddValue("totalVehicles","Total number of vehicles to be created",totalVehicles);
	cmd.AddValue("vpm","Vehicle per minute",vehiclesPerMinute);
	cmd.AddValue("verboseVehicles","Logs enablement for vehicle",verboseVehicle);
	cmd.AddValue("verboseController","Logs enablement for controller",verboseController);
	cmd.Parse(argc,argv);
	if(verboseVehicle)
		LogComponentEnable("VehicleApplication",LOG_LEVEL_ALL);
	if(verboseController)
		LogComponentEnable("Controller",LOG_LEVEL_ALL);
	std::cout<<"total vehicles"<<totalVehicles<<std::endl;
	NS_LOG_INFO("Create Nodes");
	CsmaHelper csma;
	NodeContainer n;
	n.Create(totalVehicles);

	InternetStackHelper internet;
	internet.Install(n);
	NS_LOG_INFO("Create channels");

	csma.SetChannelAttribute("DataRate",DataRateValue(DataRate("5Mbps")));
	csma.SetChannelAttribute("Delay",TimeValue(MilliSeconds(2)));
	csma.SetDeviceAttribute("Mtu",UintegerValue(1400));
	NetDeviceContainer d= csma.Install(n);

	Ipv4AddressHelper ipv4;
	ipv4.SetBase("10.1.0.0","255.255.0.0");
	Ipv4InterfaceContainer i = ipv4.Assign(d);

	/*serverAddress = Address(i.GetAddress(3));*/
	NS_LOG_INFO("Create Applications");
	uint16_t port = 80;
/*	UdpEchoServerHelper server(port);
	ApplicationContainer apps =server.Install(n.Get(3));
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
	for(int i=1;i<totalVehicles;i++){
		client.Add(n.Get(i));
	}

	apps = vehicle.Install(client);

	for(int i=0;i<totalVehicles-1;i++){
		vehicle.SetLane(apps.Get(i),i%8);
	}
	apps.Start(Seconds(2.0));
	//apps.Stop(Seconds(20));
	NodeContainer controller_node;
	controller_node.Add(n.Get(0));
	ControllerHelper controller(port);
	apps = controller.Install(controller_node);
	apps.Start(Seconds(2.0));
	//apps.Stop(Seconds(20));
	Simulator::Run();
	Simulator::Destroy();
	NS_LOG_INFO("Done");

}

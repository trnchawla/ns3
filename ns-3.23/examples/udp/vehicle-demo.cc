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
	uint16_t vehiclesPerMinuteL0 = 8;
	uint16_t vehiclesPerMinuteL1 = 8;
	uint16_t vehiclesPerMinuteL2 = 8;
	uint16_t vehiclesPerMinuteL3 = 8;
	uint16_t vehiclesPerMinuteL4 = 8;
	uint16_t vehiclesPerMinuteL5 = 8;
	uint16_t vehiclesPerMinuteL6 = 8;
	uint16_t vehiclesPerMinuteL7 = 8;

	uint16_t np = 3;
	CommandLine cmd;
	bool verboseVehicle=false,verboseController=false;
	cmd.AddValue("totalVehicles","Total number of vehicles to be created",totalVehicles);
	cmd.AddValue("l0","Vehicle per minute Lane 0",vehiclesPerMinuteL0);
	cmd.AddValue("l1","Vehicle per minute Lane 1",vehiclesPerMinuteL1);
	cmd.AddValue("l2","Vehicle per minute Lane 2",vehiclesPerMinuteL2);
	cmd.AddValue("l3","Vehicle per minute Lane 3",vehiclesPerMinuteL3);
	cmd.AddValue("l4","Vehicle per minute Lane 4",vehiclesPerMinuteL4);
	cmd.AddValue("l5","Vehicle per minute Lane 5",vehiclesPerMinuteL5);
	cmd.AddValue("l6","Vehicle per minute Lane 6",vehiclesPerMinuteL6);
	cmd.AddValue("l7","Vehicle per minute Lane 7",vehiclesPerMinuteL7);
	cmd.AddValue("np","Number of vehicle can be passed", np);
	cmd.AddValue("verboseVehicles","Logs enablement for vehicle",verboseVehicle);
	cmd.AddValue("verboseController","Logs enablement for controller",verboseController);
	cmd.Parse(argc,argv);
	if(verboseVehicle)
		LogComponentEnable("VehicleApplication",LOG_LEVEL_ALL);
	if(verboseController)
		LogComponentEnable("Controller",LOG_LEVEL_ALL);
	std::cout<<"total vehicles"<<totalVehicles<<std::endl;
	NS_LOG_INFO("TotalVehicles "<<totalVehicles<<" vpm = "<<vehiclesPerMinuteL0<<" np= "<<np);
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
	vehicle.SetVPM(apps.Get(0),0,vehiclesPerMinuteL0);
	vehicle.SetVPM(apps.Get(0),1,vehiclesPerMinuteL1);
	vehicle.SetVPM(apps.Get(0),2,vehiclesPerMinuteL2);
	vehicle.SetVPM(apps.Get(0),3,vehiclesPerMinuteL3);
	vehicle.SetVPM(apps.Get(0),4,vehiclesPerMinuteL4);
	vehicle.SetVPM(apps.Get(0),5,vehiclesPerMinuteL5);
	vehicle.SetVPM(apps.Get(0),6,vehiclesPerMinuteL6);
	vehicle.SetVPM(apps.Get(0),7,vehiclesPerMinuteL7);

	apps.Start(Seconds(2.0));
	//apps.Stop(Seconds(20));
	NodeContainer controller_node;
	controller_node.Add(n.Get(0));
	ControllerHelper controller(port);
	apps = controller.Install(controller_node);
	controller.SetNp(apps.Get(0),np);
	apps.Start(Seconds(2.0));
	//apps.Stop(Seconds(20));
	Simulator::Run();
	Simulator::Destroy();
	NS_LOG_INFO("Done");

}

#include <iostream>
#include <string>
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
using namespace ns3;
using namespace std;

//                                                      _____server
//                                                csma |_____
//                                     ____router2 ----|     WS
// WS0 ---- router0 ----- router1 ----|                |_____printer
//                                    |____router3


//Nodelist indexing
// WS1[host[0]/Nodelist0] Router1[routers[0]/Nodelist/1] Router2[routers[1]/Nodelist/2] Router3[routers[2]/Nodelist/3] Routers4[routers[3]/Nodelist4] Fileserver[branch[0]/Nodelist5] workstation[branch[1]/Nodelist6] printer[branch[2]/Nodelist7]

//net device indexing
// For router2 -- LoopbackNetDevice[Index/0/DeviceList/0] PointToPointDevice[Index/1/DeviceList/1] PointToPointNetDevice[Index/2/DeviceList1] PointToPointNetDevice[Index/3/DeviceList/3]
NS_LOG_COMPONENT_DEFINE ("Tutorial");
int main(int argc, char *argv[]){
	  // Take logs
	  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
	  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
	  LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
	  //LogComponentEnable ("PacketSinkApplication", LOG_LEVEL_INFO);
	  //LogComponentEnable ("UdpClientApplication", LOG_LEVEL_INFO);
	  //LogComponentEnable ("UdpServerApplication", LOG_LEVEL_INFO);
	//command line variables
	string speed = "10Mbps";
	CommandLine cmd;
	cmd.AddValue("DefaultRate","Default data rate to be used on network devices",speed);
	cmd.Parse(argc,argv);
	cout<<"Speed = "<<speed<<endl;
	DataRate rate(speed);
	Time delay("2ms");

	Config::SetDefault("ns3::PointToPointNetDevice::DataRate",DataRateValue(rate));
	Config::SetDefault("ns3::PointToPointChannel::Delay",TimeValue(delay));

	Config::SetDefault("ns3::CsmaChannel::DataRate",DataRateValue(rate));
	Config::SetDefault("ns3::CsmaChannel::Delay",TimeValue(delay));

	NodeContainer host;
	NodeContainer routers;
	NodeContainer branch;

	host.Create(1);
	routers.Create(4);
	branch.Create(3);

	InternetStackHelper helper;
	helper.Install(host);
	helper.Install(routers);
	helper.Install(branch);

	PointToPointHelper p2p;

	CsmaHelper csma;

	//configure the data rate for subnet5 csma channel
	//csma.SetDeviceAttribute("DataRate",StringValue("100Mbps"));

	//configure subnet1
	NodeContainer subnet1;
	subnet1.Add(host.Get(0));
	subnet1.Add(routers.Get(0));

	NetDeviceContainer subnet1device;
	subnet1device = p2p.Install(subnet1);

	Ipv4AddressHelper address;
	address.SetBase("10.0.1.0","255.255.255.0");
	Ipv4InterfaceContainer subnet1interface = address.Assign(subnet1device);

	//configure subnet2
	NodeContainer subnet2;
	subnet2.Add(routers.Get(0));
	subnet2.Add(routers.Get(1));

	//Configure atttributes specific to subnet2
	DataRate rate_subnet2("50Mbps");
	Time delay_subnet2(MilliSeconds(4));

	NetDeviceContainer subnet2device;
	subnet2device = p2p.Install(subnet2);

	address.SetBase("10.0.2.0","255.255.255.0");
	Ipv4InterfaceContainer subnet2interface = address.Assign(subnet2device);

	Config::Set("/NodeList/1/DeviceList/2/$ns3::PointToPointNetDevice/DataRate",DataRateValue(rate_subnet2));
	Config::Set("/NodeList/2/DeviceList/1/$ns3::PointToPointNetDevice/DataRate",DataRateValue(rate_subnet2));

	Config::Set("ChannelList/1/$ns3::PointToPointChannel/Delay",TimeValue(delay_subnet2));

	//configure subnet3
	NodeContainer subnet3;
	subnet3.Add(routers.Get(1));
	subnet3.Add(routers.Get(2));

	NetDeviceContainer subnet3device;
	subnet3device = p2p.Install(subnet3);

	address.SetBase("10.0.3.0","255.255.255.0");
	Ipv4InterfaceContainer subnet3interface = address.Assign(subnet3device);

	//configure subnet4
	NodeContainer subnet4;
	subnet4.Add(routers.Get(1));
	subnet4.Add(routers.Get(3));

	NetDeviceContainer subnet4device;
	subnet4device = p2p.Install(subnet4);

	//Configure the netdevices on subnet4
	Ptr<NetDevice> deviceA = subnet4device.Get(0);
	Ptr<NetDevice> deviceB = subnet4device.Get(1);
	NetDevice * deviceA_ptr = PeekPointer(deviceA);
	NetDevice * deviceB_ptr = PeekPointer(deviceB);
	PointToPointNetDevice * p2pDeviceA = dynamic_cast<PointToPointNetDevice * >(deviceA_ptr);
	PointToPointNetDevice * p2pDeviceB = dynamic_cast<PointToPointNetDevice * >(deviceB_ptr);
	DataRate rate_subnet4("100Mbps");
	p2pDeviceA->SetAttribute("DataRate",DataRateValue(rate_subnet4));
	p2pDeviceB->SetAttribute("DataRate",DataRateValue(rate_subnet4));

	//configure the channel on subnet 4
	Ptr<Channel> channel = ChannelList::GetChannel(3);
	Channel * channel_subnet4 = PeekPointer(channel);
	Time delay_subnet4("5ms");
	PointToPointChannel * p2pchannel_subnet4 = dynamic_cast<PointToPointChannel *>(channel_subnet4);
	p2pchannel_subnet4->SetAttribute("Delay",TimeValue(delay_subnet4));

	address.SetBase("10.0.4.0","255.255.255.0");
	Ipv4InterfaceContainer subnet4interface = address.Assign(subnet4device);

	//Configure subnet5
	NodeContainer subnet5;
	subnet5.Add(routers.Get(2));
	subnet5.Add(branch);

	NetDeviceContainer subnet5device;
	subnet5device = csma.Install(subnet5);
	channel = ChannelList::GetChannel(4);
	Channel * channel_subnet5 = PeekPointer(channel);
	CsmaChannel * csmaChannel_subnet5 = dynamic_cast<CsmaChannel * >(channel_subnet5);
	DataRate rate_channel5("100Mbps");
	csmaChannel_subnet5->SetAttribute("DataRate",DataRateValue(rate_channel5));
	address.SetBase("10.0.5.0","255.255.255.0");
	Ipv4InterfaceContainer subnet5interface = address.Assign(subnet5device);

	//Adding applications
	Ipv4Address FS_Address(subnet5interface.GetAddress(1));
	uint16_t FS_port = 4500;

	UdpEchoClientHelper WS1Echo(FS_Address,FS_port);
	ApplicationContainer WS1EchoApp = WS1Echo.Install(host.Get(0));
	WS1EchoApp.Start(Seconds(1.0));
	WS1EchoApp.Stop(Seconds(10.0));

	UdpEchoServerHelper FS(FS_port);
	ApplicationContainer FSEchoApp = FS.Install(subnet5.Get(1));
	FSEchoApp.Start(Seconds(0.5));
	FSEchoApp.Stop(Seconds(10));

	Ipv4Address WS2_Address(subnet5interface.GetAddress(2));
	uint16_t WS2_Port = 7250;

	InetSocketAddress inet(WS2_Address,WS2_Port);
	OnOffHelper WS1_onoff("ns3::UdpSocketFactory",inet);
	ApplicationContainer WS1_onoffapp = WS1_onoff.Install(host.Get(0));
	WS1_onoffapp.Start(Seconds(1.0));
	WS1_onoffapp.Stop(Seconds(10.0));

	PacketSinkHelper WS2_sink("ns3::UdpSocketFactory",inet);
	ApplicationContainer WS2SinkApp = WS2_sink.Install(branch.Get(1));
	WS2SinkApp.Start(Seconds(0.5));
	WS2SinkApp.Stop(Seconds(10.0));

	Ipv4Address PrtAddress(subnet5interface.GetAddress(3));
	uint16_t PrtPort = 2560;

	UdpClientHelper WS2Udp(PrtAddress,PrtPort);
	ApplicationContainer WS2UdpApp = WS2Udp.Install(branch.Get(1));
	WS2UdpApp.Start(Seconds(1.0));
	WS2UdpApp.Stop(Seconds(10.0));

	UdpServerHelper Prt(PrtPort);
	ApplicationContainer PrtApp = Prt.Install(branch.Get(2));
	PrtApp.Start(Seconds(1.0));
	PrtApp.Stop(Seconds(10.0));


	Simulator::Run();
	Simulator::Destroy();

}

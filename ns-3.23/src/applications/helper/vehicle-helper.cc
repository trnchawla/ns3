#include "ns3/applications-module.h"
namespace ns3{
VehicleHelper::VehicleHelper(Address address, uint16_t port){
	m_factory.SetTypeId(Vehicle::GetTypeId());
	SetAttribute("RemoteAddress",AddressValue(address));
	SetAttribute("RemotePort",UintegerValue(port));
}
VehicleHelper::VehicleHelper(Ipv4Address address,uint16_t port){
	m_factory.SetTypeId(Vehicle::GetTypeId());
	SetAttribute("RemoteAddress",AddressValue(address));
	SetAttribute("RemotePort",UintegerValue(port));
	}
VehicleHelper::VehicleHelper(Ipv6Address address,uint16_t port){
	m_factory.SetTypeId(Vehicle::GetTypeId());
	SetAttribute("RemoteAddress",AddressValue(address));
	SetAttribute("RemotePort",UintegerValue(port));
}
void VehicleHelper::SetAttribute(std::string name,const AttributeValue &value){
	m_factory.Set(name,value);
}
void VehicleHelper::SetFill(Ptr<Application> app,std::string fill){
	app->GetObject<Vehicle>()->setFill(fill);
}
void VehicleHelper::SetFill(Ptr<Application> app,uint8_t fill,uint32_t dataLength){
	app->GetObject<Vehicle>()->setFill(fill,dataLength);
}
void VehicleHelper::SetFill(Ptr<Application> app,uint8_t * fill,uint32_t fillLength,uint32_t dataLength){
	app->GetObject<Vehicle>()->setFill(fill,fillLength,dataLength);
}
void VehicleHelper::SetLane(Ptr<Application> app, uint16_t laneNumber){
	app->GetObject<Vehicle>()->setLane(laneNumber);
}
ApplicationContainer VehicleHelper::Install(Ptr<Node> node) const{
	return ApplicationContainer(InstallPriv(node));
}
ApplicationContainer VehicleHelper::Install(std::string nodeName) const{
	Ptr<Node> node = Names::Find<Node>(nodeName);
	return ApplicationContainer(InstallPriv(node));
}
ApplicationContainer VehicleHelper::Install(NodeContainer c) const{
	ApplicationContainer apps;
	for(NodeContainer::Iterator i=c.Begin();i!=c.End();++i){
		apps.Add(InstallPriv(*i));
	}
	return apps;
}
Ptr<Application> VehicleHelper::InstallPriv(Ptr<Node> node) const{
	Ptr<Application> app = m_factory.Create<Vehicle>();
	node->AddApplication(app);
	return app;
}
}

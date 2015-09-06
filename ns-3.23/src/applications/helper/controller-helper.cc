#include "ns3/applications-module.h"
namespace ns3{
ControllerHelper::ControllerHelper(uint16_t port){
	m_factory.SetTypeId(Controller::GetTypeId());
	SetAttribute("RemotePort", UintegerValue(port));
}
void ControllerHelper::SetAttribute(std::string name,const AttributeValue &value){
	m_factory.Set(name,value);
}
ApplicationContainer ControllerHelper::Install(Ptr<Node> node) const{
	return ApplicationContainer(InstallPriv(node));
}
ApplicationContainer ControllerHelper::Install(std::string nodeName) const{
	Ptr<Node> node = Names::Find<Node>(nodeName);
	return ApplicationContainer(InstallPriv(node));
}
void ControllerHelper::SetNp(Ptr<Application> app, uint16_t np){
	app->GetObject<Controller>()->setNp(np);
}
ApplicationContainer ControllerHelper::Install(NodeContainer c) const{
	ApplicationContainer apps;
	for(NodeContainer::Iterator i = c.Begin();i!= c.End(); ++i){
		apps.Add(InstallPriv(*i));
	}
	return apps;
}
Ptr<Application> ControllerHelper::InstallPriv(Ptr<Node> node) const{
	Ptr<Application> app = m_factory.Create<Controller>();
	node->AddApplication(app);
	return app;
}
}

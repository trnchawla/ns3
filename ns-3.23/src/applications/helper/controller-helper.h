/*
 * controller-helper.h
 *
 *  Created on: 22-Aug-2015
 *      Author: tarun
 */
#include "ns3/network-module.h"
#include "ns3/core-module.h"
#ifndef SRC_APPLICATIONS_HELPER_CONTROLLER_HELPER_H_
#define SRC_APPLICATIONS_HELPER_CONTROLLER_HELPER_H_
namespace ns3{
class ControllerHelper{
public:
	ControllerHelper(uint16_t port);
	ApplicationContainer Install(Ptr<Node> node) const;
	ApplicationContainer Install(std::string nodeName) const;
	ApplicationContainer Install(NodeContainer c) const;
	void SetAttribute(std::string name, const AttributeValue &value);
private:
	Ptr<Application> InstallPriv(Ptr<Node> node) const;
	ObjectFactory m_factory;
};
}
#endif /* SRC_APPLICATIONS_HELPER_CONTROLLER_HELPER_H_ */

/*
 * vehicle-helper.h
 *
 *  Created on: 15-Aug-2015
 *      Author: tarun
 */
#include "ns3/network-module.h"
#include "ns3/core-module.h"
#ifndef SRC_APPLICATIONS_HELPER_VEHICLE_HELPER_H_
#define SRC_APPLICATIONS_HELPER_VEHICLE_HELPER_H_
namespace ns3{
class VehicleHelper{
public:
	VehicleHelper(Address ip,uint16_t port);
	VehicleHelper(Ipv4Address ip,uint16_t port);
	VehicleHelper(Ipv6Address,uint16_t port);
	void SetAttribute(std::string name,const AttributeValue &value);
	void SetFill(Ptr<Application> app,std::string fill);
	void SetFill(Ptr<Application> app,uint8_t fill,uint32_t dataLength);
	void SetFill(Ptr<Application> app,uint8_t *fill,uint32_t fillLength,uint32_t dataLength);
	ApplicationContainer Install(Ptr<Node> node) const;
	ApplicationContainer Install(std::string nodeName) const;
	ApplicationContainer Install(NodeContainer c) const;
private:
	Ptr<Application> InstallPriv(Ptr<Node> node) const;
	ObjectFactory m_factory;
};
}
#endif /* SRC_APPLICATIONS_HELPER_VEHICLE_HELPER_H_ */

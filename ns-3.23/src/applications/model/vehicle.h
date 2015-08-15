/*
 * vehicle.h
 *
 *  Created on: 15-Aug-2015
 *      Author: tarun
 */
#include "ns3/network-module.h"
#include "ns3/core-module.h"
#ifndef SCRATCH_VEHICLE_H_
#define SCRATCH_VEHICLE_H_
namespace ns3{

class Vehicle:public Application
{
public:
	static TypeId GetTypeId(void);
	Vehicle();
	virtual ~Vehicle();
	void setRemote(Ipv4Address ip,uint16_t port);
	void setRemote(Ipv6Address ip,uint16_t port);
	void setRemote(Address ip,uint16_t port);
	void setDataSize(uint32_t datasize);
	uint32_t getDataSize() const;
	void setFill(std::string fill);
	void setFill(uint8_t fill, uint32_t datasize);
	void setFill(uint8_t *fill,uint32_t fillsize,uint32_t datasize);
protected:
	virtual void DoDispose(void);
private:
	virtual void StartApplication(void);
	virtual void StopApplication(void);
	void ScheduleTransmit(Time dt);
	void Send(void);
	void HandleRead(Ptr<Socket> socket);
	uint32_t m_count; //max number of packets the vehicle can sent
	Time m_interval; //Packet intersend time
	uint32_t m_size; //size of sent packet

	uint32_t m_datasize; //packet payload size(must be equal to m_size)
	uint8_t *m_data;

	uint32_t m_sent; //counter for sent packets
	Ptr<Socket> m_socket;
	Address m_peerAddress;
	uint16_t m_peerPort;
	EventId m_sendEvent;

	TracedCallback<Ptr<const Packet> > m_txTrace;

};
}


#endif /* SCRATCH_VEHICLE_H_ */

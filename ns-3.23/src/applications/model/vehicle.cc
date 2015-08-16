#include "vehicle.h"
namespace ns3{
NS_LOG_COMPONENT_DEFINE ("VehicleApplication");
NS_OBJECT_ENSURE_REGISTERED (Vehicle);
TypeId Vehicle::GetTypeId(void){
	static TypeId tid = TypeId("ns3::Vehicle")
			.SetParent<Application>()
			.SetGroupName("Applications")
			.AddConstructor<Vehicle>()
			.AddAttribute("MaxPackets",
					"The maximum number of packets the application will send",
					UintegerValue(100),
					MakeUintegerAccessor(&Vehicle::m_count),
					MakeUintegerChecker<uint32_t>())
			.AddAttribute("Interval",
					"The time to wait between packets",
					TimeValue(Seconds(1.0)),
					MakeTimeAccessor(&Vehicle::m_interval),
					MakeTimeChecker())
			.AddAttribute("RemoteAddress",
					"The Destination Address of outbound packets",
					AddressValue(),
					MakeAddressAccessor(&Vehicle::m_peerAddress),
					MakeAddressChecker())
			.AddAttribute("RemotePort",
					"The destination port of outbound packets",
					UintegerValue(0),
					MakeUintegerAccessor(&Vehicle::m_peerPort),
					MakeUintegerChecker<uint16_t>())
			.AddAttribute("PacketSize",
					"Size of echo data in outbount packets",
					UintegerValue(100),
					MakeUintegerAccessor(&Vehicle::setDataSize,
							&Vehicle::getDataSize),
					MakeUintegerChecker<uint32_t>())
			.AddTraceSource("Tx","A new packet is created and sent",
					MakeTraceSourceAccessor(&Vehicle::m_txTrace),
					"ns3::Packet::TraceCallback");
	return tid;
}

Vehicle::Vehicle(){
	NS_LOG_FUNCTION(this);
	m_sent = 0;
	m_socket = 0;
	m_sendEvent = EventId();
	m_data = 0;
	m_datasize = 0;
}

Vehicle::~Vehicle(){
	NS_LOG_FUNCTION(this);
	m_socket = 0;
	delete[] m_data;
	m_datasize = 0;
}
void
Vehicle::setRemote(Address ip,uint16_t port){
	NS_LOG_FUNCTION(this<<ip<<port);
	m_peerAddress = ip;
	m_peerPort = port;

}
void Vehicle::setRemote(Ipv4Address ip, uint16_t port){
	NS_LOG_FUNCTION(this<<ip<<port);
	m_peerAddress = Address(ip);
	m_peerPort = port;
}
void Vehicle::setRemote(Ipv6Address ip, uint16_t port){
	NS_LOG_FUNCTION(this<<ip<<port);
	m_peerAddress = Address(ip);
	m_peerPort = port;
}
void Vehicle::DoDispose(){
	NS_LOG_FUNCTION(this);
	Application::DoDispose();
}
void Vehicle::StartApplication(){
	NS_LOG_FUNCTION(this);
	//we can get only one object of ns3::udpsocketfactory with an instance of this class
	TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
	if(m_socket == 0){

		m_socket = Socket::CreateSocket(GetNode(), tid);
		m_socket->SetAllowBroadcast (true);
		if(Ipv4Address::IsMatchingType(m_peerAddress) == true){
			m_socket->Bind();
			m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress),m_peerPort));
		}
		else if(Ipv6Address::IsMatchingType(m_peerAddress) == true){
			m_socket->Bind6();
			m_socket->Connect(Inet6SocketAddress(Ipv6Address::ConvertFrom(m_peerAddress),m_peerPort));
		}
	}
	//creating receiver at each vehicle
	Ptr<Socket> recvSink = Socket::CreateSocket(GetNode(), tid);
	InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_peerPort);
	recvSink->Bind (local);
	recvSink->SetRecvCallback (MakeCallback (&Vehicle::HandleRead,this));
	//m_socket->SetRecvCallback(MakeCallback(&Vehicle::HandleRead,this));
	ScheduleTransmit(Seconds(1.0));
}
void Vehicle::StopApplication(){
	NS_LOG_FUNCTION(this);

	if(m_socket != 0){
		m_socket->Close();
		m_socket->SetRecvCallback(MakeNullCallback<void,Ptr<Socket> >());
		m_socket =0;
	}
	Simulator::Cancel(m_sendEvent);
}
void Vehicle::setDataSize(uint32_t dataSize){
	NS_LOG_FUNCTION(this<<dataSize);
	delete [] m_data;
	m_data = 0;
	m_datasize = 0;
	m_size = dataSize;
}
uint32_t Vehicle::getDataSize() const{
	NS_LOG_FUNCTION(this);
	return m_size;
}
void Vehicle::setFill(std::string fill){
	NS_LOG_FUNCTION(this<<fill);
	uint32_t dataSize = fill.size()+1;

	if(dataSize != m_datasize){
		delete [] m_data;
		m_data = new uint8_t[dataSize];
		m_datasize = dataSize;
	}
	memcpy(m_data,fill.c_str(),dataSize);
	m_size = dataSize;
}
void Vehicle::setFill(uint8_t fill,uint32_t dataSize){
	NS_LOG_FUNCTION(this<<fill<<dataSize);
	if(dataSize != m_datasize){
		delete [] m_data;
		m_data = new uint8_t[dataSize];
		m_datasize = dataSize;
	}
	memset(m_data,fill,dataSize);
	m_size = dataSize;
}
void Vehicle::setFill(uint8_t *fill,uint32_t fillSize , uint32_t dataSize){
	NS_LOG_FUNCTION(this<<fill<<fillSize<<dataSize);
	if(dataSize != m_datasize){
		m_data = new uint8_t[dataSize];
		m_datasize = dataSize;
	}
	if(fillSize >= dataSize){
		memcpy(m_data,fill,dataSize);
		m_size = dataSize;
		return;
	}
	uint32_t filled = 0;
	while(filled + fillSize < dataSize){
		memcpy(&m_data[filled],fill,fillSize);
		filled += fillSize;
	}
	memcpy(&m_data[filled],fill,dataSize - filled);
	m_size = dataSize;
}
void Vehicle::ScheduleTransmit(Time dt){
	NS_LOG_FUNCTION(this<<dt);
	m_sendEvent = Simulator::Schedule(dt, &Vehicle::Send, this);
}
void Vehicle::Send(){
	NS_LOG_FUNCTION(this);
	NS_ASSERT(m_sendEvent.IsExpired());
	Ptr<Packet> p;
	if(m_datasize){
		NS_ASSERT_MSG(m_datasize == m_size,"Vehicle::Send(): m_size and m_datasize inconsistent");
		NS_ASSERT_MSG(m_data,"Vehicle::Send():m_dataSize but no m_data");
		p= Create<Packet>(m_data,m_datasize); //caller wants a packet with its own data
	}
	else{
		p= Create<Packet>(m_size); //caller does not care about data in packet
	}
	m_txTrace(p);
	m_socket->Send(p);
	++m_sent;

	if(Ipv4Address::IsMatchingType(m_peerAddress)){
		NS_LOG_INFO("At time "<<Simulator::Now().GetSeconds()<<"s client sent "<<m_size <<" bytes to "<<
				Ipv4Address::ConvertFrom(m_peerAddress)<<" port "<<m_peerPort);
	}
	else if(Ipv6Address::IsMatchingType(m_peerAddress)){
		NS_LOG_INFO("At time "<<Simulator::Now().GetSeconds()<<"s client sent "<<m_size <<" bytes to "<<
				Ipv6Address::ConvertFrom(m_peerAddress)<<" port "<<m_peerPort);
	}
	if(m_sent < m_count){
		ScheduleTransmit(m_interval);
	}
}
void Vehicle::HandleRead(Ptr<Socket> socket){
	NS_LOG_FUNCTION(this<<socket);
	Ptr<Packet> packet;
	Address from;
	while((packet = socket->RecvFrom(from))){
		uint8_t * data;
		uint32_t size_packet = packet->GetSize();
		data = new uint8_t[size_packet];
		packet->CopyData(data,size_packet);
		if(InetSocketAddress::IsMatchingType(from)){
			NS_LOG_INFO("At time "<<Simulator::Now().GetSeconds()<<"s client received "<<packet->GetSize() <<" bytes from "<<
					InetSocketAddress::ConvertFrom(from).GetIpv4() <<" port "<<
					InetSocketAddress::ConvertFrom(from).GetPort()<<"Content is "<<data);
		}
		else if(Inet6SocketAddress::IsMatchingType(from)){
			NS_LOG_INFO("At time "<<Simulator::Now().GetSeconds()<< " client received "<<packet->GetSize()<< " bytes from "<<
					Inet6SocketAddress::ConvertFrom(from).GetIpv6()<< " port "<<
					Inet6SocketAddress::ConvertFrom(from).GetPort());
		}
	}
}
}

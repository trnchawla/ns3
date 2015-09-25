/*
 * controller.cc
 *
 *  Created on: 22-Aug-2015
 *      Author: tarun
 */

#include "controller.h"
namespace ns3{
NS_LOG_COMPONENT_DEFINE("Controller");
NS_OBJECT_ENSURE_REGISTERED (Controller);
SystemMutex Controller::mutexLanes;
int16_t Controller::lanes[] = {-1,-1,-1,-1,-1,-1,-1,-1};
int16_t Controller::lsl[] = {-1,-1,-1,-1,-1,-1,-1,-1};
uint16_t Controller::lockingStructure[8][3] ={{0,6,7},{1,2,3},{0,1,2},{3,4,5},{2,3,4},{5,6,7},{4,5,6},{0,1,7}};
int64_t Controller::turnTime[8][2] = {{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1}};
uint64_t Controller::queueLength[8] ={0,0,0,0,0,0,0,0};
volatile uint16_t Controller::m_np=3;
TypeId Controller::GetTypeId(void){
	static TypeId tid = TypeId("ns3::Controller")
							.SetParent<Application>()
							.SetGroupName("Applications")
							.AddConstructor<Controller>()
							.AddAttribute("RemotePort",
									"The port in which on vehicles will be listening",
									UintegerValue(0),
									MakeUintegerAccessor(&Controller::m_peerport),
									MakeUintegerChecker<uint16_t>());
	return tid;
}
Controller::Controller(){
m_totalLength = 20;
m_plt = new uint16_t[m_totalLength];
m_pltLength = 0;
m_rpTotalLength = 20;
m_rp = new uint16_t[m_rpTotalLength*3];
m_rpLength = 0;
m_sendSocket = 0;
m_recvSocket = 0;
m_np = 3;
}
Controller::~Controller(){
	m_totalLength = 0;
	delete[] m_plt;
	m_pltLength = 0;
	m_rpTotalLength = 0;
	delete[] m_rp;
	m_rpLength = 0;
	m_sendSocket = 0;
	m_recvSocket = 0;
	delete[] m_data;
}
void Controller::setFill(uint8_t *fill, uint16_t size){
	//NS_LOG_FUNCTION(this<<size);
	m_data = new uint8_t[size];
	m_size = size;
	memcpy(m_data,fill,size);
}
void Controller::setNp(uint16_t np){
	m_np = np;
}
void Controller::DoDispose(){
	NS_LOG_FUNCTION(this);
	Application::DoDispose();
}
void Controller::StartApplication(){
	NS_LOG_FUNCTION(this);
	TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
	if(m_sendSocket == 0){
		m_sendSocket = Socket::CreateSocket(GetNode(), tid);
		m_sendSocket->SetAllowBroadcast(true);
		m_sendSocket->Bind();
		m_sendSocket->Connect(InetSocketAddress(Ipv4Address("255.255.255.255"),m_peerport));//this socket will be used to broadcast
	}
	if(m_recvSocket == 0){
		m_recvSocket = Socket::CreateSocket(GetNode(), tid);
		InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(),m_peerport);
		m_recvSocket->Bind(local);
		m_recvSocket->SetRecvCallback(MakeCallback(&Controller::HandleRead, this));// this socket will be used for all incoming broadcast message
	}
}
void Controller::StopApplication(){
	NS_LOG_FUNCTION(this);
	if(m_sendSocket != 0){
		m_sendSocket->Close();
		m_sendSocket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
		m_sendSocket = 0;
	}
	if(m_recvSocket != 0){
		m_recvSocket->Close();
		m_recvSocket->SetRecvCallback(MakeNullCallback<void,Ptr<Socket> >());
		m_recvSocket = 0;
	}
}
void Controller::HandleRead(Ptr<Socket> socket){
	//NS_LOG_FUNCTION(this);
	Ptr<Packet> packet;
	Address from;
	while((packet = socket->RecvFrom(from))){
		uint8_t * data;
		uint32_t size_packet = packet->GetSize();
		data = new uint8_t[size_packet];
		packet->CopyData(data,size_packet);
		uint16_t * msgType = (uint16_t*)data;
		if(*msgType == REQUEST){
			request_msg * req = (request_msg*)data;
			NS_LOG_INFO("Controller: At time "<<Simulator::Now().GetSeconds()<<"s controller received REQUEST message of size "<<
					packet->GetSize()<<" bytes from "
					<<InetSocketAddress::ConvertFrom(from).GetIpv4()<< "port"
					<<InetSocketAddress::ConvertFrom(from).GetPort()
					<<" Vehicle sending request is "<<req->vid<<" and its lane number is "<<req->lid);

			NS_LOG_INFO("before lock");
			mutexLanes.Lock();
			//NS_LOG_INFO("Vid = "<<req->vid<<" Old1 queue++ length for lid "<<req->lid<<" is "<<queueLength[req->lid]);
			queueLength[req->lid]++;
			//NS_LOG_INFO("Vid = "<<req->vid<<" New1 queue++ length for lid "<<req->lid<<" is "<<queueLength[req->lid]);
			if(turnTime[req->lid][0] == -1){
				turnTime[req->lid][0] = Simulator::Now().GetMilliSeconds();
				NS_LOG_INFO("Request received for lane "<<req->lid<<" at time "<<turnTime[req->lid][0]<<" milliseconds");
			}
			//lanes[lid] == 0, lane is locked, lanes[lid]==-1 not locked
			//lockingStructure[lid][0,1,2] gives lanes required to be locked
			if((m_pltLength <= m_np) && (lanes[lockingStructure[req->lid][0]]==0) &&
					(lanes[lockingStructure[req->lid][1]]==0) &&
					(lanes[lockingStructure[req->lid][2]]==0)){   //all lanes are already locked
				addToPlt(req->vid);
				lsl[req->lid] = req->vid;
				permit_msg plt;
				plt.len = m_pltLength;
				plt.plt = m_plt;
				plt.type = PERMIT;
				if(turnTime[req->lid][1] == -1 && turnTime[req->lid][0] != -1){
					turnTime[req->lid][1] = Simulator::Now().GetMilliSeconds();
					NS_LOG_INFO("TURNTIME:"<<req->lid<<" "<<turnTime[req->lid][1]-turnTime[req->lid][0]);
				}
				//NS_LOG_INFO("Vid = "<<req->vid<<" Old1 queue-- length for lid  "<<req->lid<<" is "<<queueLength[req->lid]);
				queueLength[req->lid]--;
				//NS_LOG_INFO("Vid = "<<req->vid<<" New1 queue-- length for lid  "<<req->lid<<" is "<<queueLength[req->lid]);
				//NS_LOG_INFO("sending permit as all lanes are already locked lsl is "<<lsl[req->lid]<<"plt array is:");
				for(int i=0;i<m_pltLength;i++){
					NS_LOG_INFO(m_plt[i]);
				}
				setFill((uint8_t*)(&plt),sizeof(permit_msg));
				Ptr<Packet> p = Create<Packet>(m_data,m_size);
				m_sendSocket->Send(p);
			}
			else if((m_pltLength <= m_np) && (lanes[lockingStructure[req->lid][0]]==-1) &&
					(lanes[lockingStructure[req->lid][1]]==-1) &&
					(lanes[lockingStructure[req->lid][2]]==-1)){    //all lanes are unlocked
				lanes[lockingStructure[req->lid][0]]=0;
				lanes[lockingStructure[req->lid][1]]=0;
				lanes[lockingStructure[req->lid][2]]=0;
				addToPlt(req->vid);
				lsl[req->lid] = req->vid;
				permit_msg plt;
				plt.len = m_pltLength;
				plt.plt = m_plt;
				plt.type = PERMIT;
				//NS_LOG_INFO("Vid = "<<req->vid<<" Old2 queue-- length for lid "<<req->lid<<" is "<<queueLength[req->lid]);
				queueLength[req->lid]--;
				//NS_LOG_INFO("Vid = "<<req->vid<<" New2 queue-- length for lid "<<req->lid<<" is "<<queueLength[req->lid]);
				if(turnTime[req->lid][1] == -1){
					turnTime[req->lid][1] = Simulator::Now().GetMilliSeconds();
					NS_LOG_INFO("TURNTIME:"<<req->lid<<" "<<turnTime[req->lid][1]-turnTime[req->lid][0]);
				}
				setFill((uint8_t*)(&plt),sizeof(permit_msg));
				Ptr<Packet> p = Create<Packet>(m_data,m_size);
				//NS_LOG_INFO("sending permit as all lanes are  unlocked lsl is " <<lsl[req->lid] <<"plt array is:");
				for(int i=0;i<m_pltLength;i++){
					NS_LOG_INFO(m_plt[i]);
				}
				m_sendSocket->Send(p);
			}
			else{
				addToRp(req->vid,req->lid);
			}
			mutexLanes.Unlock();
			NS_LOG_INFO("After lock");
		}
		else if(*msgType == RELEASE){
			release_msg * release = (release_msg *)data;
			/*NS_LOG_INFO("Controller: At time "<<Simulator::Now().GetSeconds()<<"s controller received RELEASE message of size "<<
					packet->GetSize()<<" bytes from "
					<<InetSocketAddress::ConvertFrom(from).GetIpv4()<< "port"
					<<InetSocketAddress::ConvertFrom(from).GetPort()
					<<" Vehicle sending request is "<<release->vid<<" and its lane number is "<<release->lid<<" lsl is"<<lsl[release->lid]);*/
			mutexLanes.Lock();
			if(lsl[release->lid] >= 0 && lsl[release->lid] == release->vid){ //implement a method to store the vid which will be used to unlock the lsli
				lanes[lockingStructure[release->lid][0]]=-1;
				lanes[lockingStructure[release->lid][1]]=-1;
				lanes[lockingStructure[release->lid][2]]=-1;
				m_pltLength = 0;
				lsl[release->lid] = -1;
				float avLength=0;
				for(int i=0;i<8;i++){
					NS_LOG_INFO("QueueLength of lid "<<i<<" is"<<queueLength[i]);
					avLength+=queueLength[i];
				}
				avLength/=8;
				m_np =(uint16_t) avLength+1;
				NS_LOG_INFO("Average QUEUELENGTH before switching is "<<avLength);
			}
			else{
				mutexLanes.Unlock();
				return;
			}
			for(int i=0;i<m_rpLength;i++){  //implement a structure to remove the element from rp which can be in between too in the list
				//NS_LOG_INFO("m_rp["<<i<<"] is"<<m_rp[3*i]<<" m_rp["<<i<<"][2]"<<m_rp[3*i+2]);
				if((m_pltLength <= m_np) && (m_rp[3*i+2] == 0) && (lanes[lockingStructure[m_rp[3*i+1]][0]]==0) &&  //lockingStructure[lane][0,1,2]
						(lanes[lockingStructure[m_rp[3*i+1]][1]]==0) &&
						(lanes[lockingStructure[m_rp[3*i+1]][2]]==0)){
					addToPlt(m_rp[3*i]);
					permit_msg plt;
					plt.len = m_pltLength;
					plt.plt = m_plt;
					plt.type = PERMIT;
					if(turnTime[m_rp[3*i+1]][1] == -1){
						turnTime[m_rp[3*i+1]][1] = Simulator::Now().GetMilliSeconds();
						NS_LOG_INFO("TURNTIME:"<<m_rp[3*i+1]<<" "<<turnTime[m_rp[3*i+1]][1]-turnTime[m_rp[3*i+1]][0]);
					}
					//NS_LOG_INFO("Vid = "<<m_rp[3*i]<<" Old3 queue-- length of lid "<<m_rp[3*i+1]<<" is "<<queueLength[m_rp[3*i+1]]);
					queueLength[m_rp[3*i+1]]--;
					//NS_LOG_INFO("Vid = "<<m_rp[3*i]<<" New3 queue-- length of lid "<<m_rp[3*i+1]<<" is "<<queueLength[m_rp[3*i+1]]);
					lsl[m_rp[3*i+1]] = m_rp[3*i];
					m_rp[3*i+2]=1;//making the entry in m_rp invalid 1 means invalid, 0 means valid to be considered to check for locks
					setFill((uint8_t*)(&plt),sizeof(permit_msg));
					Ptr<Packet> p = Create<Packet>(m_data,m_size);
					m_sendSocket->Send(p);
				}
				else if((m_pltLength <= m_np) && (m_rp[3*i+2] == 0) && (lanes[lockingStructure[m_rp[3*i+1]][0]]==-1) &&  //lockingStructure[lane][0,1,2]
						(lanes[lockingStructure[m_rp[3*i+1]][1]]==-1) &&
						(lanes[lockingStructure[m_rp[3*i+1]][2]]==-1)){
					lanes[lockingStructure[m_rp[3*i+1]][0]]=0;
					lanes[lockingStructure[m_rp[3*i+1]][1]]=0;
					lanes[lockingStructure[m_rp[3*i+1]][2]]=0;
					addToPlt(m_rp[3*i]);
					permit_msg plt;
					plt.len = m_pltLength;
					plt.plt = m_plt;
					plt.type = PERMIT;
					//NS_LOG_INFO("Vid = "<<m_rp[3*i]<<" Old4 queue-- length of lid "<<m_rp[3*i+1]<<" is "<<queueLength[m_rp[3*i+1]]);
					queueLength[m_rp[3*i+1]]--;
					//NS_LOG_INFO("Vid = "<<m_rp[3*i]<<" New4 queue-- length of lid "<<m_rp[3*i+1]<<" is "<<queueLength[m_rp[3*i+1]]);
					if(turnTime[m_rp[3*i+1]][1] == -1){
						turnTime[m_rp[3*i+1]][1] = Simulator::Now().GetMilliSeconds();
						NS_LOG_INFO("TURNTIME:"<<m_rp[3*i+1]<<" "<<turnTime[m_rp[3*i+1]][1]-turnTime[m_rp[3*i+1]][0]);
					}
					lsl[m_rp[3*i+1]] = m_rp[3*i];
					m_rp[3*i+2]=1;
					setFill((uint8_t*)(&plt),sizeof(permit_msg));
					Ptr<Packet> p = Create<Packet>(m_data,m_size);
					m_sendSocket->Send(p);
				}

			}
			mutexLanes.Unlock();
		}
	}
}
void Controller::addToPlt(uint16_t vid){
	if(m_pltLength < m_totalLength){
		m_pltLength++;
		m_plt[m_pltLength-1]=vid;
		return;
	}
	m_totalLength+=20;
	uint16_t *plt = new uint16_t[m_totalLength];
	for(int i=0;i<m_pltLength;i++){
		plt[i]=m_plt[i];
	}
	delete [] m_plt;
	m_plt = plt;
	m_pltLength++;
	m_plt[m_pltLength-1]=vid;
}
void Controller::addToRp(uint16_t vid,uint16_t lid){
	//NS_LOG_INFO("addToRp vid = "<<vid<<" lid = "<<lid);
	if(m_rpLength < m_rpTotalLength){
		m_rpLength++;
		m_rp[3*(m_rpLength-1)]=vid;
		m_rp[3*(m_rpLength-1)+1]=lid;
		m_rp[3*(m_rpLength-1)+2]=0;
		NS_LOG_INFO("addToRp before returning vid = "<<m_rp[3*(m_rpLength-1)]<<" lid = "<<m_rp[3*(m_rpLength-1)+1]);
		return;
	}
	m_rpTotalLength+=20;
	uint16_t *rp = new uint16_t[3*m_rpTotalLength];
	for(int i=0;i<m_rpLength;i++){
		rp[3*i]=m_rp[3*i];
		rp[3*i+1]=m_rp[3*i+1];
		rp[3*i+2]=m_rp[3*i+2];
	}
	delete[] m_rp;
	m_rp = rp;
	m_rpLength++;
	m_rp[3*(m_rpLength-1)]=vid;
	m_rp[3*(m_rpLength-1)+1]=lid;
	m_rp[3*(m_rpLength-1)+2]=0;
	//NS_LOG_INFO("addToRp before returning vid = "<<m_rp[3*(m_rpLength-1)]<<" lid = "<<m_rp[3*(m_rpLength-1)+1]);
}
}



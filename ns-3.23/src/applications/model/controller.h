/*
 * controller.h
 *
 *  Created on: 22-Aug-2015
 *      Author: tarun
 */
#include "vehicle.h"
/*     #  |  ||  |  #
	   #  |  ||  |  #
	   #  |  ||  |  #
	   #  |  ||  |  #
######## 0|1 ||  |  #######
________            2_______
                    3
========            =======
_______7            _______
       6
########  |  || 5|4 #######
	   #  |  ||  |  #
       #  |  ||  |  #
       #  |  ||  |  #
	   #  |  ||  |  #

Lane of Vehicle   Lanes to be locked
0                 0,6,7
1                 1,2,3
2                 0,1,2
3                 3,4,5
4                 2,3,4
5                 5,6,7
6                 4,5,6
7                 0,1,7
	   */
#ifndef SRC_APPLICATIONS_MODEL_CONTROLLER_H_
#define SRC_APPLICATIONS_MODEL_CONTROLLER_H_
namespace ns3{
class Controller: public Application{
public:
	static SystemMutex mutexLanes;
	static TypeId GetTypeId(void);
	static int16_t lanes[8];
	static uint16_t lockingStructure[8][3];
	static int16_t lsl[8];
	Controller();
	~Controller();
	void setFill(uint8_t *fill,uint16_t size);
	void HandleRead(Ptr<Socket> socket);
	void addToPlt(uint16_t vid);
	void addToRp(uint16_t vid,uint16_t lid);
protected:
	virtual void DoDispose(void);
private:
	virtual void StartApplication(void);
	virtual void StopApplication(void);
	volatile uint16_t m_np;
	//m_rp vehicles in queue, m_rpTotalLength length of memory allocated to rp queue,m_rpLength length used for rp
	uint16_t *m_rp;
	uint16_t m_rpTotalLength;
	uint16_t m_rpLength;
	//m_plt vehicles having passing permit, m_totalLenght length of array allocated to plt array,m_pltLength length used in array
	uint16_t *m_plt;
	uint16_t m_totalLength;
	uint16_t m_pltLength;
	uint16_t m_size;
	uint8_t *m_data;
	Ptr<Socket> m_sendSocket;
	uint16_t m_peerport;
	Ptr<Socket> m_recvSocket;
	enum {REQUEST,PERMIT,RELEASE};


};
}

#endif /* SRC_APPLICATIONS_MODEL_CONTROLLER_H_ */

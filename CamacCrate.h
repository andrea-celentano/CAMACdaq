/*
 * CamacCrate.h
 *
 *  Created on: Oct 7, 2018
 *      Author: celentan
 */

#ifndef CAMACCRATE_H_
#define CAMACCRATE_H_

#include <string>
#include <iostream>
#include <pthread.h>
#include <vector>
#include <list>
using namespace std;

class CamacCrate {
public:
	CamacCrate();
	virtual ~CamacCrate();

	const string& getIP() const {
		return IP;
	}

	void setIP(const string& ip) {
		IP = ip;
	}

	int Connect();
	void Scan();

	int InitDiscriminator();
	void SetDiscriminatorSlot(int slot);
	void SetDiscriminatorThreshold(int ch,int thr);

	int InitTDC();
	void SetTDCSlot(int slot);
	void SetTDCMode(int mode);
	void SetTDCTimeout(int timeout);

	int getTDCSlot(){return tdc_slot;};

	int start_stop;
	int is_configured;

	short getCrateID(){return crate_id;};

	void SetBuffer(list<uint32_t> *m_buffer);
	void SetReadMutex(pthread_mutex_t *mutex);

	void ReadData();

	int getTimeLength(){return timeLength;};
	void setTimeLength(int length){timeLength=length;};

	int getEventLength(){return eventLength;};
	void setEventLength(int length){eventLength=length;};



private:
	string IP;
	short crate_id;
	short is_connected;
	unsigned int scan_result;

	/*Discriminator*/
	int disc_slot;
	int disc_thr[16];

	/*TDC*/
	int tdc_slot;
	int tdc_timeout;
	int tdc_mode;

	/*Data buffer*/
	pthread_mutex_t *read_mutex;
	list<uint32_t> *buffer; 	//pointer to the buffer memory

	/*limits*/
	int timeLength,eventLength;
};

#endif /* CAMACCRATE_H_ */

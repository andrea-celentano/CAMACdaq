/*
 * CamacCrate.cc
 *
 *  Created on: Oct 7, 2018
 *      Author: celentan
 */

#include "CamacCrate.h"
#include "crate_lib.h"

CamacCrate::CamacCrate() :
		crate_id(0), is_connected(0), scan_result(0), disc_slot(0), tdc_slot(0), is_configured(0),timeLength(-1),eventLength(-1) {
	// TODO Auto-generated constructor stub

	/*Discriminator init thr*/
	for (int ii = 0; ii < 16; ii++) {
		disc_thr[ii] = 255; //8 bits
	}
}

CamacCrate::~CamacCrate() {
	// TODO Auto-generated destructor stub
}

int CamacCrate::Connect() {
	short res;
	if (IP.length() == 0) {
		cout << "CamacCrate::Connect error, IP not set" << endl;
		return -1;
	} else {
		char ipstr[20];
		sprintf(ipstr, "%s", IP.c_str());
		printf("CamacCrate::Connect() to --> %s <--\n", ipstr);
		crate_id = CROPEN(ipstr);
		if (crate_id < 0) {
			printf("Error %d opening connection with CAMAC Controller \n", crate_id);
			return -1;
		}
		is_connected = 1;

		res = CCCZ(crate_id);
		if (res < 0) {
			printf("Error executing CCCZ operation: %d\n", res);
			return -1;
		}
		return 0;
	}
}

void CamacCrate::Scan() {

	short res;
	res = CSCAN(crate_id, &scan_result);

	if (res != CRATE_OK) {
		printf("Error occurs scanning the CRATE: %d\n", res);
	}
	for (int ii = 0; ii < 24; ii++) {
		if (scan_result & (1 << ii)) {
			printf("The slot %d is filled with a card.\n", ii + 1);
		}
	}

}

int CamacCrate::InitDiscriminator() {
	short res;
	CRATE_OP cr_op;

	/*Clear the discriminator with F(9) N*/
	cr_op.F = 9;
	cr_op.N = disc_slot;
	cr_op.A = 0;
	cr_op.DATA = 0;
	res = CFSA(crate_id, &cr_op);
	if (res < 0) {
		printf("InitDiscriminator(): error executing CSSA operation: %d\n", res);
		return -1;
	}

	/*Write discr. thresholds*/
	for (int ii = 0; ii < 15; ii++) {
		cr_op.F = 16;
		cr_op.N = disc_slot;
		cr_op.A = ii;
		cr_op.DATA = disc_thr[ii] & 0xFF; //8 bits!;
		res = CFSA(crate_id, &cr_op);
		if (res < 0) {
			printf("InitDiscriminator(): error executing CSSA operation-thr[%d] set: %d\n", ii, res);
			return -1;
		}
	}

	/*Read them back*/
	for (int ii = 0; ii < 15; ii++) {
		cr_op.F = 0;
		cr_op.N = disc_slot;
		cr_op.A = ii;
		res = CFSA(crate_id, &cr_op);
		cr_op.DATA &= 0xFF; //8 bits!
		if (res < 0) {
			printf("InitDiscriminator(): error executing CSSA operation-thr[%d] read: %d\n", ii, res);
		}
		printf("Ch[%d] set[%d] read-back[%d]\n", ii, disc_thr[ii], cr_op.DATA);
		if (disc_thr[ii] != cr_op.DATA) {
			printf("ERROR! \n");
			return -1;
		}
	}
	return 0;
}

void CamacCrate::SetDiscriminatorSlot(int slot) {
	if ((slot < 1) || (slot > 25)) {
		printf("CamacCrate::SetDiscriminatorSlot error, slot should be >=1 and <=25. You set %d \n", slot);
		return;
	}
	disc_slot = slot;
}
void CamacCrate::SetDiscriminatorThreshold(int ch, int thr) {
	if ((ch < 0) || (ch > 15)) {
		printf("CamacCrate::SetDiscriminatorThreshold error, ch should be >=0 and <=15. You set %d \n", ch);
		return;
	}
	if (thr < 3 || thr > 255) {
		printf("CamacCrate::SetDiscriminatorThreshold error, ch should be >=3 and <=255. You set %d \n", thr);
		return;
	}
	disc_thr[ch] = thr;
}

void CamacCrate::SetTDCSlot(int slot) {
	if ((slot < 1) || (slot > 25)) {
		printf("CamacCrate::SetTDCSlot error, slot should be >=1 and <=25. You set %d \n", slot);
		return;
	}
	tdc_slot = slot;
}

void CamacCrate::SetTDCMode(int mode) {
	if ((mode < 0) || (mode > 1)) {
		printf("CamacCrate::SetTDCMode error, slot should 0 or 1. You set %d \n", mode);
		return;
	}
	tdc_mode = mode;
}

void CamacCrate::SetTDCTimeout(int timeout) {
	if ((timeout < 0) || (timeout > 7)) {
		printf("CamacCrate::SetTDCTimeout error, slot should be >=1 and <=25. You set %d \n", timeout);
		return;
	}
	tdc_timeout = timeout;
}

int CamacCrate::InitTDC() {
	short res;
	int mode, timeout;
	CRATE_OP cr_op;

	/*Clear the tdc with F(9) N*/
	cr_op.F = 9;
	cr_op.N = tdc_slot;
	cr_op.A = 0;
	cr_op.DATA = 0;
	res = CFSA(crate_id, &cr_op);
	if (res < 0) {
		printf("InitTDC(): error executing CFSA operation: %d\n", res);
		return -1;
	}

	/*Write Status register for relevant parts*/
	cr_op.F = 17;
	cr_op.N = tdc_slot;
	cr_op.A = 0;

	cr_op.DATA = 0;
	//mode: bit 15
	cr_op.DATA = cr_op.DATA | ((tdc_mode & 0x1) << 15);
	//timeout: bit 12,13,14
	cr_op.DATA = cr_op.DATA | ((tdc_timeout & 0x7) << 12);
	//hit leading edge, bit 11: set to 1
	cr_op.DATA = cr_op.DATA | (1 << 11);
	//hit trailing edge, bit 10: set to 0
	cr_op.DATA = cr_op.DATA | (0 << 10);

	res = CFSA(crate_id, &cr_op);
	if (res < 0) {
		printf("InitDiscriminator(): error executing CFSA operation-configure register set\n");
		return -1;
	}

	/*Read back*/
	cr_op.F = 1;
	cr_op.N = tdc_slot;
	cr_op.A = 0;
	res = CFSA(crate_id, &cr_op);
	if (res < 0) {
		printf("InitDiscriminator(): error executing CFSA operation-read back register\n");
		return -1;
	}

	mode = (cr_op.DATA >> 15) & 0x1;
	if (mode != tdc_mode) {
		printf("InitDiscriminator(): error in tdc_mode readback. Set[%d] Read[%]d\n", tdc_mode, mode);
		return -1;
	}
	timeout = (cr_op.DATA >> 12) & 0x7;
	if (timeout != tdc_timeout) {
		printf("InitDiscriminator(): error in tdc_timeout readback. Set[%d] Read[%]d\n", tdc_timeout, timeout);
		return -1;
	}

	/*Enable LAM*/
	cr_op.F = 26;
	cr_op.N = tdc_slot;
	cr_op.A = 0;
	res = CFSA(crate_id, &cr_op);
	if (res < 0) {
		printf("InitDiscriminator(): error executing CFSA operation-enable LAM\n");
		return -1;
	}

	return 0;
}

void CamacCrate::SetBuffer(list<uint32_t> *m_buffer) {
	buffer = m_buffer;
}

void CamacCrate::SetReadMutex(pthread_mutex_t *mutex) {
	this->read_mutex = mutex;
}

/*This should be called resulting from a LAM*/
void CamacCrate::ReadData() {
	CRATE_OP cr_op, cr_op2;
	uint32_t data;

	/*Read the tdc with F(0) N*/
	cr_op.F = 0;
	cr_op.N = tdc_slot;
	cr_op.A = 0;

	/*Test the BIP with F27*/
	cr_op2.F = 27;
	cr_op2.N = tdc_slot;
	cr_op2.A = 0;

	while (1) {
		CFSA(crate_id, &cr_op); /*READ TDC*/

		if (cr_op.Q == 1) { /*Was the word a valid word? if Q==1 yes, continue and try next*/
			data = (uint32_t) (cr_op.DATA & 0x3FFFFF); //22-bit words

			pthread_mutex_lock(this->read_mutex);
			buffer->push_back(data);
			pthread_mutex_unlock(this->read_mutex);
			continue;
		}

		else { /*Q=0. Last word was not goot. Have we ended or still buffering?*/
			CFSA(crate_id, &cr_op2);
			if (cr_op2.Q == 0) { /*Yes, ended*/
				break;
			} else {
				continue;
			}
		}
	}
}

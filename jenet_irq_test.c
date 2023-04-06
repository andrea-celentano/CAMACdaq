/*
 =======================================================
 JENET IRQ Test
 =======================================================
 Author      : Mauro Vetuschi
 Company     : ZP Engineering srl
 Description : Example using IRQ Callback
 Date        : 23/03/2005
 Version     : 1.00
 Revision    :     
 =======================================================
*/

#ifdef WIN32

#include <stdio.h>
#include <time.h>

#else

#include <stdio.h>
#include <stdlib.h> 
#include <fcntl.h> 
#include <string.h> 
#include <errno.h> 
#include <dirent.h> 
#include <signal.h> 
#include <unistd.h> 
#include <ctype.h> 
#include <netdb.h>
#include <sys/wait.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/stat.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <sys/ioctl.h>

#endif

#include "crate_lib.h"

short irq_cnt, lam_cnt;
CRATE_OP op;

void IRQHandler(short crate_id, short irq_type, unsigned int irq_data)
{
	char nimo, value;

	if (irq_cnt >= 8)
		return;

	if (lam_cnt >= 1000) 
		return;

	switch (irq_type) {
		case LAM_INT:
			
			// Clear LAM on dataway display
			
			op.F = 10;
			op.N = 11;
			op.A = 0;
			op.DATA = 0;

			CSSA(crate_id, &op);

			// Clear any pending LAM interrupts
	
			LACK(crate_id);
			
			lam_cnt++;

			// Toggle nim set out in order to get next LAM IRQ request 
			NOSOS(crate_id, 1, 0);
			if (lam_cnt < 1000) {
				NOSOS(crate_id, 1, 1);
			}
			
			break;
		case COMBO_INT:
			printf("COMBO Reg: %x\n", irq_data);
			break;
		case DEFAULT_INT:
			nimo = (irq_cnt % 4) + 1;
			value = 1 - (irq_cnt / 4);
			printf("Default button pressed!\n");
			NOSOS(crate_id, nimo, value);
			irq_cnt++;
			break;
	}
	return;
}

int main(int argc, char *argv[])
{
	short crate_id, retcode;
	time_t start, end;
	double eta;

	
	// NB example assuming that IP address of JENET Board is 192.168.0.103 

	printf("JENET IRQ Test\n");
	irq_cnt = 0;
	lam_cnt = 0;
	
	if (argc == 2)
		crate_id = CROPEN(argv[1]);
	else
		crate_id = CROPEN("192.168.0.98");

	if (crate_id >= 0) {
		printf("Press Default button on JENET Board...\n");

		// Init crate
		retcode = CCCC(crate_id);
		
		retcode = CCCZ(crate_id);

		// Enable LAM on dataway display
		op.F = 26;
		op.N = 11;
		op.A = 0;
		op.DATA = 0;

		retcode = CSSA(crate_id, &op);
		
		// Clear LAM on dataway display
		op.F = 10;
		retcode = CSSA(crate_id, &op);

		// Clear any pending LAM interrupts
		retcode = LACK(crate_id);

		retcode = CRIRQ(crate_id, IRQHandler);
	
		if (retcode != CRATE_OK) {
			printf("ERROR: %d - Unable to register IRQ Callback!\n", retcode);
			CRCLOSE(crate_id);
			return 0;
		}

		// This test perform 1000 toggle nim set out in order to get 1000 LAM IRQ request 
		NOSOS(crate_id, 1, 0);
		NOSOS(crate_id, 1, 1);
		
		start = time(NULL);
	
		while ((irq_cnt < 8) && (lam_cnt < 1000)) 
#ifdef WIN32
			Sleep(1000);
#else
			sleep(1);
#endif
		end = time(NULL);
		eta = difftime(end, start);

		printf("Elapsed Time: %.2f - %.2f Hz\n", eta, 1000/eta);

		CRCLOSE(crate_id);
	}
	else {
		printf("ERROR: %d - Unable to connect with specified ip address!\n", crate_id);
	}

    printf("Bye bye!\n");
	return 0;
}

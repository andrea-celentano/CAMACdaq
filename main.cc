/*ROOT STUFF */
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TAxis.h"
#include "TApplication.h"
#include "TMultiGraph.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TH1D.h"

extern "C" {
#include "crate_lib.h"
}
#include <pthread.h>
#include <gtk/gtk.h>
#include "interface.h"
#include "callback.h"
#include "support.h"
#include "struct_event.h"
#include "CamacCrate.h"

#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <vector>
#include <list>

#define BUF_LENGTH 41000000
#define MAX_LENGTH 4100

using namespace std;

TApplication gui("gui", 0, NULL);

uint32_t nevents_ready = 0;
uint32_t nevents_read = 0;
//int pthread_create(pthread_t * pth, pthread_attr_t *att, void * (*function), void * arg);
//int pthread_cancel(pthread_t thread);
//pthread_join(pth, NULL /* void ** return value could go here */);
//pthread_t pth;	// this is our thread identifier

//pthread_mutex_init, which takes 2 parameters. The first is a pointer to a mutex_t that we're creating. The second parameter is usually NULL//pthread_mutex_lock(*mutex_t)
//pthread_mutex_unlock(*mutex_t)

CamacCrate cc; //global var to handle crate

list<uint32_t> buf;

/********** Global vars for the output and for simple analisis ***********/
//FILE *out_file; /*output file */
uint32_t time_difference;
TTree *out_tree;
TFile *out_file;
ofstream file_txt;

event this_event;
event past_event;

pthread_mutex_t read_mutex;
pthread_mutex_t tree_mutex;

int is_first = 1;
int prev_samples = 0;
int this_samples = 0;

/* The function to decode events and write them to disk/root file/whatever it is.. */
/* At the beginning of the function,*write_buf point to the first word to be written or analized */
void decode_event(int event_size) {

	/*Load analizer, get data*/
	out_tree->Fill();
	if (this_event.num % 1000 == 0) out_tree->AutoSave("SaveSelf");
}

/*Function for gui thread: this handles gtk! */
void * gui_fun(void *arg) {

	printf("Start gui thread\n");
	gtk_set_locale();
	gtk_init(0, 0);

	GtkWidget *window1;
	window1 = create_window1();

	g_signal_connect(window1, "delete_event", G_CALLBACK(delete_event), NULL);
	g_signal_connect_swapped(window1, "destroy", G_CALLBACK(gtk_widget_destroy), window1);
	g_signal_connect(window1, "destroy", gtk_main_quit, NULL);

	/*quit*/
	GtkWidget *button5;
	button5 = lookup_widget(window1, "button5");
	g_signal_connect(button5, "clicked", G_CALLBACK(on_button5_clicked), (gpointer)(&cc));

	/*configure*/
	GtkWidget *button2;
	button2 = lookup_widget(window1, "button2");
	g_signal_connect(button2, "clicked", G_CALLBACK(on_button2_clicked), (gpointer)(&cc));

	/*start*/
	GtkWidget *button3;
	button3 = lookup_widget(window1, "button3");
	g_signal_connect(button3, "clicked", G_CALLBACK(on_button3_clicked), (gpointer)(&cc));

	/*Stop*/
	GtkWidget *button4;
	button4 = lookup_widget(window1, "button4");
	g_signal_connect(button4, "clicked", G_CALLBACK(on_button4_clicked), (gpointer)(&cc));

	/*Soft trigger*/
	GtkWidget *button6;
	button6 = lookup_widget(window1, "button6");
	g_signal_connect(button6, "clicked", G_CALLBACK(on_button6_clicked), (gpointer)(&cc));

	/*Abilito solo configure all'avvio, ovvero button 2, e quit, ovvero button 5*/
	gtk_widget_set_sensitive(button3, 0);
	gtk_widget_set_sensitive(button4, 0);
	//gtk_widget_set_sensitive(button6,0);

	/*SHOW WINDOW 1*/
	gtk_widget_show(window1);
	gtk_main();

	printf("gui fun exit\n");
	return 0;
}

void IRQHandler(short crate_id, short irq_type, unsigned int irq_data) {
	switch (irq_type) {
	case LAM_INT:
#ifdef DEBUG
		printf("GOT LAM, irq_data[0x%x]\n",irq_data);
#endif
		/*Check the slot*/
		if ((irq_data & (1 < (cc.getTDCSlot() - 1))) == 0x0) {
			printf("IRQHandler error, IRQ data[0x%x] \n", irq_data);
			exit(1);
		}
		/*Read the data. since we use LAM, need to read using F0.A0, then check Q. If Q==1,read again. If Q==0, check BIP with F27.A0: if Q=1, buffering in progress, wait */
		cc.ReadData(); //Mutex inside
		nevents_read++;
		LACK(crate_id);
		break;
	case COMBO_INT:
// Do something when a COMBO event occurs
// Write your code here
// ...
		break;
	case DEFAULT_INT:
		printf("You pressed default button on module \n");
		break;
	}
	return;
}

void * read_fun(void *arg) {
	printf("Start read thr \n");
	short res;
	uint32_t word, data;
	int ch;
	bool flagExit=false;
	while (1) {
		/*IF the CC was configured, register IRQ (only once!)*/
		if (cc.is_configured == 1) {
			//Register IRQ function
			res = CRIRQ(cc.getCrateID(), IRQHandler); /*This opens a new thread!*/
			if (res < 0) {
				printf("Error occurs registering callback: %d\n", res);
				exit(1);
			}
			printf("read_fun: IRQ was set\n");
			break;
		}
	}

	while (1) {
		if (cc.start_stop == 1) {
			flagExit=true;
			pthread_mutex_lock(&read_mutex);
			nevents_ready = buf.size();
			if (nevents_ready > 0) {
				word = buf.front();
				buf.pop_front();

				/*Channel is 5 bits from 17*/
				ch = (word >> 17) & 0x1F;
				/*Data is 16 bits from 0*/
				data = (word >> 0) & 0xFFFF;
				if (ch == 0) {
					pthread_mutex_lock(&tree_mutex);
					this_event.data = data;
					this_event.num++;
					out_tree->Fill();
					pthread_mutex_unlock(&tree_mutex);
					file_txt<<data<<endl;
				}
			}
			pthread_mutex_unlock(&read_mutex);
		}
		else if ((cc.start_stop ==0)&&(flagExit)){
			printf("End read thr \n");
			break;
		}
	}
	printf("Exit read thr \n");
}

/*just a simple function to print out the rate */
/*And to do some monitoring!*/
void * rate_fun(void *arg) {
	printf("Start rate thr \n");
	uint64_t CurrentTime, PrevRateTime, ElapsedTime, TotalElapsedTime, OriginalTime;
	uint64_t nevents_diff = 0;
	uint64_t nevents_prev = 0;
	uint64_t nevents = 0;
	PrevRateTime = 0;

	float diff_rate, int_rate;

	/*
	 TCanvas *c_waveforms = new TCanvas("Waveforms", "Waveforms", 800, 800);
	 c_waveforms->Divide(3, 3);

	 TCanvas *c_energy = new
	 TCanvas("Energy", "Energy", 800, 800);
	 c_energy->Divide(3, 3);

	 TCanvas *c_pedestal = new
	 TCanvas("Pedestal", "Pedestal", 800, 800);
	 c_pedestal->Divide(3, 3);

	 TGraph **waveforms = new TGraph*[FADC_CHANNELS_PER_BOARD];
	 for (int ii = 0; ii < FADC_CHANNELS_PER_BOARD; ii++) {
	 waveforms[ii] = new
	 TGraph();
	 waveforms[ii]->SetTitle(Form("CH %i", ii));
	 waveforms[ii]->GetXaxis()->SetTitle("Time (ns)");
	 waveforms[ii]->GetYaxis()->SetTitle("Voltage (mV)");
	 waveforms[ii]->SetMarkerStyle(7);
	 }

	 TH1D **energy = new
	 TH1D*[FADC_CHANNELS_PER_BOARD];
	 for (int ii = 0; ii < FADC_CHANNELS_PER_BOARD; ii++) {
	 energy[ii] = new
	 TH1D(Form("hen%i", ii), Form("hen%i", ii), 200, -10, 500);
	 energy[ii]->SetTitle(Form("CH %i", ii));
	 energy[ii]->GetXaxis()->SetTitle("Energy (pC)");
	 }

	 TH1D **pedestal = new
	 TH1D*[FADC_CHANNELS_PER_BOARD];
	 for (int ii = 0; ii < FADC_CHANNELS_PER_BOARD; ii++) {
	 pedestal[ii] = new
	 TH1D(Form("hped%i", ii), Form("hped%i", ii), 210, -100, 10);
	 pedestal[ii]->SetTitle(Form("CH %i", ii));
	 pedestal[ii]->GetXaxis()->SetTitle("Pedestal (mV)");
	 }

	 */
	/*RATES*/
	TGraph *gdiff_rate = new TGraph();
	gdiff_rate->GetXaxis()->SetTitle("Time (5s)");
	gdiff_rate->GetYaxis()->SetTitle("Rate (Hz)");
	gdiff_rate->SetMarkerStyle(7);

	TGraph *gint_rate = new TGraph();
	gint_rate->GetXaxis()->SetTitle("Time (5s)");
	gint_rate->GetYaxis()->SetTitle("Rate (Hz)");
	gint_rate->SetMarkerStyle(7);
	gint_rate->SetMarkerColor(2);

	TMultiGraph *grate = NULL;

	int rate_counter = 0;
	const int n_rate_points = 500;

	bool flagExit=false;

	while (1) {
		if (cc.start_stop == 1) {
			if (rate_counter == 0) OriginalTime = get_time();
			CurrentTime = get_time();
			ElapsedTime = CurrentTime - PrevRateTime;
			TotalElapsedTime = CurrentTime - OriginalTime;


			if (((TotalElapsedTime/1000.)>cc.getTimeLength())&&(cc.getTimeLength()>-1)){
				printf("End program due to time limit: you set %i seconds \n",cc.getTimeLength());
				cc.start_stop=0;
			}


			if ((nevents_read>cc.getEventLength())&&(cc.getEventLength()>-1)){
				printf("End program due to event limit: you set %i events \n",cc.getEventLength());
				cc.start_stop=0;
			}

			if ((ElapsedTime > 5000) && (cc.start_stop == 1)) {

				nevents_prev = nevents;
				pthread_mutex_lock(&tree_mutex);
				nevents = this_event.num;
				pthread_mutex_unlock(&tree_mutex);
				nevents_diff = nevents - nevents_prev;

				pthread_mutex_lock(&tree_mutex);
				int_rate = (float) nevents / (float) (TotalElapsedTime);
				int_rate *= 1000.0f;
				if (nevents_diff == 0) {
					diff_rate = 0;
					printf("No data...\n");
				} else {
					diff_rate = (float) nevents_diff * 1000.0f / (float) ElapsedTime;

					//c_energy->Modified();
					//c_energy->Update();

					// printf("%i %i %i %i",nevents_diff,nevents,ElapsedTime,TotalElapsedTime);
					printf("Trg Rate: %.2f Hz (differential), %.2f Hz (integrated) \n", diff_rate, int_rate);
					// for (int jj=0;jj<FADC_CHANNELS_PER_BOARD;jj++) printf(" %i: E: %.2f V: %.2f T: %.2f Ped: %.2f start: %.2f stop: %.2f	\n",jj,this_event.energy[jj],this_event.peak_val[jj],this_event.time[jj],this_event.ped_mean[jj],this_event.peak_start[jj],this_event.peak_end[jj]);
					printf("\n");
					nevents_diff = 0;
				}
				if (rate_counter == 0) {
					gdiff_rate->Set(0);
					gint_rate->Set(0);
				}
				gdiff_rate->SetPoint(rate_counter, rate_counter * 5, diff_rate);
				gint_rate->SetPoint(rate_counter, rate_counter * 5, int_rate);
				rate_counter++;
				//c_waveforms->cd(9);
				gint_rate->Draw("AP");
				gdiff_rate->Draw("PSAME");

				if (rate_counter > n_rate_points) {
					gint_rate->GetXaxis()->SetRangeUser((rate_counter - n_rate_points) * 5, rate_counter * 5);
				}
				gint_rate->GetYaxis()->SetRangeUser(0, int_rate * 2);

				//c_waveforms->Modified();
				//c_waveforms->Update();
				PrevRateTime = CurrentTime;
				pthread_mutex_unlock(&tree_mutex);
			} //end if elapsed time
		} //end if (start_stop==1, i.e. board is acquiring)
		else {
			rate_counter = 0;
			if (flagExit==true){
				printf("End rate thr \n");
				break;
			}
		}
	} //end while(1)
	printf("Exit rate thr\n");
}

int main(int argc, char **argv) {

	gROOT->SetStyle("Plain");
	gStyle->SetPalette(1);
	gStyle->SetNumberContours(100);

	char OutputTime[20];
	char fname[50];
	time_t aclock;
	struct tm *newtime;
	aclock = time(NULL);
	newtime = localtime(&aclock);
	strftime(OutputTime, sizeof(OutputTime), "%Y%m%d%H%M%S", newtime);

	/*Open ROOT file*/
	strcpy(fname, "data");
	strcat(fname, OutputTime);
	strcat(fname, ".root");
	out_file = new TFile(fname, "recreate");
	out_tree = new TTree("out", "out");

	/*Open TXT file*/
	strcpy(fname, "data");
	strcat(fname, OutputTime);
	strcat(fname, ".txt");
	file_txt.open(fname);

	time_t now;
	time(&now);
	printf("Starting at %s", ctime(&now));

	out_tree->Branch("DATA", &this_event.data);

//	int ii;
//	for (ii = 0; ii < BUF_LENGTH; ii++)
//		buf[ii] = 0;


	this_event.num=0;

	cc.start_stop = 0;
	cc.SetBuffer(&buf);
	cc.SetReadMutex(&read_mutex);
	/*Create thread for gui */
	pthread_t GUI_tr;
	pthread_create(&GUI_tr, NULL, gui_fun, NULL);

	/*Create thread for readout */
	pthread_t READ_tr;
	pthread_create(&READ_tr, NULL, read_fun, NULL);

	/*Create thread to print out the rate and monitor*/
	pthread_t RATE_tr;
	pthread_create(&RATE_tr, NULL, rate_fun, NULL);

	pthread_join(RATE_tr, NULL);
	pthread_join(GUI_tr, NULL);
	pthread_join(READ_tr, NULL);

	printf("all threads exit. Save\n");
	file_txt.close();
	out_file->Write();
	out_file->Close();
	printf("Done - end program\n");

	return 1;
}

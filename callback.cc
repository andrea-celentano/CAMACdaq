#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "callback.h"
#include "interface.h"
#include "support.h"
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "TTree.h"
#include "CamacCrate.h"
#include "crate_lib.h"
#include <string>

using namespace std;

/* Soft Trigger */
void on_button6_clicked(GtkButton *button, gpointer user_data) {

	g_print("Software Trigger \n");

}

/*START BUTTON*/
void on_button3_clicked(GtkButton *button, gpointer user_data) {
	CamacCrate *cc;
	cc = (CamacCrate *) user_data;
	/*Disable START again*/
	gtk_widget_set_sensitive((GtkWidget*) button, 0);
	GtkWidget *stop_button;

	/*Disable CONFIGURE */
	stop_button = lookup_widget((GtkWidget*) button, "button2");
	gtk_widget_set_sensitive(stop_button, 0);

	/*Enable STOP */
	stop_button = lookup_widget((GtkWidget*) button, "button4");
	gtk_widget_set_sensitive(stop_button, 1);

	/*DISABLE Quit */
	stop_button = lookup_widget((GtkWidget*) button, "button5");
	gtk_widget_set_sensitive(stop_button, 0);

	/*Enable SOFT TRIGGER */
	stop_button = lookup_widget((GtkWidget*) button, "button6");
	gtk_widget_set_sensitive(stop_button, 1);

	g_print("START ACQUISITION \n");
	cc->start_stop = 1;
//  V1720_board *bd;
//  bd=(V1720_board *)user_data;

}

/*Stop BUTTON*/
void on_button4_clicked(GtkButton *button, gpointer user_data) {
	CamacCrate *cc;
	cc = (CamacCrate *) user_data;
	/*Disable STOP again*/
	gtk_widget_set_sensitive((GtkWidget*) button, 0);
	/*Enable START */
	GtkWidget *start_button;
	start_button = lookup_widget((GtkWidget*) button, "button3");
	gtk_widget_set_sensitive(start_button, 1);

	/*Enable CONFIGURE */
	start_button = lookup_widget((GtkWidget*) button, "button2");
	gtk_widget_set_sensitive(start_button, 1);

	/*ENABLE Quit */
	start_button = lookup_widget((GtkWidget*) button, "button5");
	gtk_widget_set_sensitive(start_button, 1);

	/*Disable Soft Trigger */
	start_button = lookup_widget((GtkWidget*) button, "button6");
	gtk_widget_set_sensitive(start_button, 1);

	g_print("STOP ACQUISITION \n");

	cc->start_stop = 0;

	/* V1720_board *bd;
	 bd=(V1720_board *)user_data;
	 v1720StartStopAcquisition(bd->handle,0);
	 v1720Clear(bd->handle);
	 bd->start_stop=0;
	 */
}

/*QUIT BUTTON. DATA ARE SAVED HERE */
/*BEFORE QUITTING, MAKE SURE THAT *write_buf has read all the data */
void on_button5_clicked(GtkButton *button, gpointer user_data)

{
	CamacCrate *cc;
	cc = (CamacCrate *) user_data;
	printf("Quit button pressed");
	cc->start_stop = 0;
	gtk_main_quit();
}

/*CONFIGURE BUTTON */
void on_button2_clicked(GtkButton *button, gpointer user_data) {
	g_print("CONFIGURE \n");
	CamacCrate *cc;
	cc = (CamacCrate *) user_data;

	ifstream file("configure.dat");
	if (!file.good()) {
		g_print("ERROR, configure.dat not found");
		return;
	}
	/*Read all lines*/
	std::string line, word1, word2, word3;
	vector<string> lines;
	while (std::getline(file, line)) {
		if ((line[0] != '#') && (line.length() != 0)) {
			lines.push_back(line);
		}
	}
	file.close();
	for (int ii = 0; ii < lines.size(); ii++) {
		std::cout << lines[ii] << std::endl;
		std::istringstream iss(lines[ii]);
		iss >> word1;

		/*check word1 and make appropriate actions*/
		if (word1 == "IP") {
			iss >> word2;
			cc->setIP(word2);
		} else if (word1 == "DISC_SLOT") {
			iss >> word2;
			cc->SetDiscriminatorSlot(atoi(word2.c_str()));
		} else if (word1 == "DISC_THR") {
			iss >> word2 >> word3;
			cc->SetDiscriminatorThreshold(atoi(word2.c_str()), atoi(word3.c_str()));
		} else if (word1 == "TDC_SLOT") {
			iss >> word2;
			cc->SetTDCSlot(atoi(word2.c_str()));
		} else if (word1 == "TDC_MODE") {
			iss >> word2;
			cc->SetTDCMode(atoi(word2.c_str()));
		} else if (word1 == "TDC_TIMEOUT") {
			iss >> word2;
			cc->SetTDCTimeout(atoi(word2.c_str()));
		} else if (word1 == "MAX_TIME") {
			iss >> word2;
			cc->setTimeLength(atoi(word2.c_str()));
		} else if (word1 == "MAX_EVENTS") {
			iss >> word2;
			cc->setEventLength(atoi(word2.c_str()));
		}
	}

	/*At the end of configure, start the CamacCrate and configure it*/
	if (cc->Connect() != 0) {
		printf("Error with cc->Connect() - exit \n");
		exit(1);
	}
	cc->Scan();
	if (cc->InitDiscriminator() != 0) {
		printf("Error with cc->InitDiscriminator() - exit \n");
		exit(1);
	}
	if (cc->InitTDC() != 0) {
		printf("Error with cc->InitTDC() - exit \n");
		exit(1);
	}
	cc->is_configured = 1;

	/*Now enable start and disable stop*/
	/*Disable STOP*/
	GtkWidget *stop_button;
	stop_button = lookup_widget((GtkWidget*) button, "button4");
	gtk_widget_set_sensitive(stop_button, 0);

	/*Enable START*/
	GtkWidget *start_button;
	start_button = lookup_widget((GtkWidget*) button, "button3");
	gtk_widget_set_sensitive(start_button, 1);

}

gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
	g_print("delete event occurred\n");
	return FALSE;
	gtk_main_quit();
}


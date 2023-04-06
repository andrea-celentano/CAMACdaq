/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "interface.h"
#include "callback.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget*
create_window1(void) {
	GtkWidget *window1;
	GtkWidget *fixed2;
	GtkWidget *button4;
	GtkWidget *button3;
	GtkWidget *button5;
	GtkWidget *button2;
	GtkWidget *button6;

	window1 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window1), ("V1720 Control"));

	fixed2 = gtk_fixed_new();
	gtk_widget_show(fixed2);
	gtk_container_add(GTK_CONTAINER(window1), fixed2);

	button4 = gtk_button_new_from_stock("gtk-media-stop");
	gtk_widget_show(button4);
	gtk_fixed_put(GTK_FIXED(fixed2), button4, 176, 16);
	gtk_widget_set_size_request(button4, 56, 32);

	button3 = gtk_button_new_from_stock("gtk-media-play");
	gtk_widget_show(button3);
	gtk_fixed_put(GTK_FIXED(fixed2), button3, 112, 16);
	gtk_widget_set_size_request(button3, 64, 32);

	button5 = gtk_button_new_from_stock("gtk-quit");
	gtk_widget_show(button5);
	gtk_fixed_put(GTK_FIXED(fixed2), button5, 248, 16);
	gtk_widget_set_size_request(button5, 56, 32);

	button2 = gtk_button_new_with_mnemonic(("Configure"));
	gtk_widget_show(button2);
	gtk_fixed_put(GTK_FIXED(fixed2), button2, 24, 16);
	gtk_widget_set_size_request(button2, 80, 32);

	button6 = gtk_button_new_with_mnemonic(("Softw Trigg"));
	gtk_widget_show(button6);
	gtk_fixed_put(GTK_FIXED(fixed2), button6, 320, 16);
	gtk_widget_set_size_request(button6, 80, 32);

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF(window1, window1, "window1");
	GLADE_HOOKUP_OBJECT(window1, fixed2, "fixed2");
	GLADE_HOOKUP_OBJECT(window1, button4, "button4");
	GLADE_HOOKUP_OBJECT(window1, button3, "button3");
	GLADE_HOOKUP_OBJECT(window1, button5, "button5");
	GLADE_HOOKUP_OBJECT(window1, button2, "button2");
	GLADE_HOOKUP_OBJECT(window1, button6, "button6");

	return window1;
}


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <cairo.h>
//`pkg-config --cflags --libs gtk+-2.0`
// https://developer.gnome.org/gtk-tutorial/stable/c39.html



static gboolean delete_event(GtkWidget* widget, GdkEvent* event, gpointer data){

  return FALSE;
}


//Callback that will quit the program
static void destroy(GtkWidget* widget, gpointer data){
  gtk_main_quit();
}




int main(int argc, char** argv){
  GtkWidget* window;
  GtkWidget* vbox;
	
  GtkWidget* menubar;
  GtkWidget* fileMenu;
  GtkWidget* fileMi;
  GtkWidget* quitMi;
  
  GtkWidget* editMenu;
  GtkWidget* editMi;
	
  gtk_init(&argc, &argv);

  //instantiate principal parent window
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "openPaint");
  gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);

  //make the menu
  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  menubar = gtk_menu_bar_new();
  fileMenu = gtk_menu_new();

  editMenu = gtk_menu_new();
  editMi = gtk_menu_item_new_with_label("Edit");
  
  fileMi = gtk_menu_item_new_with_label("File");
  quitMi = gtk_menu_item_new_with_label("Quit");

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(editMi), editMenu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), editMi);
  
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileMi), fileMenu);
  gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), quitMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileMi);
  gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    //Add events
  g_signal_connect(G_OBJECT(window), "delete_event",
		   G_CALLBACK(delete_event), NULL);
	
  g_signal_connect(G_OBJECT(window), "destroy",
		   G_CALLBACK(destroy), NULL);

  g_signal_connect(G_OBJECT(quitMi), "activate",
		   G_CALLBACK(gtk_main_quit), NULL);
  
  /*---------------------------- Drawing ------------------------------*/
	
  GtkWidget* darea;

  darea = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(window), darea);

  //g_signal_connect(G_OBJECT(darea), "draw");

  
  /*-------------------------------------------------------------------*/
  
  gtk_widget_show_all(window);
	
  gtk_main();
	
  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "draw.h"

//http://snipplr.com/view/57664/

//static variables     
static GtkWidget* window = NULL;
static cairo_surface_t* surface = NULL;
static GtkWidget* toolbar = NULL;
static GdkColor* color = NULL;

    

/*Create a new surface of the appropriate size to store scribbles
 */
static gboolean scribble_configure_event(GtkWidget *widget,
																				 GdkEventConfigure *event,
																				 gpointer data){
  cairo_t *cr = NULL;
     
  if (surface)
    cairo_surface_destroy(surface);
    
  surface = gdk_window_create_similar_surface(widget -> window,
																							CAIRO_CONTENT_COLOR,
																							widget -> allocation.width,
																							widget -> allocation.height);
     
  /* Initialize the surface to white */
  cr = cairo_create(surface);
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_paint(cr);
  cairo_destroy(cr);
     
  return TRUE;
}
     
/*Redraw the screen from the surface
 */
static gboolean scribble_expose_event(GtkWidget *widget,
																			GdkEventExpose *event,
																			gpointer data){
  cairo_t *cr = NULL;
     
  cr = gdk_cairo_create(widget->window);
	cairo_set_source_surface(cr, surface, 0, 0);
  gdk_cairo_rectangle(cr, &event->area);
  cairo_fill(cr);
  cairo_destroy(cr);
     
  return FALSE;
}
     
/*Draw a rectangle on the screen
 */
static void draw_brush(GtkWidget *widget, gdouble x, gdouble y){
  GdkRectangle update_rect;
  //clear the buffer
  memset(&update_rect, 0, sizeof(GdkRectangle));
     
  cairo_t *cr = NULL;
     
  update_rect.x = x - 2;
  update_rect.y = y - 2;
  update_rect.width = 4;
  update_rect.height = 4;
     
  /* Paint to the surface, where state is stored */
  cr = cairo_create(surface);
	
	if (color)
		gdk_cairo_set_source_color(cr, color);


	gdk_cairo_rectangle(cr, &update_rect);
	cairo_fill(cr);
	cairo_destroy(cr);
     
	/*invalidate the affected region of the drawing area. */
	gdk_window_invalidate_rect(widget->window,
														 &update_rect,
														 FALSE);
}


static gboolean scribble_button_press_event(GtkWidget *widget,
																						GdkEventButton *event,
																						gpointer data){
	if (surface == NULL)
		return FALSE; 
    
	if (event->button == 1)
		draw_brush(widget, event->x, event->y);
  
	return TRUE;
}


static gboolean scribble_motion_notify_event(GtkWidget *widget,
																						 GdkEventMotion *event,
																						 gpointer data) {
	int x = 0;
	int y = 0;
	GdkModifierType state = 0;
     
	if (surface == NULL)
		return FALSE;
    
	gdk_window_get_pointer(event->window, &x, &y, &state);
     
	if (state & GDK_BUTTON1_MASK)
		draw_brush(widget, x, y);
    
	return TRUE;
}
     

     
static void close_window(void){
	window = NULL;
	if (surface) 
		g_object_unref (surface);
 
	surface = NULL;
	gtk_main_quit();
}

static void color_set_event(GtkColorButton* color_button, gpointer data){
	if (color == NULL)
		color = (GdkColor*)malloc(sizeof(GdkColor*));
	
	gtk_color_button_get_color(color_button, color);	
}

static void setup_toolbar(){
	GtkWidget* table;
	GtkWidget* button;

	if (!toolbar){
		toolbar = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(toolbar), "Toolbar");
		gtk_window_set_default_size(GTK_WINDOW(toolbar), 200, 500);
		gtk_widget_set_uposition(toolbar, 240, 260);
		gtk_container_set_border_width(GTK_CONTAINER(toolbar), 20);
	
		g_signal_connect(G_OBJECT(toolbar), "delete-event",
										 G_CALLBACK(gtk_widget_hide_on_delete), NULL);

		table = gtk_table_new(8, 2, TRUE);
		gtk_table_set_row_spacings(GTK_TABLE(table), 2);
		gtk_table_set_col_spacings(GTK_TABLE(table), 2);

		button = gtk_button_new_with_label("Draw");
		gtk_table_attach_defaults(GTK_TABLE(table), button, 0, 1, 0, 1);

		button = gtk_button_new_with_label("Erase");
		gtk_table_attach_defaults(GTK_TABLE(table), button, 1, 2, 0, 1);

		button = gtk_color_button_new();
		gtk_table_attach_defaults(GTK_TABLE(table), button, 0, 2, 6, 8);
		
		g_signal_connect(G_OBJECT(button), "color-set",
										 G_CALLBACK(color_set_event), NULL);

		gtk_container_add(GTK_CONTAINER(toolbar), table);
	}
}


static void setup_window(){
	if (!window){
		//instantiate principal parent window
		window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(window), "openPaint");
		gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
		gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
		gtk_container_set_border_width(GTK_CONTAINER(window), 10);

		g_signal_connect (G_OBJECT(window), "destroy",
											G_CALLBACK (close_window), NULL);
	}
}


static void do_drawing(){
	GtkWidget* da = NULL;

	da = gtk_drawing_area_new();
	gtk_widget_set_size_request(da, 500, 500);
	gtk_container_add(GTK_CONTAINER (window), da);
     
	/* Signals used to handle backing surface */
	g_signal_connect(da, "expose_event",
									 G_CALLBACK(scribble_expose_event), NULL);
      
	g_signal_connect(da, "configure_event",
									 G_CALLBACK(scribble_configure_event), NULL);
     
	/* Event signals */
	g_signal_connect(da, "motion-notify-event",
									 G_CALLBACK(scribble_motion_notify_event), NULL);
 
	g_signal_connect(da, "button-press-event",
									 G_CALLBACK(scribble_button_press_event), NULL);
  
	/* Ask to receive events the drawing area doesn't normally
	 * subscribe to
	 */
	gtk_widget_set_events(da, gtk_widget_get_events (da)
												| GDK_LEAVE_NOTIFY_MASK
												| GDK_BUTTON_PRESS_MASK
												| GDK_POINTER_MOTION_MASK
												| GDK_POINTER_MOTION_HINT_MASK); 
}


int main(int argc, char *argv[]){
	gtk_init (&argc, &argv);
	setup_window();
	setup_toolbar();
	
	do_drawing();
	gtk_widget_show_all(window);
	gtk_widget_show_all(toolbar);
	gtk_main();
	return 0;
}

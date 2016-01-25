#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
#include "draw.h"
#include "server.h"

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "server.h"

//http://snipplr.com/view/57664/

//static variables     
static GtkWidget* window = NULL;
static cairo_surface_t* surface = NULL;
static GtkWidget* toolbar = NULL;
static GdkColor* color = NULL;

/*----------------------------- Data Handling ------------------------------*/

void serialize_gdkColor(guint buff[4], GdkColor* color){
  buff[0] = color -> pixel;
  buff[1] = color -> red;
  buff[2] = color -> green;
  buff[3] = color -> blue;

  /*
  printf("Color\n");
  int i;
  for (i = 0; i < 4; i++)
    printf("\tbuff[%d]: %d\n", i, buff[i]);
  */
}

void serialize_gdkRectangle(gint buff[4], GdkRectangle* rect){
  buff[0] = rect -> x;
  buff[1] = rect -> y;
  buff[2] = rect -> width;
  buff[3] = rect -> width;
  /*
  printf("Rectangle\n");
  int i;
  for (i = 0; i < 4; i++)
    printf("\tbuff[%d]: %d\n", i, buff[i]);
  */
}


/*----------------------------- Drawing Area ------------------------------*/

/*Create a new surface of the appropriate size to store scribbles
 */
gboolean scribble_configure_event(GtkWidget *widget,
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
gboolean scribble_expose_event(GtkWidget *widget,
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
void draw_brush(GtkWidget *widget, gdouble x, gdouble y, int* socket_id){
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
  //send data to server
  gint buff[4];
  guint colorbuff[4];
  serialize_gdkRectangle(buff, &update_rect);
  serialize_gdkColor(colorbuff, color);
  write(*socket_id, buff, sizeof(buff));
  write(*socket_id, colorbuff, sizeof(colorbuff));
}


gboolean scribble_button_press_event(GtkWidget *widget,
																		 GdkEventButton *event,
																		 gpointer data){
  if (surface == NULL)
    return FALSE; 
    
  if (event->button == 1)
    draw_brush(widget, event->x, event->y, data);
  
  return TRUE;
}


gboolean scribble_motion_notify_event(GtkWidget *widget,
																			GdkEventMotion *event,
																			gpointer data) {
  int x = 0;
  int y = 0;
  GdkModifierType state = 0;
     
  if (surface == NULL)
    return FALSE;
    
  gdk_window_get_pointer(event->window, &x, &y, &state);
     
  if (state & GDK_BUTTON1_MASK)
    draw_brush(widget, x, y, data);
    
  return TRUE;
}


void do_drawing(int* socket_id){
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
									 G_CALLBACK(scribble_motion_notify_event), socket_id);
 
  g_signal_connect(da, "button-press-event",
									 G_CALLBACK(scribble_button_press_event), socket_id);
  
  /* Ask to receive events the drawing area doesn't normally
   * subscribe to
   */
  gtk_widget_set_events(da, gtk_widget_get_events (da)
												| GDK_LEAVE_NOTIFY_MASK
												| GDK_BUTTON_PRESS_MASK
												| GDK_POINTER_MOTION_MASK
												| GDK_POINTER_MOTION_HINT_MASK); 
}

     
/*------------------------------- Window -----------------------------------*/
     
void close_window(void){
  window = NULL;
  if (surface) 
    g_object_unref (surface);
 
  surface = NULL;
  gtk_main_quit();
}

void setup_window(){
  GdkCursor* cursor;
  
  if (!window){
    //instantiate principal parent window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "openPaint");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    g_signal_connect (G_OBJECT(window), "destroy",
											G_CALLBACK (close_window), NULL);

    //set initial action to draw
    cursor = gdk_cursor_new(GDK_PENCIL);
    gdk_window_set_cursor(window->window, cursor);

    //set initial color to black
    color = (GdkColor*)malloc(sizeof(GdkColor*));
    color -> red = 255;
    color -> blue = 255;
    color -> green = 255;
  }
}


/*--------------------------------- Toolbar ------------------------------*/

void color_set_event(GtkColorButton* color_button, gpointer data){
  if (color == NULL)
    color = (GdkColor*)malloc(sizeof(GdkColor*));
	
  gtk_color_button_get_color(color_button, color);	
}

void draw_button_click_event(GtkWidget* widget, gpointer data){
  gboolean active;
  GdkCursor* cursor;

  active = gtk_toggle_button_get_active((GtkToggleButton*)widget);
  
  if (!active){
		cursor = gdk_cursor_new(GDK_PENCIL);
		gdk_window_set_cursor(window->window, cursor);
  }
  else {
    gtk_toggle_button_set_active((GtkToggleButton*)widget, TRUE);
  }
}

void erase_button_click_event(GtkWidget* widget, gpointer data){
  GdkCursor* cursor = gdk_cursor_new(GDK_IRON_CROSS);
  gdk_window_set_cursor(window->window, cursor);
}


void setup_toolbar(){
  GtkWidget* table;
  GtkWidget* button;
  GdkColor* col;
  GdkCursor* cursor;

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

    //Drawing
    button = gtk_toggle_button_new_with_label("Draw");
    gtk_table_attach_defaults(GTK_TABLE(table), button, 0, 1, 0, 1);

    g_signal_connect(G_OBJECT(button), "clicked",
										 G_CALLBACK(draw_button_click_event), NULL);
		
    //Erasing
    button = gtk_toggle_button_new_with_label("Erase");
    gtk_table_attach_defaults(GTK_TABLE(table), button, 1, 2, 0, 1);

    g_signal_connect(G_OBJECT(button), "clicked",
										 G_CALLBACK(erase_button_click_event), NULL);

    button = gtk_color_button_new();
    gtk_table_attach_defaults(GTK_TABLE(table), button, 0, 2, 6, 8);
		
    g_signal_connect(G_OBJECT(button), "color-set",
										 G_CALLBACK(color_set_event), NULL);

    gtk_container_add(GTK_CONTAINER(toolbar), table);
  }
}


/*--------------------------------- Main ------------------------------*/


int main(int argc, char *argv[]){
  
  int socket_id;
  char buffer[256];
  int i;

  socket_id = socket( AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in sock;
  sock.sin_family = AF_INET;
  sock.sin_port = htons(MY_PORT);
  inet_aton("127.0.0.1", &(sock.sin_addr));
  bind(socket_id, (struct sockaddr*)&sock, sizeof(sock));

  i = connect(socket_id, (struct sockaddr*)&sock, sizeof(sock));
  printf("<client> Connect returned: %d\n", i);
  if ( i < 0 ){
    printf("Error: %s\n", strerror(errno));
  }
  
  gtk_init (&argc, &argv);
  setup_window();
  setup_toolbar();
	
  do_drawing(&socket_id);
  gtk_widget_show_all(window);
  gtk_widget_show_all(toolbar);
  gtk_main();
  return 0;
}

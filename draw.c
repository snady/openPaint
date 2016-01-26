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
static GtkWidget* da = NULL;
static cairo_surface_t* surface = NULL;
static GtkWidget* toolbar = NULL;
static GdkColor* color = NULL;
static gboolean drawing = TRUE;
gdouble size;


/*----------------------------- Data Handling ------------------------------*/

void* serialize_data(GdkRectangle* rect, GdkColor* color){
	gint rbuff[4];
	guint cbuff[4];
	int size = sizeof(gint)*4 + sizeof(guint)*4;
	void* buff = (void*)malloc(size);

	rbuff[0] = rect -> x;
	rbuff[1] = rect -> y;
	rbuff[2] = rect -> width;
	rbuff[3] = rect -> height;

	cbuff[0] = color -> pixel;
	cbuff[1] = color -> red;
	cbuff[2] = color -> green;
	cbuff[3] = color -> blue;

	memcpy(buff, (void*)rbuff, sizeof(rbuff));
	memcpy(buff + sizeof(rbuff), (void*)cbuff, sizeof(cbuff));

	return buff;
}

void unserialize_data(gint rbuff[4], guint cbuff[4], void* read_buff){
	void* pos = read_buff;
	memcpy(rbuff, pos, sizeof(rbuff));
	memcpy(cbuff, pos + sizeof(rbuff), sizeof(cbuff));
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
		memset(&update_rect, 0, sizeof(GdkRectangle));
     
		cairo_t *cr = NULL;

		if (!size)
			size = 2;
	
		update_rect.x = x - size / 2;
		update_rect.y = y - size / 2;
		update_rect.width = size;
		update_rect.height = size;
     
		/* Paint to the surface, where state is stored */
		cr = cairo_create(surface);
	
		if (drawing)
			gdk_cairo_set_source_color(cr, color);
	
		else {
			GdkColor col;
			gdk_color_parse("white", &col);
			gdk_cairo_set_source_color(cr, &col);
		}

		gdk_cairo_rectangle(cr, &update_rect);
		cairo_fill(cr);
		cairo_destroy(cr);

		void* buff = serialize_data(&update_rect, color);
	
		write(*socket_id, buff, sizeof(guint)*4+sizeof(gint)*4);

		free(buff);

		/*invalidate the affected region of the drawing area. */
		gdk_window_invalidate_rect(widget->window,
															 &update_rect,
															 FALSE);
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
	da = gtk_drawing_area_new();
	gtk_widget_set_size_request(da, 500, 500);
	gtk_container_add(GTK_CONTAINER (window), da);
     
	g_signal_connect(da, "expose_event",
									 G_CALLBACK(scribble_expose_event), NULL);
      
	g_signal_connect(da, "configure_event",
									 G_CALLBACK(scribble_configure_event), NULL);
     
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
     
void close_window(){
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
		//cursor = gdk_cursor_new(GDK_PENCIL);
		//gdk_window_set_cursor(window->window, cursor);

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
	GdkCursor* cursor;

	cursor = gdk_cursor_new(GDK_PENCIL);
	gdk_window_set_cursor(window->window, cursor);
	
	drawing = TRUE;
}

void erase_button_click_event(GtkWidget* widget, gpointer data){
	GdkCursor* cursor;
	
	cursor = gdk_cursor_new(GDK_IRON_CROSS);
		gdk_window_set_cursor(window->window, cursor);

		drawing = FALSE;
	}


	void scale_change_event(GtkWidget* widget, gpointer data){
		GtkAdjustment* range;
	
		range = gtk_range_get_adjustment(GTK_RANGE(widget));
		size = gtk_adjustment_get_value(range);
	}

	void setup_toolbar(){
		GtkWidget* table;
		GtkWidget* button;
		GtkWidget* hscale;
		GtkAdjustment* range;
		GdkColor* col;

		if (!toolbar){
			toolbar = gtk_window_new(GTK_WINDOW_TOPLEVEL);
			gtk_window_set_title(GTK_WINDOW(toolbar), "Toolbar");
			gtk_window_set_default_size(GTK_WINDOW(toolbar), 200, 500);
			gtk_widget_set_uposition(toolbar, 240, 260);
			gtk_container_set_border_width(GTK_CONTAINER(toolbar), 20);
	
			g_signal_connect(G_OBJECT(toolbar), "delete-event",
											 G_CALLBACK(gtk_widget_hide_on_delete), NULL);

			table = gtk_table_new(9, 2, TRUE);
			gtk_table_set_row_spacings(GTK_TABLE(table), 2);
			gtk_table_set_col_spacings(GTK_TABLE(table), 2);

			//Drawing
			button = gtk_button_new_with_label("Draw");
			gtk_table_attach_defaults(GTK_TABLE(table), button, 0, 1, 0, 1);

			g_signal_connect(G_OBJECT(button), "clicked",
											 G_CALLBACK(draw_button_click_event), NULL);

			//Erasing
			button = gtk_button_new_with_label("Erase");
			gtk_table_attach_defaults(GTK_TABLE(table), button, 1, 2, 0, 1);

			g_signal_connect(G_OBJECT(button), "clicked",
											 G_CALLBACK(erase_button_click_event), NULL);

			button = gtk_color_button_new();
			gtk_table_attach_defaults(GTK_TABLE(table), button, 0, 2, 6, 8);

		
			g_signal_connect(G_OBJECT(button), "color-set",
											 G_CALLBACK(color_set_event), NULL);

			//hscale
			hscale = gtk_hscale_new_with_range(2, 30, 1);
			gtk_table_attach_defaults(GTK_TABLE(table), hscale, 0, 2, 4, 5);
  
			g_signal_connect(G_OBJECT(hscale), "value-changed",
											 G_CALLBACK(scale_change_event), NULL);

			gtk_container_add(GTK_CONTAINER(toolbar), table);
		}
	}

	/*-------------------------------- Server Data -------------------------------*/
	void draw_from_server(gint rectbuff[4], guint colorbuff[4]){
		GdkRectangle update_rect;
		GdkColor col;
		cairo_t* cr = NULL;

		/* Create and set GdkRectangle values based on rectbuff */
		memset(&update_rect, 0, sizeof(GdkRectangle));
		update_rect.x = rectbuff[0];
		update_rect.y = rectbuff[1];
		update_rect.width = rectbuff[2];
		update_rect.height = rectbuff[3];

		/* Create and set GdkColor values based on colorbuff */
		memset(&col, 0, sizeof(GdkColor));
		col.pixel = colorbuff[0];
		col.red = colorbuff[1];
		col.green = colorbuff[2];
		col.blue = colorbuff[3];

		cr = cairo_create(surface);

		gdk_cairo_set_source_color(cr, &col);

		gdk_cairo_rectangle(cr, &update_rect);
		cairo_fill(cr);
		cairo_destroy(cr);

		gdk_window_invalidate_rect(da -> window,
															 &update_rect,
															 FALSE);
	}

	/*--------------------------------- Main ------------------------------*/

void read_from_server(gpointer data, gint source, GdkInputCondition condition){
	guint cbuff[4];
	gint rbuff[4];
	char rd_buffer[256];
	
	read(source, rd_buffer, sizeof(gint)*4+sizeof(guint)*4);
	//printf("%s\n", rd_buffer);
  unserialize_data(rbuff, cbuff, rd_buffer);

	draw_from_server(rbuff, cbuff);
}


int main(int argc, char *argv[]){
	int socket_id;
	int i;

	socket_id = socket( AF_INET, SOCK_STREAM, 0);
	printf("<draw>Socket id: %d\n", socket_id);
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

	gdk_input_add(socket_id, GDK_INPUT_READ, read_from_server, NULL);

	gtk_main();
}




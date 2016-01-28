#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>

#include "draw.h"
#include "select.h"

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


//static variables     
static GtkWidget* window = NULL;
static GtkWidget* da = NULL;
static cairo_surface_t* surface = NULL;
static GtkWidget* toolbar = NULL;
static GdkColor* color = NULL;
static gboolean drawing = TRUE;
gdouble size;


/*----------------------------- Data Handling ------------------------------*/

void serialize_data(GdkRectangle* rect, GdkColor* color, char* buff){
	gint rbuff[4];
	char* pos = buff;
	gchar* cstring = gdk_color_to_string(color);
	int j;
	
	rbuff[0] = rect -> x;
	rbuff[1] = rect -> y;
	rbuff[2] = rect -> width;
	rbuff[3] = rect -> height;
	
	for (j = 0; j < 4; j++){
		sprintf(pos, "%d", rbuff[j]);
		pos = strchr(pos, '\0');
		*pos = '^';
		pos++;
	}	
	strcpy(pos, cstring);
}

void unserialize_data(GdkRectangle* rect, GdkColor* col, char* read_buff){
	char** data = parse(read_buff, '^');
	char* ptr;
	int i;

	memset(rect, 0, sizeof(GdkRectangle));
	rect -> x = strtol(data[0], &ptr, 10);
	rect -> y = strtol(data[1], &ptr, 10);
	rect -> width = strtol(data[2], &ptr, 10);
	rect -> height = strtol(data[3], &ptr, 10);

	gdk_color_parse(data[4], col);
	free(data);
}

char** parse(char* line, char dlimit){
	char** ret = (char**)malloc(sizeof(char*)*5);
	char lim[2] = {dlimit, '\0'};
	char* tmp = line;
	int i = 0;
	
	while (i < 5){
		tmp = strsep(&line, lim);
		if (*tmp){
			ret[i] = tmp;
			i++;
		}
	}
	return ret;
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
		cairo_t* cr;
		char buff[PACKSIZE];

		memset(&update_rect, 0, sizeof(GdkRectangle));
     
		cr = NULL;

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

	
		serialize_data(&update_rect, color, buff);
		write(*socket_id, buff, strlen(buff)+1);

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
	GtkWidget* da;
	
	da = gtk_drawing_area_new();
	gtk_widget_set_size_request(da, 500, 500);
	gtk_container_add(GTK_CONTAINER (window), da);

	gdk_input_add(*socket_id, GDK_INPUT_READ, read_from_server, da);
     
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
void draw_from_server(GdkRectangle* rect, GdkColor* col, GtkWidget* widget){
	cairo_t* cr = NULL;

	cr = cairo_create(surface);
	gdk_cairo_set_source_color(cr, col);
	gdk_cairo_rectangle(cr, rect);
	cairo_fill(cr);
	cairo_destroy(cr);

	gdk_window_invalidate_rect(widget -> window,
														 rect,
														 FALSE);
}

/*--------------------------------- Main ------------------------------*/

void read_from_server(gpointer data, gint source, GdkInputCondition condition){
	char rd_buffer[PACKSIZE];
	GdkColor col;
	GdkRectangle rect;

	read(source, rd_buffer, PACKSIZE);
	unserialize_data(&rect, &col, rd_buffer);
	draw_from_server(&rect, &col, data);
}


int main(int argc, char *argv[]){
	int socket_id;
	int i;

	socket_id = socket( AF_INET, SOCK_STREAM, 0);
	printf("<draw>Socket id: %d\n", socket_id);
	struct sockaddr_in sock;
  
	sock.sin_family = AF_INET;
	sock.sin_port = htons(PORT);
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
}




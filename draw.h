#ifndef DRAW
#define DRAW


void* serialize_gdkColor(GdkColor* color);

void* serialize_gdkRectangle(GdkRectangle* rect);

void unserialize_gdkColor(guint buff[4], void* read_buff);

void unserialize_gdkRectangle(gint buff[4], void* read_buff);

void read_from_server(gpointer data, gint source, GdkInputCondition condition);

gboolean scribble_configure_event(GtkWidget *widget,
																	GdkEventConfigure *event,
																	gpointer data);
     

gboolean scribble_expose_event(GtkWidget *widget,
															 GdkEventExpose *event,
															 gpointer data);
     

void draw_brush(GtkWidget *widget, gdouble x, gdouble y, int* socket_id);


gboolean scribble_button_press_event(GtkWidget *widget,
																		 GdkEventButton *event,
																		 gpointer data);


gboolean scribble_motion_notify_event(GtkWidget *widget,
																			GdkEventMotion *event,
																			gpointer data); 


void do_drawing(int* socket_id);
      
void close_window();

void setup_window();


void color_set_event(GtkColorButton* color_button, gpointer data);

void draw_button_click_event(GtkWidget* widget, gpointer data);

void erase_button_click_event(GtkWidget* widget, gpointer data);

void setup_toolbar();

#endif

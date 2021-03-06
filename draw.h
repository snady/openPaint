#ifndef DRAW
#define DRAW


void serialize_data(GdkRectangle* rect, GdkColor* color, char* buff);

void unserialize_data(GdkRectangle* rect, GdkColor* col, char* read_buff);

char** parse(char* line, char dlimit);


void read_from_server(gpointer data, gint source, GdkInputCondition condition);

void draw_from_server(GdkRectangle* rect, GdkColor* col, GtkWidget* widget);

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

#ifndef DRAW
#define DRAW


static gboolean scribble_configure_event(GtkWidget*,
																				 GdkEventConfigure*,
																				 gpointer);

static gboolean scribble_expose_event(GtkWidget*, GdkEventExpose*, gpointer);
     
static void draw_brush(GtkWidget *widget, gdouble x, gdouble y);
     
static gboolean scribble_button_press_event(GtkWidget*,
																						GdkEventButton*,
																						gpointer);

static gboolean scribble_motion_notify_event(GtkWidget*,
																						 GdkEventMotion*,
																						 gpointer);

static void close_window(void);

static void setup_window();

GtkWidget* do_drawingarea();


#endif

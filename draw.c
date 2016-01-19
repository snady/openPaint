#include <string.h>
#include <gtk/gtk.h>

//http://snipplr.com/view/57664/

//static variables     
static GtkWidget *window = NULL;
static cairo_surface_t *surface = NULL;

//functions
static gboolean scribble_configure_event (GtkWidget*, GdkEventConfigure*, gpointer);

static gboolean scribble_expose_event (GtkWidget*, GdkEventExpose*, gpointer);
     
static void draw_brush (GtkWidget *widget, gdouble x, gdouble y);
     
static gboolean scribble_button_press_event (GtkWidget*, GdkEventButton*, gpointer);
static gboolean scribble_motion_notify_event (GtkWidget*, GdkEventMotion*, gpointer);
static void close_window (void);
GtkWidget* do_drawingarea ();
     
/*!
 * \brief Create a new surface of the appropriate size to store our scribbles
 */
static gboolean scribble_configure_event (GtkWidget *widget,
					  GdkEventConfigure *event,
					  gpointer data){
  cairo_t *cr = NULL;
     
  if (surface)
    cairo_surface_destroy (surface);
    
  surface = gdk_window_create_similar_surface(widget -> window,
					      CAIRO_CONTENT_COLOR,
					      widget -> allocation.width,
					      widget -> allocation.height);
     
  /* Initialize the surface to white */
  cr = cairo_create (surface);
  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_paint (cr);
  cairo_destroy (cr);
     
  return TRUE;
}
     
/*!
 * \brief Redraw the screen from the surface
 */
static gboolean scribble_expose_event (GtkWidget *widget,
				       GdkEventExpose *event,
				       gpointer data){
  cairo_t *cr = NULL;
     
  cr = gdk_cairo_create (widget->window);
  cairo_set_source_surface (cr, surface, 0, 0);
  gdk_cairo_rectangle (cr, &event->area);
  cairo_fill (cr);
     
  cairo_destroy (cr);
     
  return FALSE;
}
     
/*!
 * \brief Draw a rectangle on the screen
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
     
  /* Paint to the surface, where we store our state */
  cr = cairo_create(surface);
  gdk_cairo_rectangle(cr, &update_rect);
  cairo_fill(cr);
  cairo_destroy(cr);
     
  /* Now invalidate the affected region of the drawing area. */
  gdk_window_invalidate_rect (widget->window,
			      &update_rect,
			      FALSE);
}


static gboolean scribble_button_press_event (GtkWidget *widget,
					     GdkEventButton *event,
					     gpointer data){
  if (surface == NULL)
    return FALSE; 
    
  if (event->button == 1)
    draw_brush (widget, event->x, event->y);
  
  return TRUE;
}


static gboolean scribble_motion_notify_event (GtkWidget *widget,
					      GdkEventMotion *event,
					      gpointer data) {
  int x = 0;
  int y = 0;
  GdkModifierType state = 0;
     
  if (surface == NULL)
      return FALSE;
    
  gdk_window_get_pointer(event->window, &x, &y, &state);
     
  if (state & GDK_BUTTON1_MASK)
      draw_brush (widget, x, y);
    
  return TRUE;
}
     

     
static void close_window (void){
  window = NULL;
  if (surface) 
    g_object_unref (surface);
 
  surface = NULL;
  gtk_main_quit();
}



static void setupWindow(){
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
  gtk_widget_set_size_request (da, 500, 500);
  gtk_container_add (GTK_CONTAINER (window), da);
     
  /* Signals used to handle backing surface */
  g_signal_connect(da, "expose_event",
		   G_CALLBACK (scribble_expose_event), NULL);
      
  g_signal_connect(da, "configure_event",
		   G_CALLBACK (scribble_configure_event), NULL);
     
  /* Event signals */
  g_signal_connect(da, "motion-notify-event",
		   G_CALLBACK (scribble_motion_notify_event), NULL);
 
  g_signal_connect(da, "button-press-event",
		   G_CALLBACK (scribble_button_press_event), NULL);
  
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
  setupWindow();
  do_drawing();
  gtk_widget_show_all(window);
  gtk_main();
  return 0;
}

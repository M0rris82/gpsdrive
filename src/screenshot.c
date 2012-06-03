#include <gdk/gdktypes.h>
#include <gtk/gtk.h>
#include "gpsproto.h"
#include "gpsdrive_config.h"
#include "poi_gui.h"


extern GtkWidget *settings_window;
extern GtkWidget *main_window;
extern GtkWidget *poi_lookup_window;

guint TimeoutEvent = 0;

/*
 * take a screenshot of a GdkWindow-Object
 */
gint take_screenshot(GdkWindow *window, gchar *filename) {
	GdkPixbuf *image;
	gint width = 0;
	gint height = 0;
	gchar FilePath[1500];
	
	g_snprintf (FilePath, sizeof (FilePath),
		"%s/%s.png",
		local_config.screenshot_dir, filename);
	
	gdk_window_process_updates(window, FALSE);

	/* wait for gtk events to finish */
	while (gtk_events_pending())
		gtk_main_iteration();
	
	/* now take the screenshoot */
	gdk_drawable_get_size(window, &width, &height);
	image = gdk_pixbuf_get_from_drawable(NULL, window, gdk_drawable_get_colormap(window) ,0 , 0, 0, 0, width, height);
	gdk_pixbuf_save(image, FilePath, "png", NULL, NULL);
	g_object_unref(image);
	return 0;
}

gint auto_take_screenshots_cb() {
	static gint JobId = 0; 
	g_source_remove(TimeoutEvent);

	switch(JobId) {
	case 0:
		if (GTK_WIDGET_DRAWABLE(main_window))
			take_screenshot(main_window->window, "window");
		++JobId;
		break;
	case 1:
		settings_main_cb(NULL, 0);
		++JobId;
		break;
	case 2:
		if (GTK_WIDGET_DRAWABLE(settings_window))
			take_screenshot(settings_window->window, "settings");
		++JobId;
	case 3:
		show_poi_lookup_cb(NULL, 0);
		++JobId;
		break;
	case 4:
		if (GTK_WIDGET_DRAWABLE(poi_lookup_window))
			take_screenshot(poi_lookup_window->window, "poi");
		++JobId;
		break;
	case 5:
		quit_program_cb(NULL, NULL);
		break;
	}
	
	TimeoutEvent = g_timeout_add (2000, (GtkFunction) auto_take_screenshots_cb, NULL);
	return FALSE;
}

/*
 * take screenshots from different settings and windows
 */
gint auto_take_screenshots() {
	TimeoutEvent = g_timeout_add (5000, (GtkFunction) auto_take_screenshots_cb, NULL);
	return 0;
}

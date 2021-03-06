/* ***********************************************************************  */
/*  Prototypes */
//int initkismet (void);
//int readkismet (void);
int deletesqldata (int index);
int sqlinit (void);
void sqlend (void);
int loadmap (char *filename);
gint zoom_cb (GtkWidget * widget, guint datum);
void show_splash (void);
void display_status (char *message);
gint drawmarker (GtkWidget * widget, guint * datum);
void savemapconfig ();
gint loadmapconfig ();
void loadwaypoints ();
void savewaypoints ();
void storepoint();

gdouble calcdist (gdouble lon, gdouble lat);
gdouble calcdist2 (gdouble lon, gdouble lat);
gdouble calc_wpdist (gdouble, gdouble, gdouble, gdouble, gint);
void inv_geodesic(gdouble, gdouble, gdouble, gdouble, gint, gdouble *, gdouble *, gdouble *);

gint help_cb (GtkWidget * widget, guint datum);
gint import1_cb (GtkWidget * widget, guint datum);

gint friendsagent_cb (GtkWidget * widget, guint * datum);
void writeconfig ();
void readconfig ();
gint create_route_cb (GtkWidget * widget, guint datum);
void insertroutepoints ();
void setroutetarget ();
gint initgps ();
gint defaultserver_cb (GtkWidget * widget, guint datum);
void saytargettext (gchar * filename, gchar * target);
void display_dsc (void);
void coordinate2gchar (gchar * text, gint buff_size, gdouble pos, gint islat, gint mode);
void checkinput (gchar * text);
void mintodecimal (gchar * text);
void mainsetup (void);
void infos (void);
gint removesetutc (GtkWidget * widget, guint datum);
gint nav_doit (GtkWidget * widget, guint * datum);
gint expose_cb (GtkWidget * widget, guint * datum);
gint expose_compass (GtkWidget * widget, guint * datum);
gint expose_mini_cb (GtkWidget * widget, guint * datum);
void setup_poi (void);
int friends_sendmsg (char *serverip, char *message);
int friends_init ();
void friendssetup (void);
gint quit_program_cb (GtkWidget * widget, gpointer datum);
gint loadtrack_cb (GtkWidget * widget, gpointer datum);
gint storetrack_cb (GtkWidget * widget, guint * datum);
gint about_cb (GtkWidget * widget, guint datum);
gint settings_main_cb (GtkWidget *widget, guint datum);
gint sel_message_cb (GtkWidget * widget, guint datum);
gint setmessage_cb (GtkWidget * widget, guint datum);
GdkPixbuf *create_pixbuf (const gchar * filename);
gint simulated_pos (GtkWidget * widget, guint * datum);
int create_nasa_mapfile (double lat, double lon, int test, char *fn);
int init_nasa_mapfile ();
void cleanup_nasa_mapfile ();
gint checksum (gchar * text);
void daylights (void);
gint setutc (GtkWidget * widget, guint datum);
G_MODULE_EXPORT gint modulesetup ();
gint message_cb (char *msgid, char *name, char *text, int fs);
void exiterr (int exitcode);
void calcxy (gint * posx, gint * posy, gdouble lon, gdouble lat, gint zoom);
void minimap_xy2latlon(gint x, gint y, gdouble *lon, gdouble *lat, gdouble *dif);
void rebuildtracklist (void);
gint error_popup (gpointer datum);
void calcxymini (gint * posx, gint * posy, gdouble lon, gdouble lat, gint zoom);
gdouble calcR (gdouble lat);
void calcxytopos (int , int , gdouble *, gdouble *, int );
gint navi_cb (GtkWidget * widget, guint datum);
GtkWidget* create_pixmap(GtkWidget *widget, const gchar *filename);
gint speech_out_cb (GtkWidget * widget, guint * datum);
gchar *escape_sql_string (const gchar *data);
void distance2gchar (gdouble dist, gchar *diststring, gint diststr_size, gchar *unitstring, gint unitstr_size);
gint check_if_night_cb (void);
void cleanup_friends (void);

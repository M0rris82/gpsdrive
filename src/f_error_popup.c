

gint
error_popup (gpointer datum)
{
  GtkWidget *popup;
  GtkWidget *knopf2;
  GtkWidget *knopf3;
  GtkWidget *image;


  if (errortextmode)
    {
      g_print ("\nError: %s\n", (char *) datum);
      return (FALSE);
    }
  popup = gtk_dialog_new ();
  gtk_window_set_transient_for (GTK_WINDOW (popup), GTK_WINDOW (mainwindow));

  image =
    gtk_image_new_from_stock (GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_DIALOG);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (popup)->vbox), image,
		      FALSE, FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (popup), 5);

  gtk_window_set_title (GTK_WINDOW (popup), _("  Message  "));
  knopf2 = gtk_button_new_from_stock (GTK_STOCK_OK);
  GTK_WIDGET_SET_FLAGS (knopf2, GTK_HAS_FOCUS);

  gtk_signal_connect_object (GTK_OBJECT (knopf2), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (popup));

  gdk_beep ();
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (popup)->action_area), knopf2, TRUE,
		      TRUE, 20);
  knopf3 = gtk_label_new (datum);
  gtk_label_set_use_markup (GTK_LABEL (knopf3), TRUE);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (popup)->vbox), knopf3, FALSE,
		      FALSE, 10);


  gtk_window_set_position (GTK_WINDOW (popup), GTK_WIN_POS_CENTER);
  gtk_widget_show (image);
  gtk_widget_show_all (popup);

  return (FALSE);

}



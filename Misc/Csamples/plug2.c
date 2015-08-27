
/*
    Test code for plugs and sockets with interprocess communication. For it to work, both the
plug2.c and socket2.c files need to be compiled and then run socket2.

    gcc -Wall plug2.c -o plug2 `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<gtk/gtkx.h>
#include<stdio.h>

static void on_embed(GtkPlug *plug, gpointer data)
  {
    g_print("Plug Embedded\n"); 
  }
static gboolean send_out(gpointer data)
  {
    static int counter=0;
    g_print("%i\n", (gint)gtk_plug_get_id(GTK_PLUG(data)));
    counter++;
    if(counter>10)
      {
        g_print("Finish Broadcasting Window ID\n");
        return FALSE;
      }
    else return TRUE;
  }
static void disconnect_plug(GtkWidget *widget, gpointer data)
  {
    g_print("dis\n");
  }
int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *plug=gtk_plug_new(0);
    gtk_widget_set_size_request(plug, 200, 100);
    g_signal_connect(plug, "embedded", G_CALLBACK(on_embed), NULL);

    GtkWidget *label=gtk_label_new("");
    gchar *markup=NULL;
    if(argc>1)
      {
        markup=g_strdup_printf("<span foreground='%s' size='xx-large'>Plug</span>", argv[1]);
      }
    else
      {
        markup=g_strdup("<span foreground='black' size='xx-large'>Plug</span>");
      }
    gtk_label_set_markup(GTK_LABEL(label), markup);
    gtk_widget_set_hexpand(label, TRUE);
    gtk_widget_set_vexpand(label, TRUE);
    g_free(markup);

    GtkWidget *button=gtk_button_new_with_label("Disconnect Plug");
    gtk_widget_set_hexpand(button, TRUE);
    g_signal_connect(button, "clicked", G_CALLBACK(disconnect_plug), NULL);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(plug), grid);

    gtk_widget_show_all(plug);

    //Broadcast plug id. 
    g_timeout_add(500, (GSourceFunc)send_out, plug);

    gtk_main();
    
    return 0;   
  }

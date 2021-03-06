
/*

    Test the NIST time servers with glib/GTK. Try the "TIME" protocol and NTP. The program adjusts the
NTP fraction of a second to micro seconds to compare with system time.
    Tested with Ubuntu14.04 and GTK3.10. 
   
    NIST: http://tf.nist.gov/tf-cgi/servers.cgi
    Details: https://tools.ietf.org/html/rfc5905

    gcc -Wall nist_time1.c -o nist_time1 `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<string.h>
#include<time.h>

#define LITTLE_ENDIAN1 0
#define BIG_ENDIAN1    1

static void check_four_seconds(GtkWidget *button, gpointer *data);
static void nist_atomic_time(GtkWidget *widget, gpointer *data);
//http://www.ibm.com/developerworks/aix/library/au-endianc/
unsigned long int unpacku32_lendian(unsigned char *buffer);
int check_endianess();
static GdkPixbuf* draw_icon();
static void about_dialog(GtkWidget *widget, gpointer data);

int main(int argc, char *argv[])
  {   
    gtk_init(&argc, &argv);
  
    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "NIST Atomic Time");
    gtk_container_set_border_width(GTK_CONTAINER(window),10);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 300);
    g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit), NULL);

    GdkPixbuf *icon=draw_icon();
    gtk_window_set_default_icon(icon);

    GtkWidget *menu1=gtk_menu_new();
    GtkWidget *menu1item1=gtk_menu_item_new_with_label("NIST Atomic Time");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu1), menu1item1);
    GtkWidget *title1=gtk_menu_item_new_with_label("About");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(title1), menu1);
    GtkWidget *menu_bar=gtk_menu_bar_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), title1);
    g_signal_connect(menu1item1, "activate", G_CALLBACK(about_dialog), window);

    GtkWidget *radio1=gtk_radio_button_new_with_label(NULL, "NTP Time");
    GtkWidget *radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Time protocol");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio1), TRUE);

    GtkWidget *label1=gtk_label_new("Time Label");
    gtk_widget_set_hexpand(label1, TRUE);
    gtk_widget_set_vexpand(label1, TRUE);

    gpointer widgets[]={label1, radio1};

    GtkWidget *button1=gtk_button_new_with_label("Get NIST Time");
    gtk_widget_set_hexpand(button1, TRUE);
    g_signal_connect(button1, "clicked", G_CALLBACK(check_four_seconds), widgets);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_attach(GTK_GRID(grid), radio1, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio2, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 2, 4, 1);
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 3, 4, 1);
    gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 4, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);

    GError *css_error=NULL;
    gchar css_string[]="GtkWindow{background-image: -gtk-gradient (linear, left center, right center, color-stop(0.0,rgba(255,255,0,0.8)), color-stop(0.5,rgba(255,170,0,0.8)), color-stop(1.0,rgba(255,0,0,0.8)));}GtkButton{background: rgba(220,220,220,0.5);}";
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_css_provider_load_from_data(provider, css_string, -1, &css_error);
    if(css_error!=NULL) g_print("CSS loader error %s\n", css_error->message);
    g_object_unref(provider);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
  }
static void check_four_seconds(GtkWidget *button, gpointer *data)
  {
    static long seconds=-1;
    time_t time1=time(NULL);
    
    if(time1-seconds>4||seconds==-1) nist_atomic_time(button, data);
    else
      {
        gchar *error_string=g_strdup_printf("Wait four seconds between time request. Time diff %ld", time1-seconds);
        g_print("%s\n", error_string);
        gtk_label_set_text(GTK_LABEL(data[0]), error_string);
        g_free(error_string);
      }
    seconds=time1;
  }
static void nist_atomic_time(GtkWidget *widget, gpointer *data)
  {
    GError *error=NULL;
    gchar *ntp_server="time.nist.gov";
    gchar buffer[48];
    gboolean fail_error=FALSE;
    gboolean ntp_time=TRUE; //FALSE for time protocol on port 37.
    gchar *protocol="NTP";

    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data[1]))) ntp_time=FALSE;

    memset(buffer, '\0', 48*sizeof(gchar));
    // Initialize values needed to form NTP request
    //From https://www.arduino.cc/en/Tutorial/UdpNTPClient
    if(ntp_time)
      {
        buffer[0] = 0b11100011;   // LI, Version, Mode
        buffer[1] = 0;     // Stratum, or type of clock
        buffer[2] = 6;     // Polling Interval
        buffer[3] = 0xEC;  // Peer Clock Precision
        // 8 bytes of zero for Root Delay & Root Dispersion
        buffer[12]  = 49;
        buffer[13]  = 0x4E;
        buffer[14]  = 49;
        buffer[15]  = 52;
      }

    GSocketConnection *connection=NULL;
    GSocketClient *client=g_socket_client_new();
    g_socket_client_set_timeout(client, 2);
    g_socket_client_set_socket_type(client, G_SOCKET_TYPE_DATAGRAM);
    //Set the connection.
    if(ntp_time) connection=g_socket_client_connect_to_host(client, ntp_server, 123, NULL, &error);
    else connection=g_socket_client_connect_to_host(client, ntp_server, 37, NULL, &error);

    if(error!=NULL)
      {
        gchar *error_string=g_strdup_printf("Connection Error: %s", error->message);
        g_print("%s\n", error_string);
        gtk_label_set_text(GTK_LABEL(data[0]), error_string);
        fail_error=TRUE;
        g_free(error_string);
      }

    GSocket *sock=NULL;
    if(!fail_error)
      {
        sock=g_socket_connection_get_socket(connection);
        if(sock==NULL)
          {
            gchar *error_string=g_strdup("Socket From Connection Error");
            g_print("%s\n", error_string);
            gtk_label_set_text(GTK_LABEL(data[0]), error_string);
            fail_error=TRUE;
            g_free(error_string);
          }
      }

    if(!fail_error)
      {
        g_socket_send(sock, buffer, 48, NULL, &error);
        if(error!=NULL)
          {
            gchar *error_string=g_strdup_printf("Send Error: %s", error->message);
            g_print("%s\n", error_string);
            gtk_label_set_text(GTK_LABEL(data[0]), error_string);
            fail_error=TRUE;
            g_free(error_string);
          }
      }

    gint length=0;
    if(!fail_error)
      {
        length=g_socket_receive(sock, buffer, 48, NULL, &error);
        if(error!=NULL)
          {
            gchar *error_string=g_strdup_printf("Receive Error: %s", error->message);
            g_print("%s\n", error_string);
            gtk_label_set_text(GTK_LABEL(data[0]), error_string);
            fail_error=TRUE;
            g_free(error_string);
          }   
      }

   if(!fail_error)
      {
        GString *string_builder=g_string_new(NULL);
        gint64 system_time=g_get_real_time();
        time_t time1=(time_t)(system_time/1e6); //time in seconds
        time_t time2;
        gint64 system_micro=system_time-(gint64)((gint64)(system_time/1e6)*1e6);
        gint64 ntp_micro=0;
        gchar s_buffer[5];
        gchar u_buffer[5];
        memset(s_buffer, '\0', 5*sizeof(gchar));
        memset(u_buffer, '\0', 5*sizeof(gchar));

        if(ntp_time)
          {
            //Get the time values.
            s_buffer[0]=buffer[40];
            s_buffer[1]=buffer[41];
            s_buffer[2]=buffer[42];
            s_buffer[3]=buffer[43];
            u_buffer[0]=buffer[44];
            u_buffer[1]=buffer[45];
            u_buffer[2]=buffer[46];
            u_buffer[3]=buffer[47];
          }
        else
          {
            s_buffer[0]=buffer[0];
            s_buffer[1]=buffer[1];
            s_buffer[2]=buffer[2];
            s_buffer[3]=buffer[3];
          } 
       
        if(check_endianess()==LITTLE_ENDIAN1)
          {
            time2=unpacku32_lendian((unsigned char*)s_buffer);
            ntp_micro=unpacku32_lendian((unsigned char*)u_buffer);
          }
        else
          {
            time2=g_ascii_strtoll(s_buffer, NULL, 10);
            ntp_micro=g_ascii_strtoll(u_buffer, NULL, 10);
          }

        //Adjust NTP pico seconds to micro seconds.
        if(ntp_time) ntp_micro=((ntp_micro*1000000)>>32);
 
        //Adjust time to start at Jan 1, 1970.
        time2=time2-(2208988800ul);
        if(ntp_time)
          {
            g_print("Returned Buffer Length %i, Client Time %lu.%llu, NIST %s Time %lu.%llu\n", length, time1, system_micro, protocol, time2, ntp_micro);
            g_string_append_printf(string_builder, "Returned Buffer Length %i, Client Time %lu.%llu, NIST %s Time %lu.%llu\n", length, time1, system_micro, protocol, time2, ntp_micro);
          }
        else
          {
            g_print("Returned Buffer Length %i, Client Time %lu, NIST Time %lu\n", length, time1, time2);
            g_string_append_printf(string_builder, "Returned Buffer Length %i, Client Time %lu, NIST Time %lu\n", length, time1, time2);
          }

        //Format some date time strings.
        size_t time_length=0;
        struct tm info1;
        struct tm info2;
        localtime_r(&time1, &info1);
        localtime_r(&time2, &info2);
        gchar time_buffer1[256];
        gchar time_buffer2[256];
        time_length=strftime(time_buffer1, 256, "Local Client Time: %A, %B %d,  %I:%M:%S %p %Y", &info1);
        if(time_length!=0)
          {
            g_print("%s\n", time_buffer1);
            g_string_append_printf(string_builder, "%s\n", time_buffer1);
          }
        else
          {
            fail_error=TRUE;
          }
        time_length=strftime(time_buffer2, 256, "NIST Server Time:  %A, %B %d,  %I:%M:%S %p %Y", &info2);
        if(time_length!=0&&!fail_error)
          {
            g_print("%s\n", time_buffer2);
            g_string_append_printf(string_builder, "%s\n", time_buffer2);
          }
        else
          {
            fail_error=TRUE;
          }
        if(!fail_error)
          {
            gtk_label_set_text(GTK_LABEL(data[0]), string_builder->str);
          }
        else 
          {
            g_print("Time String Buffer Error\n");
            gtk_label_set_text(GTK_LABEL(data[0]), "Time String Buffer Error");
          }
        g_string_free(string_builder, TRUE);     
      }
   
    if(connection!=NULL) g_object_unref(connection);
    if(client!=NULL) g_object_unref(client);   
  }
unsigned long int unpacku32_lendian(unsigned char *buffer)
  {
    g_print("Or %lu\n",((unsigned long int)buffer[0]<<24)|((unsigned long int)buffer[1]<<16)|((unsigned long int)buffer[2]<<8)|buffer[3]);
    g_print("Plus %lu\n",((unsigned long int)buffer[0]<<24)+((unsigned long int)buffer[1]<<16)+((unsigned long int)buffer[2]<<8)+buffer[3]);

    return ((unsigned long int)buffer[0]<<24)|((unsigned long int)buffer[1]<<16)|((unsigned long int)buffer[2]<<8)|buffer[3];
  }
int check_endianess()
  {
    int i=1;
    char *p =(char*)&i;

    if(p[0]==1) return LITTLE_ENDIAN1;
    else return BIG_ENDIAN1;
  }
static GdkPixbuf* draw_icon()
  {
    //Create a surface to draw a 256x256 icon. 
    cairo_surface_t *surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 256, 256);
    cairo_t *cr=cairo_create(surface);
    gint i=0;
    
    //Paint the background.
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.4);
    cairo_paint(cr);

    //A blue icon border.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 6);
    cairo_rectangle(cr, 0, 0, 256, 256);
    cairo_stroke(cr);

    cairo_pattern_t *pattern1=cairo_pattern_create_linear(0.0, 0.0,  350.0, 0.0);
    for(i=0;i<=200;i+=20)
      { 
        cairo_pattern_add_color_stop_rgb(pattern1, (double)(i/200.0), 1.0, 1.0, 0.0); 
        cairo_pattern_add_color_stop_rgb(pattern1, (double)(i+10.0)/200.0, 1.0, 0.0, 0.0); 
      }
 
    //The wave.    
    cairo_save(cr);
    cairo_set_source(cr, pattern1);
    cairo_set_line_width(cr, 3);
    cairo_move_to(cr, 0, 64);
    cairo_curve_to(cr, 20, 110, 180, 120, 190, 128);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 180, 134, 20, 150, 0, 196);
    cairo_stroke_preserve(cr);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    cairo_fill(cr);
    cairo_restore(cr);

    //The electron.
    cairo_save(cr);
    cairo_set_source_rgb(cr, 0.6, 0.6, 1.0);
    cairo_scale(cr, 1.0, 0.5);
    cairo_arc(cr, 220, 256, 25, 0, 2*G_PI);
    cairo_fill(cr);
    cairo_stroke(cr);
    cairo_restore(cr);

    //The electron line.
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 2);
    cairo_move_to(cr, 210, 128);
    cairo_line_to(cr, 230, 128);
    cairo_stroke(cr);

    GdkPixbuf *icon=gdk_pixbuf_get_from_surface(surface, 0, 0, 256, 256);

    cairo_pattern_destroy(pattern1);
    cairo_destroy(cr);
    cairo_surface_destroy(surface); 
    return icon;
  }
static void about_dialog(GtkWidget *widget, gpointer data)
  {
    GtkWidget *dialog=gtk_about_dialog_new();
    //Null will add the program icon to the logo.
    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), NULL);
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "NIST Atomic Time");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "Test Version 1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "Compare system time with NIST time.");
   
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }









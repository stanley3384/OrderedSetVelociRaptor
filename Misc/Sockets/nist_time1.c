
/*

    Test the NIST time servers with glib/GTK. Try the "TIME" protocol and NTP. Tested with
Ubuntu14.04 and GTK3.10.
    This one adds a GTK UI to glib_client1.c. 
   
    NIST: http://tf.nist.gov/tf-cgi/servers.cgi

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

int main(int argc, char *argv[])
  {   
    gtk_init(&argc, &argv);
  
    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "NIST Time");
    gtk_container_set_border_width(GTK_CONTAINER(window),10);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 300);
    g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit), NULL);

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
        time_t time1=time(NULL);
        time_t time2;
        if(ntp_time)
          {
            //Move time values to start of buffer for bit shifting. Only using time value in second here.
            buffer[0]=buffer[40];
            buffer[1]=buffer[41];
            buffer[2]=buffer[42];
            buffer[3]=buffer[43];
          } 
       
        if(check_endianess()==LITTLE_ENDIAN1)
          {
            time2=unpacku32_lendian((unsigned char*)buffer);
          }
        else
          {
            gchar time_temp[5];
            memset(time_temp, '\0', 5*sizeof(gchar));
            time_temp[0]=buffer[0];
            time_temp[1]=buffer[1];
            time_temp[2]=buffer[2];
            time_temp[3]=buffer[3];
            time2=g_ascii_strtoll(time_temp, NULL, 10);
          }
 
        //Adjust time to start at Jan 1, 1970.
        time2=time2-(2208988800ul);
        if(ntp_time)
          {
            g_print("Returned Buffer Length %i, Client Time %lu, NIST %s Time %lu\n", length, time1, protocol, time2);
            g_string_append_printf(string_builder, "Returned Buffer Length %i, Client Time %lu, NIST %s Time %lu\n", length, time1, protocol, time2);
          }
        else
          {
            g_print("Returned Buffer Length %i, Client Time %lu, NIST Time %lu\n", length, time1, time2);
            g_string_append_printf(string_builder, "Returned Buffer Length %i, Client Time %lu, NIST Time %lu\n", length, time1, time2);
          }

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
    return ((unsigned long int)buffer[0]<<24)|((unsigned long int)buffer[1]<<16)|((unsigned long int)buffer[2]<<8)|buffer[3];
  }
int check_endianess()
  {
    int i=1;
    char *p =(char*)&i;

    if(p[0]==1) return LITTLE_ENDIAN1;
    else return BIG_ENDIAN1;
  }







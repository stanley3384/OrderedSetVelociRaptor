
/*

    Test the NIST time servers with glib/GTK. Try the "TIME" protocol and NTP. Tested with
Ubuntu14.04 and GTK3.10. 

    gcc -Wall glib_client1.c -o glib_client1 `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<string.h>
#include<time.h>

#define LITTLE_ENDIAN1 0
#define BIG_ENDIAN1    1

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

int main(int argc, char *argv[])
  {
    GError *error=NULL;
    gchar *ntp_server="time.nist.gov";
    gchar buffer[48];
    gint fail_error=0;
    gboolean ntp_time=TRUE; //FALSE for time protocol on port 37.

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
    //Choose the time server protocol.
    if(ntp_time) connection=g_socket_client_connect_to_host(client, ntp_server, 123, NULL, &error);
    else connection=g_socket_client_connect_to_host(client, ntp_server, 37, NULL, &error);
    GSocket *sock=g_socket_connection_get_socket(connection);

    if(error!=NULL)
      {
        g_print("Connection Error: %s\n", error->message);
        fail_error=-1;
      }

    if(fail_error>=0)
      {
        g_socket_send(sock, buffer, 48, NULL, &error);
        if(error!=NULL)
          {
            g_print("Write Error: %s\n", error->message);
            fail_error=-1;
          }
      }

    if(fail_error>=0)
      {
        gsize length=g_socket_receive(sock, buffer, 48, NULL, &error);
        if(error!=NULL)
          {
            g_print("Read Error: %s\n", error->message);
            fail_error=-1;
          }   
        g_print("length=%i\n", (gint)length);
      }

   if(fail_error>=0)
      {
        time_t time1=time(NULL);
        time_t time2;
        if(ntp_time)
          {
            //Move time values to start of buffer. Only using time values here.
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
            memset(buffer, '\0', 48*sizeof(gchar));
            time_temp[0]=buffer[0];
            time_temp[1]=buffer[1];
            time_temp[2]=buffer[2];
            time_temp[3]=buffer[3];
            time2=g_ascii_strtoll(time_temp, NULL, 0);
          }
 
        time2=time2-(2208988800ul);
        g_print("Client Time %lu, Nist Time %lu\n", time1, time2);

        struct tm info1;
        struct tm info2;
        localtime_r(&time1, &info1);
        localtime_r(&time2, &info2);
        gchar time_buffer1[256];
        gchar time_buffer2[256];
        strftime(time_buffer1, 256, "Local Client Time: %A, %B %d,  %I:%M:%S %p %Y", &info1);
        g_print("%s\n", time_buffer1);
        strftime(time_buffer2, 256, "NIST Server Time:  %A, %B %d,  %I:%M:%S %p %Y", &info2);
        g_print("%s\n", time_buffer2);     
      }
   
    g_object_unref(connection);
    g_object_unref(client);
   
    return 0;
  }







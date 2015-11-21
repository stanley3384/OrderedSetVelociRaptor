
/*

    Test code for using a client server setup within a single process. Start the server on a different
thread and just send data to and from the server in the single process. Keep GTK function calls on main 
thread.
    Also get the NIST atomic time with a client function. Check for the four second request limit
specified by the NIST.

    Ubuntu14.04 with GTK3.10. 

    gcc -Wall inet1.c -o inet1 `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>   
#include<sys/socket.h> 
#include<arpa/inet.h>
#include<errno.h>
#include<netdb.h>
#include<time.h>

#define LITTLE_ENDIAN1 0
#define BIG_ENDIAN1    1

ssize_t server_socket;
ssize_t client_socket;
ssize_t new_socket;
struct sockaddr_in server;
struct sockaddr_in client;
struct sockaddr_in time_server;
GThread *thread=NULL;

static void close_program(GtkWidget *widget, gpointer data);
//The client for local message passing.
static void client_send_receive(GtkWidget *button, gpointer *data);
//Server functions on different thread for local message passing.
static void server_setup(gpointer data);
static void server_accept();
//NIST time.
static void check_four_seconds(GtkWidget *button, gpointer data);
static void nist_atomic_time(GtkWidget *button, gpointer data);
//http://www.ibm.com/developerworks/aix/library/au-endianc/
unsigned long int unpacku32_lendian(unsigned char *buffer);
static int check_endianess();

int main(int argc, char *argv[])
  {   
    gtk_init(&argc, &argv);
  
    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "NIST and Local Time");
    gtk_container_set_border_width(GTK_CONTAINER(window),10);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    g_signal_connect(window,"destroy",G_CALLBACK(close_program), NULL);

    GtkWidget *entry=gtk_entry_new();
    gtk_widget_set_hexpand(entry, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry), "Client Entry");

    GtkWidget *textview=gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_CHAR);
    gtk_widget_set_vexpand(textview, TRUE);

    gpointer widgets[]={entry, textview};

    GtkWidget *button1=gtk_button_new_with_label("Send Client Entry to Local Server");
    gtk_widget_set_hexpand(button1, TRUE);
    g_signal_connect(button1, "clicked", G_CALLBACK(client_send_receive), widgets);

    GtkWidget *button2=gtk_button_new_with_label("Get NIST Atomic Time");
    gtk_widget_set_hexpand(button2, TRUE);
    g_signal_connect(button2, "clicked", G_CALLBACK(check_four_seconds), textview);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), textview, 0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button2, 0, 4, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);

    //Start test server.
    thread=g_thread_new("TestThread", (GThreadFunc)server_setup, NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
  }
static void close_program(GtkWidget *widget, gpointer data)
  {
    close(server_socket);
    close(new_socket);
    g_thread_unref(thread);
    gtk_main_quit();
  }
//The client code.
static void client_send_receive(GtkWidget *button, gpointer *data)
  {
    int error = 0;
    char buffer[256];
    memset(buffer, '\0', 256*sizeof(char)); 
    const char *text = gtk_entry_get_text(GTK_ENTRY(data[0]));

    client_socket = socket(AF_INET , SOCK_STREAM , 0);
    g_print("Socket Client %i\n", client_socket);
    memset(&client, 0, sizeof(client)); 
    client.sin_addr.s_addr = inet_addr("127.0.0.1");
    client.sin_family = AF_INET;
    client.sin_port = htons(5500);

    error=connect(client_socket, (struct sockaddr *)&server, sizeof(server));
    if(error<0) fprintf(stderr, "connect: %s , errno %d\n", strerror(errno), errno);
    error=write(client_socket, text, strlen(text));
    if(error<0) fprintf(stderr, "Write: %s , errno %d\n", strerror(errno), errno);
  
    error=read(client_socket, buffer, 255);
    g_print("Buffer %s", buffer);
    if(error<0) fprintf(stderr, "Read: %s , errno %d\n", strerror(errno), errno);
  
    GtkTextBuffer* text_buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(data[1]));
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(text_buffer, &end);
    gtk_text_buffer_insert(text_buffer, &end, buffer, -1);

    close(client_socket);
  }
//The server code running on a different thread in the same process.
static void server_setup(gpointer data)
  {
    int error=0; 
    server_socket=socket(AF_INET , SOCK_STREAM , 0);
    g_print("Socket Server %i\n", server_socket);
    memset(&server, 0, sizeof(server)); 
    server.sin_addr.s_addr=inet_addr("127.0.0.1");
    server.sin_family=AF_INET;
    server.sin_port=htons(5500);
    error=bind(server_socket, (struct sockaddr *)&server, sizeof(server));
    if(error<0) fprintf(stderr, "bind: %s , errno %d\n", strerror(errno), errno);
    error=listen(server_socket, 5);
    if(error<0) fprintf(stderr, "listen: %s , errno %d\n", strerror(errno), errno);
    server_accept();
  }
static void server_accept()
  {
    char accept_buffer[256];
    int accept_error=0;
    socklen_t accept_length=0;

    while(1)
      {
        memset(accept_buffer, '\0', 256*sizeof(char));
        accept_length = sizeof(client);

        new_socket=accept(server_socket, (struct sockaddr *)&client, &accept_length);
        g_print("Read Server\n");
        accept_error=read(new_socket, accept_buffer, 255);
        if(accept_error<0) fprintf(stderr, "read: %s , errno %d\n", strerror(errno), errno);

        gchar *string=g_strdup_printf("Server response to: %s\n", accept_buffer);
        accept_error=write(new_socket, string, strlen(string));
        if(accept_error<0) fprintf(stderr, "Server Write: %s , errno %d\n", strerror(errno), errno);
        g_free(string);
      }
  }
static void check_four_seconds(GtkWidget *button, gpointer data)
  {
    static long seconds=-1;
    time_t time1=time(NULL);
    
    if(time1-seconds>4||seconds==-1) nist_atomic_time(button, data);
    else
      {
        g_print("Wait four seconds between time request. Time diff %ld\n", time1-seconds);
      }
    seconds=time1;
  }
static void nist_atomic_time(GtkWidget *widget, gpointer data)
  {
    g_print("Atomic Time\n");
    struct sockaddr_in nist_server;
    struct sockaddr_in response;
    unsigned int response_size;
    int client_socket;
    int port=37;
    char *hostname="time.nist.gov";
    struct hostent *hostent;
    //Store a 4 byte unsigned int for the time and a \0.
    char buffer[5];
    int length; 
    int fail_error=0;

    hostent = gethostbyname(hostname);
    if(hostent==NULL)
      {
        fprintf(stderr, "Hostname Error: %d\n", h_errno);
        fail_error=-1;
      }	
    printf("Host's IP: %s\n", inet_ntoa(nist_server.sin_addr));
	
    client_socket=socket(AF_INET, SOCK_DGRAM, 0);
    if(client_socket<0&&fail_error>=0)
      {
        fprintf(stderr, "Socket Error: %s\n", strerror(errno));
	fail_error=-1;
      }

    if(fail_error>=0)
      {
        memset(buffer, '\0', 5*sizeof(char));
        nist_server.sin_family=AF_INET;
        nist_server.sin_port=htons(port);
        nist_server.sin_addr.s_addr = *(unsigned long *)hostent->h_addr_list[0];
      }

    if(fail_error>=0)
      {  
        strcpy(buffer, "T");
        printf("Sending to Server... %s\n", buffer);     
        if(sendto(client_socket, buffer, strlen(buffer)+1, 0, (struct sockaddr *)&nist_server, sizeof(nist_server))<0)
          {	
            fprintf(stderr, "Send Error: %s\n", strerror(errno));
            fail_error=-1;
          } 
      }

    if(fail_error>=0)
      {
        struct timeval tv;
        //Set both sec and usec for receive timeout.
        tv.tv_sec=1;
        tv.tv_usec=500000;
        if(setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO , &tv, sizeof(tv))<0)
          {
            fprintf(stderr, "Timeout Error: %s\n", strerror(errno));
            fail_error=-1;
          }
      }

    if(fail_error>=0)
      {
        response_size=sizeof(response);
        printf("Receive From Server...\n");
        length=recvfrom(client_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&response, &response_size);	
        if(length<0)
          {
            fprintf(stderr, "Receive Error: %s\n", strerror(errno));
            fail_error=-1;
          }
      }
     
    if(fail_error>=0)
      {  
        char time_buffer1[256];
        char time_buffer2[256];
        time_t time1=time(NULL);
        time_t time2;
        //Check if computer is little endian. If it is, shift bits to change big endian int to little endian. 
        if(check_endianess()==LITTLE_ENDIAN1) time2=unpacku32_lendian((unsigned char*)buffer);
        else time2=atoi(buffer); 
        time2=time2-(2208988800ul);
        printf("Returned Buffer: Length %i, Client Time %lu, Nist Time %lu\n", length, time1, time2);

        GtkTextBuffer *text_buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(data));
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(text_buffer, &end); 
        gchar *string_time1=g_strdup_printf("Returned Buffer: Length %i, Client Time %lu, Nist Time %lu\n", length, time1, time2);
        gtk_text_buffer_insert(text_buffer, &end, string_time1, -1);
        g_free(string_time1);
        
        struct tm info1;
        struct tm info2;
        localtime_r(&time1, &info1);
        localtime_r(&time2, &info2);
   
        strftime(time_buffer1, 256, "Local Client Time: %A, %B %d,  %I:%M:%S %p %Y", &info1);
        printf("%s\n", time_buffer1);
        strftime(time_buffer2, 256, "NIST Server Time:  %A, %B %d,  %I:%M:%S %p %Y", &info2);
        printf("%s\n", time_buffer2);

        gtk_text_buffer_get_end_iter(text_buffer, &end); 
        gchar *string_time2=g_strdup_printf("%s\n%s\n", time_buffer1, time_buffer2);
        gtk_text_buffer_insert(text_buffer, &end, string_time2, -1);
        g_free(string_time2);
      }

    if(client_socket>-1) close(client_socket);
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




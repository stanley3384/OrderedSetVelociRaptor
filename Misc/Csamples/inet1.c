
/*

    Test code for using a client server setup within a single process. Start the server on a different
thread and just send data to and from the server in the single process.
    Ubuntu14.04 with GTK3.10. 

    gcc -Wall -std=c99 inet1.c -o inet1 `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>   
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <errno.h>

ssize_t server_socket;
ssize_t client_socket;
ssize_t new_socket;
struct sockaddr_in server;
struct sockaddr_in client;
GThread *thread=NULL;

static void close_program(GtkWidget *widget, gpointer data);
static void send_receive_from_server(GtkButton *button, gpointer *data);
//Test server functions on different thread.
static void server_setup(gpointer data);
static void server_accept();


int main(int argc, char *argv[])
  {   
    gtk_init(&argc, &argv);
  
    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width(GTK_CONTAINER(window),10);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    g_signal_connect(window,"destroy",G_CALLBACK(close_program), NULL);

    GtkWidget *entry=gtk_entry_new();
    gtk_widget_set_hexpand(entry, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry), "Client Entry");

    GtkWidget *textview=gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_CHAR);
    gtk_widget_set_vexpand(textview, TRUE);

    gpointer widgets[]={entry, textview};

    GtkWidget *button=gtk_button_new_with_label("Send to Server");
    gtk_widget_set_hexpand(button, TRUE);
    g_signal_connect(GTK_BUTTON(button), "clicked", G_CALLBACK(send_receive_from_server), widgets);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), textview, 0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 3, 1, 1);
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
static void send_receive_from_server(GtkButton *button, gpointer *data)
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
  
    GtkTextBuffer* text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data[1]));
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





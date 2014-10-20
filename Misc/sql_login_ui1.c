
/*
  Test code for a login UI. Needs to be hooked into the sql_login1.c code. 

  gcc -Wall sql_login_ui1.c -o sql_login_ui1 `pkg-config --cflags --libs gtk+-3.0`

  C. Eric Cashon
*/


#include<gtk/gtk.h>
#include<stdlib.h>

typedef struct
  {
    GtkWidget *entry1, *entry2, *combo1;
  } Widgets;

static void run_database_command(GtkWidget *widget, Widgets *w)
 {
   gchar *user_name=g_strdup(gtk_entry_get_text(GTK_ENTRY(w->entry1)));
   gchar *password=g_strdup(gtk_entry_get_text(GTK_ENTRY(w->entry2)));
   gint combo_id=atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(w->combo1)));
  
   g_print("%i %s %s\n", combo_id, user_name, password);
   switch(combo_id)
    {
      case 0:
        g_print("Create Administrator Login\n");
        break;
      case 1:
        g_print("Create User Login\n");
        break;
      case 2:
        g_print("Update Login\n");
        break;
      case 3:
        g_print("Delete Login\n");
        break;
      case 4:
        g_print("Login Exists\n");
        break;
      case 5:
        g_print("Initialize Passwords Table\n");
        break;
    }

   g_free(user_name);
   g_free(password);
 }
static void close_program()
 {
   printf("Quit Program\n");
   gtk_main_quit();
 }
int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Login Administrator");
   gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
   gtk_container_set_border_width(GTK_CONTAINER(window), 15);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   g_signal_connect_swapped(window, "destroy", G_CALLBACK(close_program), NULL);

   GtkWidget *label1=gtk_label_new("User Name");
   GtkWidget *label2=gtk_label_new("Password");  
         
   GtkWidget *entry1=gtk_entry_new();
   gtk_entry_set_width_chars(GTK_ENTRY(entry1), 20);

   GtkWidget *entry2=gtk_entry_new();
   gtk_entry_set_width_chars(GTK_ENTRY(entry2), 20);

   GtkWidget *combo1=gtk_combo_box_text_new();     
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "0", "Create Administrator Login");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "1", "Create User Login");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "2", "Update Login");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "3", "Delete Login");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "4", "Login Exists");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "5", "Initialize Passwords Table");
   gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 1);

   Widgets *w;
   w=g_slice_new(Widgets);
   w->entry1=GTK_WIDGET(entry1);
   w->entry2=GTK_WIDGET(entry2);
   w->combo1=GTK_WIDGET(combo1);
   GtkWidget *button1=gtk_button_new_with_label("Run");
   g_signal_connect(button1, "clicked", G_CALLBACK(run_database_command), (gpointer) w);  

   GtkWidget *label3=gtk_label_new("");
   gtk_widget_set_hexpand(label1, TRUE);
   gtk_widget_set_vexpand(label1, TRUE);

   GtkWidget *grid1=gtk_grid_new();
   gtk_grid_set_row_spacing(GTK_GRID(grid1), 10);
   gtk_grid_attach(GTK_GRID(grid1), label1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), label2, 0, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), entry1, 1, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), entry2, 1, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), combo1, 0, 2, 2, 1);
   gtk_grid_attach(GTK_GRID(grid1), button1, 0, 3, 2, 1);
   gtk_grid_attach(GTK_GRID(grid1), label3, 0, 4, 2, 2);
   gtk_container_add(GTK_CONTAINER(window), grid1);

   gtk_widget_show_all(window);

   gtk_main();

   return 0;
}


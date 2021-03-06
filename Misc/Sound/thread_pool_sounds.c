
/*
    Test code for playing sound files with alsa. Set up a thread pool to play a sound several
times without binding up the UI. Concerned the playsound function isn't reentrant but it seems
to work fine.
    This one looks for wav and ogg files in the local directory and loads them into a combobox. It tries
to set the background transparency of the the main window also.

    ALSA is thread safe and the following about sndfile.
    http://comments.gmane.org/gmane.comp.audio.libsndfile.devel/563

    Tested on Ubuntu16.04 with GTK3.18.

    gcc -Wall thread_pool_sounds.c -o thread_pool_sounds -lasound -lsndfile `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<sndfile.h>
#include<alsa/asoundlib.h>
#include<stdio.h>

static char *sound_file=NULL;
static GSList *pool_sounds=NULL;
static GMutex mutex;
static gint timer_id=0;

static gboolean check_list(gpointer data);
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data);
static void exit_program(GtkWidget *widget, gpointer data);
static void load_sounds(GtkWidget *combo, gpointer data);
static void play_sounds(GtkWidget *button, gpointer *data);
static int play_sound(char *sound_file);
static void spool_sound(snd_pcm_t *pcm_handle, SNDFILE *sndfile, short *buffer, snd_pcm_uframes_t frames, snd_pcm_sframes_t frames_written);
static void about_dialog(GtkWidget *widget, gpointer data);
static GdkPixbuf* draw_icon();
static void error_message(const gchar *string);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Thread Pool Sounds");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_widget_set_app_paintable(window, TRUE);
    //Set transparency of main window.
    if(gtk_widget_is_composited(window))
      {
        GdkScreen *screen = gtk_widget_get_screen(window);
        GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
        gtk_widget_set_visual(window, visual);
      }
    else g_print("Can't set window transparency.\n");
    g_signal_connect(window, "draw", G_CALLBACK(draw_background), NULL);
    GdkPixbuf *icon=draw_icon();
    gtk_window_set_default_icon(icon);

    g_mutex_init(&mutex);

    GError *pool_error=NULL;
    GThreadPool *sound_pool=g_thread_pool_new((GFunc)play_sound, sound_file, -1, FALSE, &pool_error);
    if(pool_error!=NULL)
      {
        g_print("Pool Error %s\n", pool_error->message);
        g_error_free(pool_error);
      }
    
    g_signal_connect(window, "destroy", G_CALLBACK(exit_program), sound_pool);

    GtkWidget *button1=gtk_button_new_with_label("Play Sound");
    gtk_widget_set_hexpand(button1, TRUE);
    GtkWidget *b1_label=gtk_bin_get_child(GTK_BIN(button1));
    gtk_widget_set_name(b1_label, "b1_label");
    gtk_label_set_markup(GTK_LABEL(b1_label), "<span foreground='yellow' size='x-large'>Play Sound</span>");

    GtkWidget *combo1=gtk_combo_box_text_new();
    gtk_widget_set_hexpand(combo1, TRUE);
    //Load the .wav files into the combobox to start with.
    load_sounds(combo1, NULL); 

    GtkWidget *combo2=gtk_combo_box_text_new();
    gtk_widget_set_hexpand(combo2, TRUE);
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 0, "1", ".wav");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 1, "2", ".ogg");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo2), "1");
    g_signal_connect_swapped(combo2, "changed", G_CALLBACK(load_sounds), combo1);

    GtkWidget *label1=gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(label1), "<span foreground='yellow' size='x-large'>Sound Pool</span>");
    gtk_widget_set_hexpand(label1, TRUE);

    GtkWidget *label2=gtk_label_new("");
    gtk_widget_set_hexpand(label2, TRUE);
    gtk_widget_set_vexpand(label2, TRUE);
    gtk_label_set_justify(GTK_LABEL(label2), GTK_JUSTIFY_LEFT);

    gpointer combo_sound[]={combo1, sound_pool, label2};
    g_signal_connect(button1, "clicked", G_CALLBACK(play_sounds), combo_sound); 

    GtkWidget *menu1=gtk_menu_new();
    GtkWidget *menu1item1=gtk_menu_item_new_with_label("Thread Pool Sounds");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu1), menu1item1);
    GtkWidget *title1=gtk_menu_item_new_with_label("About");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(title1), menu1);
    GtkWidget *menu_bar=gtk_menu_bar_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), title1);
    g_signal_connect(menu1item1, "activate", G_CALLBACK(about_dialog), window);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 20);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo1, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo2, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 4, 1, 3);
    gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 5, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);

    //Works on GTK3.18. The CSS needs to be changed for >=GTK3.20.
    GError *css_error=NULL;
    gchar css_string[]="GtkWidget{background-color: rgba(0,0,255,0.3); color: yellow} GtkButton{background-image: -gtk-gradient (linear, left bottom, right top, color-stop(0.0,rgba(0,255,0,0.9)), color-stop(0.5,rgba(180,180,180,0.9)), color-stop(1.0,rgba(25,0,200,0.9)));} GtkLabel#b1_label{background-image: -gtk-gradient (linear, left bottom, right top, color-stop(0.0,rgba(0,255,0,0.9)), color-stop(0.5,rgba(180,180,180,0.9)), color-stop(1.0,rgba(25,0,200,0.9)))} GtkDialog{background: rgba(0,0,255,1.0)}";
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_css_provider_load_from_data(provider, css_string, -1, &css_error);
    if(css_error!=NULL) g_print("CSS loader error %s\n", css_error->message);
    g_object_unref(provider);

    gtk_widget_show_all(window);

    timer_id=g_timeout_add(400, check_list, label2); 

    gtk_main();

    return 0;   
  }
static gboolean check_list(gpointer data)
  {
    GSList *iterator=NULL;
    GString *active_sounds=g_string_new(NULL);

    g_mutex_lock(&mutex);
    for(iterator=pool_sounds; iterator; iterator=iterator->next)
      {
        g_string_append(active_sounds, iterator->data);
      }
    g_mutex_unlock(&mutex);

    gtk_label_set_markup(GTK_LABEL(data), active_sounds->str);
    g_string_free(active_sounds, TRUE);
    return TRUE;
  }
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.5);
    cairo_paint(cr);
    return FALSE;
  }
static void exit_program(GtkWidget *widget, gpointer data)
  {
    g_thread_pool_free((GThreadPool*)data, TRUE, FALSE);
    g_slist_free_full(pool_sounds, g_free);
    if(sound_file!=NULL) g_free(sound_file);
    gtk_main_quit();
  }
static void load_sounds(GtkWidget *combo, gpointer data)
  {
    GError *dir_error=NULL;
    const gchar *file_temp=NULL;
    gchar *file_type=NULL;
    gint i=0;

    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(combo));

    if(data!=NULL) file_type=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(data));
    else file_type=g_strdup(".wav");

    GDir *directory=g_dir_open("./", 0, &dir_error);
    if(dir_error!=NULL)
      {
        g_print("dir Error %s\n", dir_error->message);
        g_error_free(dir_error);
        gtk_widget_set_sensitive(combo, FALSE);
      }
    else
      {
        file_temp=g_dir_read_name(directory);
        while(file_temp!=NULL)
          {
            if(file_temp!=NULL&&g_str_has_suffix(file_temp, file_type))
              {
                //g_print("%s\n", file_temp);
                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), file_temp);
                i++;
              }
            file_temp=g_dir_read_name(directory);
          }
        if(i>0)
          {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
            gtk_widget_set_sensitive(combo, TRUE);
          }
        else gtk_widget_set_sensitive(combo, FALSE);
        g_dir_close(directory);
      }

    if(file_type!=NULL) g_free(file_type);

  }
static void play_sounds(GtkWidget *button, gpointer *data)
  {
    GError *sound_error=NULL;
    GString *active_sounds=g_string_new(NULL);
    GSList *iterator=NULL;
 
    if(sound_file!=NULL) g_free(sound_file);

    if(gtk_widget_is_sensitive(GTK_WIDGET(data[0])))
      {
        sound_file=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(data[0]));
        g_mutex_lock(&mutex);
        pool_sounds=g_slist_append(pool_sounds, g_strdup_printf("%s\n", sound_file));
        for(iterator=pool_sounds; iterator; iterator=iterator->next)
          {
            g_string_append(active_sounds, iterator->data);
          }
        g_mutex_unlock(&mutex);
        gtk_label_set_markup(GTK_LABEL(data[2]), active_sounds->str);
        //Add sound to pool.
        g_thread_pool_push((GThreadPool*)data[1], sound_file, &sound_error);
        if(sound_error!=NULL)
          {
            g_print("Sound Error %s\n", sound_error->message);
            g_clear_error(&sound_error);
          } 
        g_string_free(active_sounds, TRUE);
      } 
    else error_message("There are no sound files in the local directory.");  
  }
static int play_sound(char *sound_file)
  {
    //Problem that the sound_file pointer is global. Is it a problem?
    gchar *sound=g_strdup_printf("%s\n", sound_file);
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames=0;
    snd_pcm_sframes_t frames_written=0;
    short *buffer=NULL;
    int dir=0;
    int error=0;
    int ret_val=0;

    SF_INFO sf_info;
    SF_FORMAT_INFO sf_format_info;
    SNDFILE *sndfile = NULL;

    //Set format to 0 in accordance to the documentation.
    sf_info.format=0;
    printf("Open %s\n", sound_file);
    sndfile=sf_open(sound_file, SFM_READ, &sf_info);

    //Check the sndfile.
    if(sndfile==NULL)
      {
        const char *error_string=sf_error_number(sf_error(sndfile));
        printf("Couldn't open sound file. libsndfile error: %s\n", error_string);
        ret_val=1;
        goto error_return;
      }
    else
      {
        printf("Channels: %i\n", sf_info.channels);
        printf("Sample Rate: %d\n", sf_info.samplerate);
        printf("Sections: %d\n", sf_info.sections);
        //Look up format meanings in sndfile library reference.
        printf("Format: %08x\n", sf_info.format);
        //Set for playback of float wav files.
        sf_command(sndfile, SFC_SET_SCALE_FLOAT_INT_READ, NULL, SF_TRUE);
 
        sf_format_info.format=sf_info.format;       
        sf_command(sndfile, SFC_GET_FORMAT_INFO, &sf_format_info, sizeof(sf_format_info));
        if(sf_format_info.extension==NULL)
          {
            printf("Format Info: Name %s\n", sf_format_info.name);
          }
        else
          {
            printf("Format Info: Name %s Extension %s\n", sf_format_info.name, sf_format_info.extension);
          }
      }

    //Check for alsa errors.
    if((error=snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0))<0)
      {
        printf("Cannot open audio device (%s)\n", snd_strerror(error));
	ret_val=1;
        goto error_return;
      }
    snd_pcm_hw_params_alloca(&params);
    if((error=snd_pcm_hw_params_any(pcm_handle, params))<0)
      {
        printf("Cannot initialize hardware parameter structure (%s)\n", snd_strerror(error));
	ret_val=1;
        goto error_return;
      }
    if((error=snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED))<0)
      {
        printf("Cannot set access type (%s)\n", snd_strerror(error));
	ret_val=1;
        goto error_return;
      }
    if((error=snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE))<0)
      {
        printf("Cannot set sample format (%s)\n", snd_strerror(error));
	ret_val=1;
        goto error_return;
      }
    if((error=snd_pcm_hw_params_set_channels(pcm_handle, params, sf_info.channels))<0)
      {
        printf("Cannot set channel count (%s)\n", snd_strerror(error));
	ret_val=1;
        goto error_return;
      }
    if((error=snd_pcm_hw_params_set_rate(pcm_handle, params, sf_info.samplerate, 0))<0)
      {
        printf("Cannot set sample rate (%s)\n", snd_strerror(error));
	ret_val=1;
        goto error_return;
      }
    if((error=snd_pcm_hw_params(pcm_handle, params))<0)
      {
        printf("Cannot set parameters (%s)\n", snd_strerror(error));
	ret_val=1;
        goto error_return;
      }
    if((error=snd_pcm_hw_params_get_period_size(params, &frames, &dir))<0)
      {
        printf("Cannot get period size %s\n", snd_strerror(error));
        ret_val=1;
        goto error_return;
      }

    printf("Play %s\n", sound_file);
    buffer=malloc(frames*sf_info.channels*sizeof(short));
    if(buffer==NULL)
      {
        printf("Couldn't allocate sound buffer.\n");
        ret_val=1;
        goto error_return;
      }
    else
      {
        spool_sound(pcm_handle, sndfile, buffer, frames, frames_written);
      }

    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    if(buffer!=NULL) free(buffer);
    sf_close(sndfile);
error_return:
    if(ret_val==1) printf("\n");
    printf("Sound Done\n");

    g_mutex_lock(&mutex);
    GSList *iterator=NULL;
    for(iterator=pool_sounds; iterator; iterator=iterator->next)
      {
        //g_print("Compare %s %s\n", (gchar*)iterator->data, sound);
        if(g_strcmp0((gchar*)iterator->data, sound)==0)
          {
            pool_sounds=g_slist_delete_link(pool_sounds, iterator);
          }
      }
    g_mutex_unlock(&mutex);

    if(sound!=NULL) g_free(sound);
    return ret_val;
  }
static void spool_sound(snd_pcm_t *pcm_handle, SNDFILE *sndfile, short *buffer, snd_pcm_uframes_t frames, snd_pcm_sframes_t frames_written)
  {
    int counter=0;
    printf("Frames %i FramesW %i\n", (int)frames, (int)frames_written);
    while((counter=sf_readf_short(sndfile, buffer, frames))>0)
      {
        frames_written=snd_pcm_writei(pcm_handle, buffer, counter);
        if(frames_written<0)
          {
            if(frames_written==-EBADFD)
              {
                printf("PCM is not in the right state.\n");
              }
            else if(frames_written==-EPIPE)
              {
                printf("Underrun occurred.\n");
                snd_pcm_prepare(pcm_handle);
              }
            else if(frames_written==-ESTRPIPE)
              {
                printf("A suspend event occurred.\n");
              }
            else
              {
                printf("Unknown frame write error.\n");
              }
         }
      }
        
  }
static void about_dialog(GtkWidget *widget, gpointer data)
  {
    GtkWidget *dialog=gtk_about_dialog_new();
    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), NULL);
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Thread Pool Sounds");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "Test Version 1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "Play multiple short sounds at once.");
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "(C) 2015 C. Eric Cashon");
   
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
//The diving and swimming pool notes.
static GdkPixbuf* draw_icon()
  {
    //Create a surface to draw a 256x256 icon. 
    cairo_surface_t *surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 256, 256);
    cairo_t *cr=cairo_create(surface);
    
    //Paint the background.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_paint(cr);

    //The diving note.
    cairo_save(cr);
    cairo_set_line_width(cr, 1);
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_translate(cr, 60, 60);
    cairo_rotate(cr, 3*G_PI/4);
    cairo_scale(cr, 1.0, 0.60);
    cairo_arc(cr, 0, 0, 30, 0, 2*G_PI);
    cairo_fill(cr);
    cairo_stroke_preserve(cr);
    cairo_restore(cr);

    //The diving note line.
    cairo_save(cr);
    cairo_set_line_width(cr, 6);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND); 
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_move_to(cr, 80, 40);
    cairo_line_to(cr, 170, 130);
    cairo_stroke(cr);
    cairo_restore(cr);

    //Two leg lines on diving note line.
    cairo_save(cr);
    cairo_set_line_width(cr, 6);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);  
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_move_to(cr, 170, 130);
    cairo_line_to(cr, 170, 90);
    cairo_stroke(cr); 
    cairo_move_to(cr, 150, 110);
    cairo_line_to(cr, 150, 70);
    cairo_stroke(cr);
    cairo_restore(cr);

    //The pool arc.
    cairo_save(cr);
    cairo_set_line_width(cr, 6);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.5);
    cairo_arc(cr, 128, 380, 240, 0, 2*G_PI);
    cairo_fill(cr);
    cairo_stroke_preserve(cr);
    cairo_restore(cr);

    //A note in the pool.
    cairo_save(cr);
    cairo_set_line_width(cr, 1);
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_translate(cr, 70, 230);
    cairo_rotate(cr, G_PI);
    cairo_scale(cr, 0.60, 1.0);
    cairo_arc(cr, 0, 0, 15, 0, G_PI);
    cairo_fill(cr);
    cairo_stroke_preserve(cr);
    cairo_restore(cr);

    //Note flag legs sticking up in pool.
    cairo_save(cr);
    cairo_set_line_width(cr, 2);
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_move_to(cr, 100, 180);
    cairo_line_to(cr, 105, 170);
    cairo_stroke(cr);
    cairo_move_to(cr, 105, 180);
    cairo_line_to(cr, 110, 170);
    cairo_stroke(cr);
    cairo_restore(cr);

    //Note flag in pool.
    cairo_save(cr);
    cairo_set_line_width(cr, 2);
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_move_to(cr, 50, 190);
    cairo_line_to(cr, 50, 170);
    cairo_stroke(cr);
    cairo_move_to(cr, 50, 170);
    cairo_line_to(cr, 43, 177);
    cairo_stroke(cr);
    cairo_move_to(cr, 50, 175);
    cairo_line_to(cr, 43, 182);
    cairo_stroke(cr);
    cairo_restore(cr);

    //A green border.
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_set_line_width(cr, 6);
    cairo_rectangle(cr, 0, 0, 256, 256);
    cairo_stroke(cr);

    //The diving board.
    cairo_save(cr);
    cairo_matrix_t matrix;
    cairo_matrix_init(&matrix, 1.0, 0.5, 0.0, 1.0, 0.0, 0.0);
    cairo_transform(cr, &matrix);
    cairo_rectangle(cr, 148, 100, 140, 50);
    cairo_stroke_preserve(cr);
    cairo_fill(cr);
    cairo_restore(cr);

    GdkPixbuf *icon=gdk_pixbuf_get_from_surface(surface, 0, 0, 256, 256);

    cairo_destroy(cr);
    cairo_surface_destroy(surface); 
    return icon;
  }
static void error_message(const gchar *string)
  {
    GtkWidget *dialog=gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "%s", string);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }



/*
    Test code for playing sound files with alsa. Set up a thread pool to play a sound several
times without binding up the UI. Concerned the playsound function isn't reentrant but it seems
to work. 

    Tested on Ubuntu14.03 with GTK3.10.

    gcc -Wall alsa_wave2.c -o alsa_wave2 -lasound -lsndfile `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<sndfile.h>
#include<alsa/asoundlib.h>
#include<stdio.h>


//Short test sounds. Sndfile won't recognize a mp3 so that one should produce an error.
static char *sounds[]={"piano2.wav", "0906.ogg", "test24b.wav", "cougar.mp3"}; 
static char *sound_file=NULL;

static void exit_program(GtkWidget *widget, gpointer data);
static void play_sounds(GtkWidget *button, gpointer data);
static int play_sound(char *sound_file);
static void spool_sound(snd_pcm_t *pcm_handle, SNDFILE *sndfile, short *buffer, snd_pcm_uframes_t frames, snd_pcm_sframes_t frames_written);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Thread Pool Sounds");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 250, 75);

    GError *pool_error=NULL;
    GThreadPool *sound_pool=g_thread_pool_new((GFunc)play_sound, sound_file, 4, TRUE, &pool_error);
    if(pool_error!=NULL)
      {
        g_print("Pool Error %s\n", pool_error->message);
        g_error_free(pool_error);
      }
    
    g_signal_connect(window, "destroy", G_CALLBACK(exit_program), sound_pool);

    GtkWidget *button1=gtk_button_new_with_label("Play Sound");
    gtk_widget_set_hexpand(button1, TRUE);
    gtk_widget_set_vexpand(button1, TRUE);
    g_signal_connect(button1, "clicked", G_CALLBACK(play_sounds), sound_pool);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 0, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;   
  }
static void exit_program(GtkWidget *widget, gpointer data)
  {
    g_thread_pool_free((GThreadPool*)data, FALSE, TRUE);
    gtk_main_quit();
  }
static void play_sounds(GtkWidget *button, gpointer data)
  {
    GError *sound_error=NULL;
    gint i=0;

    //Play one sound several times by clicking. Then try more than one sound.
    for(i=0;i<1;i++)
      {
        sound_file=sounds[i];
        //play_sound(sound_file);
        g_thread_pool_push((GThreadPool*)data, sound_file, &sound_error);
        if(sound_error!=NULL)
          {
            g_print("Sound Error %s\n", sound_error->message);
            g_clear_error(&sound_error);
          }   
      } 
  }
static int play_sound(char *sound_file)
  {
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
    return ret_val;
  }
static void spool_sound(snd_pcm_t *pcm_handle, SNDFILE *sndfile, short *buffer, snd_pcm_uframes_t frames, snd_pcm_sframes_t frames_written)
  {
    int counter=0;

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

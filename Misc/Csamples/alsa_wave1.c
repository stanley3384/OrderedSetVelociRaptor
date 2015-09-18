
/*
    Test code for playing a wave file with alsa. The sndfile library gets the arguments from
the wave file and passes them to the alsa functions to set up playback.

    gcc -Wall alsa_wave1.c -o alsa_wave1 -lasound -lsndfile

    C. Eric Cashon
*/

#include<sndfile.h>
#include<alsa/asoundlib.h>
#include<stdio.h>

//Test a 8 bit PCM wave file. 
static char *file_name="piano2.wav";
//Test a 8 bit PCM in an ogg container.
//static char *file_name="0906.ogg";

int main(int argc, char **argv)
  {
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames;
    snd_pcm_sframes_t frames_written;
    short *buffer=NULL;
    int dir=0;
    int counter=0;
    int error=0;

    SF_INFO sf_info;
    SF_FORMAT_INFO sf_format_info;
    SNDFILE *sndfile = NULL;

    sndfile=sf_open(file_name, SFM_READ, &sf_info);

    //Check the sndfile.
    if(sndfile==NULL)
      {
        const char *error_string=sf_error_number(sf_error(sndfile));
        printf("Couldn't open sound file. libsndfile error: %s\n", error_string);
        exit(1);
      }
    else
      {
        printf("Channels: %i\n", sf_info.channels);
        printf("Sample Rate: %d\n", sf_info.samplerate);
        printf("Sections: %d\n", sf_info.sections);
        sf_command(sndfile, SFC_GET_FORMAT_INFO, &sf_format_info, sizeof(sf_format_info));
        if(sf_format_info.extension==NULL)
          {
            printf("Format: %08x %s\n", sf_format_info.format, sf_format_info.name);
          }
        else
          {
            printf("Format: %08x %s %s\n", sf_format_info.format, sf_format_info.name, sf_format_info.extension);
          }
         //Set for wave files in float format.
         sf_command(sndfile, SFC_SET_SCALE_FLOAT_INT_READ, NULL, SF_TRUE);
      }

    //Check for alsa errors.
    if((error=snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0))<0)
      {
        printf("Cannot open audio device (%s)\n", snd_strerror(error));
	exit(1);
      }
    snd_pcm_hw_params_alloca(&params);
    if((error=snd_pcm_hw_params_any(pcm_handle, params))<0)
      {
        printf("Cannot initialize hardware parameter structure (%s)\n", snd_strerror(error));
	exit(1);
      }
    if((error=snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED))<0)
      {
        printf("Cannot set access type (%s)\n", snd_strerror(error));
	exit(1);
      }
    if((error=snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE))<0)
      {
        printf("Cannot set sample format (%s)\n", snd_strerror(error));
	exit(1);
      }
    if((error=snd_pcm_hw_params_set_channels(pcm_handle, params, sf_info.channels))<0)
      {
        printf("Cannot set channel count (%s)\n", snd_strerror(error));
	exit(1);
      }
    if((error=snd_pcm_hw_params_set_rate(pcm_handle, params, sf_info.samplerate, 0))<0)
      {
        printf("Cannot set sample rate (%s)\n", snd_strerror(error));
	exit(1);
      }
    if((error=snd_pcm_hw_params(pcm_handle, params))<0)
      {
        printf("Cannot set parameters (%s)\n", snd_strerror(error));
	exit(1);
      }
    if((error=snd_pcm_hw_params_get_period_size(params, &frames, &dir))<0)
      {
        printf("Cannot get period size %s\n", snd_strerror(error));
        exit(1);
      }

    printf("Play %s\n", file_name);
    buffer=malloc(frames*sf_info.channels*sizeof(short));
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

    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    free(buffer);
    sf_close(sndfile);
    return 0;
}

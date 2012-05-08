/*
 *  TI FM kernel driver's sample application.
 *
 *  Copyright (C) 2010 Texas Instruments
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <math.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#include "kfmapp.h"

static char *pdevice = "hw:0,0";                        /* playback device */
static char *cdevice = "hw:0,1";                        /* capture device */

snd_pcm_sw_params_t *swparams;
snd_output_t *output = NULL;
static snd_pcm_t *phandle,*chandle;
static int fm_aud_enable;

/* TODO: kfmapp.c and kfmapp.h needs code cleanup */
/* #define V4L2_TUNER_SUB_RDS 0x10 */

static char *g_mutemodes[]={"Mute On","Mute Off","Attenuate Voice"};
/*
static char *g_bands[]={"Europe/US","Japan"};
static char *g_sm_modes[]={"Stereo","Mono"};
static char *g_rx_deemphasis_modes[]={"50 usec","75 usec"};
static char *g_rds_opmodes[]={"RDS","RBDS"};
static char *g_af_switch_mode[]={"Off","On"};
*/
static char *g_rds_modes[]={"Off","On"};
static int g_vol_to_set;
static pthread_t g_rds_thread_ptr;
volatile char g_rds_thread_terminate,g_rds_thread_running;

static int g_radio_fd;

/* Program Type */
static char *pty_str[]= {"None", "News", "Current Affairs",
                         "Information","Sport", "Education",
                         "Drama", "Culture","Science",
                         "Varied Speech", "Pop Music",
                         "Rock Music","Easy Listening",
                         "Light Classic Music", "Serious Classics",
                         "other Music","Weather", "Finance",
                         "Childrens Progs","Social Affairs",
                         "Religion", "Phone In", "Travel",
                         "Leisure & Hobby","Jazz", "Country",
                         "National Music","Oldies","Folk",
                         "Documentary", "Alarm Test", "Alarm"};

void fmapp_display_tx_menu(void)
{
   printf("Available FM TX Commands:\n");
   printf("f <freq> tune to freq(in MHz)\n");
/*   printf("gf get frequency(MHz)\n");
   printf("e <val> set deemphasis filter value (1 - 2)\n");
   printf("ge get deemphasis filter\n");
   printf("p <val> set FM TX powerlevel (91 - 122)\n");
   printf("gp get deemphasis filter\n");
   printf("i <val> set FM TX antenna impedance value (0 = 50, 1 = 200 and 2 = 500)\n");
   printf("gi get FM TX antenna impedance value\n"); */
}
void fmapp_display_rx_menu(void)
{
   printf("Available FM RX Commands:\n");
/* printf("p power on/off\n"); */
   printf("f <freq> tune to freq(in MHz)\n");
   printf("gf get frequency(MHz)\n");
   printf("gr get rssi level\n");
   printf("t  turns RDS on/off\n");
   printf("gt get RDS on/off\n");
   printf("+ increases the volume\n");
   printf("- decreases the volume\n");
   printf("v <0-70> sets the volume\n");
   printf("gv get volume\n");
/* printf("b switches Japan / Eur-Us\n");
   printf("gb get band\n"); */
   printf("s switches stereo / mono\n");
   printf("gs get stereo/mono mode\n");
   printf("m changes mute mode\n");
   printf("gm get mute mode\n");
/* printf("e set deemphasis filter\n");
   printf("ge get deemphasis filter\n");
   printf("d set rf dependent mute\n");
   printf("gd get rf dependent mute\n");
   printf("z set rds system\n");
   printf("gz get rds system\n");
   printf("c set rds af switch\n");
   printf("gc get rds af switch\n"); */
   printf("< seek down\n");
   printf("> seek up\n");
/* printf("? <(-128)-(127)> set RSSI threshold\n");
   printf("g? get rssi threshold\n");*/
   printf("ga get tuner attributes\n");
/* printf("gn auto scan\n"); */
   printf("A Start FM RX Audio Routing\n");
   printf("q quit rx menu\n");
}
int fmapp_get_tx_ant_imp(void)
{
	struct v4l2_control vctrl;
	int res;

	vctrl.id = V4L2_CID_TUNE_ANTENNA_CAPACITOR;

	res = ioctl(g_radio_fd,VIDIOC_G_CTRL,&vctrl);
	if(res < 0)
	{
		printf("Failed to get FM Tx antenna impedence value\n");
		return res;
	}

	printf("FM Tx antenna impedence value is --> %d\n",vctrl.value);
	return 0;
}

int fmapp_get_tx_power_level(void)
{
	struct v4l2_control vctrl;
	int res;

	vctrl.id = V4L2_CID_TUNE_POWER_LEVEL;

	res = ioctl(g_radio_fd,VIDIOC_G_CTRL,&vctrl);
	if(res < 0)
	{
		printf("Failed to get FM Tx power level\n");
		return res;
	}

	printf("FM Tx Power level is --> %d\n",vctrl.value);
	return 0;
}
int fmapp_get_premphasis_filter_mode(void)
{
	struct v4l2_control vctrl;
	int res;

	vctrl.id = V4L2_CID_TUNE_PREEMPHASIS;

	res = ioctl(g_radio_fd,VIDIOC_G_CTRL,&vctrl);
	if(res < 0)
	{
		printf("Failed to get preemphasis filter val\n");
		return res;
	}

	printf("Preemphasis filter val is --> %d\n",vctrl.value);
	return 0;
}
int fmapp_get_tx_frequency(void)
{
	struct v4l2_frequency vf;
	struct v4l2_modulator vm;
	int res, div;

	vm.index = 0;
	res = ioctl(g_radio_fd, VIDIOC_G_MODULATOR, &vm);
	if(res < 0)
   {
       printf("Failed to get modulator capabilities\n");
       return res;
   }

   res = ioctl(g_radio_fd, VIDIOC_G_FREQUENCY,&vf);
   if(res < 0)
   {
     printf("Failed to read current frequency\n");
     return res;
   }

   div = (vm.capability & V4L2_TUNER_CAP_LOW) ? 1000 : 1;

   printf("Tuned to frequency %3.2f MHz \n",vf.frequency / ( 16.0 * div));
   return 0;
}
int fmapp_get_rx_frequency(void)
{
   struct v4l2_frequency vf;
   struct v4l2_tuner vt;
   int res, div;

   vt.index = 0;
   res = ioctl(g_radio_fd, VIDIOC_G_TUNER, &vt);
   if(res < 0)
   {
       printf("Failed to get tuner capabilities\n");
       return res;
   }

   res = ioctl(g_radio_fd, VIDIOC_G_FREQUENCY,&vf);
   if(res < 0)
   {
     printf("Failed to read current frequency\n");
     return res;
   }

   div = (vt.capability & V4L2_TUNER_CAP_LOW) ? 1000 : 1;

   printf("Tuned to frequency %3.2f MHz \n",vf.frequency / ( 16.0 * div));
   return 0;
}

int fmapp_set_tx_ant_imp(char *cmd)
{
	int user_val;
	struct v4l2_control vctrl;
	int res;

	sscanf(cmd, "%d", &user_val);

	vctrl.id = V4L2_CID_TUNE_ANTENNA_CAPACITOR;
	vctrl.value = user_val;
	res = ioctl(g_radio_fd,VIDIOC_S_CTRL,&vctrl);
	if(res < 0)
	{
		printf("Failed to set FM Tx antenna impedence value\n");
		return res;
	}

	printf("Setting FM Tx antenna impedence value to ---> %d\n",vctrl.value);
	return 0;
}

int fmapp_set_tx_power_level(char *cmd)
{
	int user_val;
	struct v4l2_control vctrl;
	int res;

	sscanf(cmd, "%d", &user_val);

	vctrl.id = V4L2_CID_TUNE_POWER_LEVEL;
	vctrl.value = user_val;
	res = ioctl(g_radio_fd,VIDIOC_S_CTRL,&vctrl);
	if(res < 0)
	{
		printf("Failed to set FM Tx power level\n");
		return res;
	}

	printf("Setting FM Tx Power level to ---> %d\n",vctrl.value);
	return 0;
}
int fmapp_set_premphasis_filter_mode(char *cmd)
{
	int user_val;
	struct v4l2_control vctrl;
	int res;

	sscanf(cmd, "%d", &user_val);

	vctrl.id = V4L2_CID_TUNE_PREEMPHASIS;
	vctrl.value = user_val;
	res = ioctl(g_radio_fd,VIDIOC_S_CTRL,&vctrl);
	if(res < 0)
	{
		printf("Failed to set preemphasis filter val\n");
		return res;
	}

	printf("Setting preemphasis filter val success\n");
	return 0;
}
int fmapp_set_tx_frequency(char *cmd)
{
   float user_freq;
   struct v4l2_frequency vf;
   struct v4l2_tuner vm;
   int res, div;

   sscanf(cmd, "%f", &user_freq);

   vm.index = 0;
   res = ioctl(g_radio_fd, VIDIOC_G_MODULATOR, &vm);
   if(res < 0)
   {
	   printf("Failed to get modulator capabilities\n");
	   return res;
   }

   vf.tuner = 0;
   vf.frequency = rint(user_freq * 16000 + 0.5);

   div = (vm.capability & V4L2_TUNER_CAP_LOW) ? 1000 : 1;
   if (div == 1)
	   vf.frequency /= 1000;

   res = ioctl(g_radio_fd, VIDIOC_S_FREQUENCY, &vf);
   if(res < 0)
   {
	   printf("Failed to set frequency %f\n",user_freq);
	   return res;
   }
   printf("Tuned to frequency %3.2f MHz\n", vf.frequency / (16.0 * div));

   return res;
}
int fmapp_set_rx_frequency(char *cmd)
{
   float user_freq;
   struct v4l2_frequency vf;
   struct v4l2_tuner vt;
   int res, div;

   sscanf(cmd, "%f", &user_freq);

   vf.tuner = 0;
   /* As per V4L2 specifications VIDIOC_S_FREQUENCY ioctl expects tuning
    * frequency in units of 62.5 KHz, or if the struct v4l2_tuner or struct
    * v4l2_modulator capabilities flag V4L2_TUNER_CAP_LOW is set, in units
    * of 62.5 Hz. But FM ST v4l2 driver presently handling the frequency in
    * units of 1 KHz
    */
   vf.frequency = rint(user_freq * 16000 + 0.5);

   vt.index = 0;
   res = ioctl(g_radio_fd, VIDIOC_G_TUNER, &vt);
   if(res < 0)
   {
       printf("Failed to get tuner capabilities\n");
       return res;
   }

   div = (vt.capability & V4L2_TUNER_CAP_LOW) ? 1000 : 1;
   if (div == 1)
	vf.frequency /= 1000;

   if(vf.frequency < vt.rangelow || vf.frequency > vt.rangehigh){
	printf("Failed to set frequency: Frequency is not in range"
		"(%3.2f MHz to %3.2f MHz)\n", (vt.rangelow/(16.0 * div)),
		(vt.rangehigh/(16.0 * div)));
	return -EINVAL;
   }

   res = ioctl(g_radio_fd, VIDIOC_S_FREQUENCY, &vf);
   if(res < 0)
   {
       printf("Failed to set frequency %f\n",user_freq);
       return res;
   }
   printf("Tuned to frequency %3.2f MHz\n", vf.frequency / (16.0 * div));
   return 0;
}

inline void display_volume_bar(void)
{
  int index;
  printf("\nVolume: ");
  for(index=0; index<g_vol_to_set; index++)
     printf("#");

  printf("\nVolume is : %d\n",g_vol_to_set);

}
int fmapp_set_rx_volume(char *cmd,int interactive,int vol_to_set)
{
   struct v4l2_control vctrl;
   int res;

   if(interactive == FMAPP_INTERACTIVE)
     sscanf(cmd, "%d", &g_vol_to_set);
   else
     g_vol_to_set = vol_to_set;

   vctrl.id = V4L2_CID_AUDIO_VOLUME;
   vctrl.value = g_vol_to_set;
   res = ioctl(g_radio_fd,VIDIOC_S_CTRL,&vctrl);
   if(res < 0)
   {
     g_vol_to_set = 0;
     printf("Failed to set volume\n");
     return res;
   }
   printf("Setting volume to %d \n",g_vol_to_set);
   return 0;
}

int fmapp_get_rx_volume(void)
{
   struct v4l2_control vctrl;
   int res;

   vctrl.id = V4L2_CID_AUDIO_VOLUME;
   res = ioctl(g_radio_fd,VIDIOC_G_CTRL,&vctrl);
   if(res < 0)
   {
     printf("Failed to get volume\n");
     return res;
   }
   g_vol_to_set = vctrl.value;
   display_volume_bar();
   return 0;
}

int fmapp_rx_increase_volume(void)
{
   int ret;

   g_vol_to_set +=1;
   if(g_vol_to_set > 70)
      g_vol_to_set = 70;

   ret = fmapp_set_rx_volume(NULL,FMAPP_BATCH,g_vol_to_set);
   if(ret < 0)
     return ret;

   display_volume_bar();
   return 0;
}
int fmapp_rx_decrease_volume(void)
{
   int ret;
   g_vol_to_set -=1;
   if(g_vol_to_set < 0)
      g_vol_to_set = 0;

   ret = fmapp_set_rx_volume(NULL,FMAPP_BATCH,g_vol_to_set);
   if(ret < 0)
    return ret;

   display_volume_bar();
   return 0;
}
int fmapp_set_rx_mute_mode(void)
{
   struct v4l2_control vctrl;
   static int mute_mode = FM_MUTE_OFF;
   int res;

   switch (mute_mode)
   {
     case FM_MUTE_OFF:
          mute_mode = FM_MUTE_ON;
          break;

     case FM_MUTE_ON:
          mute_mode = FM_MUTE_ATTENUATE;
          break;

     case FM_MUTE_ATTENUATE:
          mute_mode = FM_MUTE_OFF;
          break;
   }

   vctrl.id = V4L2_CID_AUDIO_MUTE;
   vctrl.value = mute_mode;
   res = ioctl(g_radio_fd,VIDIOC_S_CTRL,&vctrl);
   if(res < 0)
   {
     printf("Failed to set mute mode\n");
     return res;
   }

  printf("Setting to \"%s\" \n",g_mutemodes[mute_mode]);
  return 0;
}
int fmapp_get_rx_mute_mode(void)
{
   struct v4l2_control vctrl;
   int res;

   vctrl.id = V4L2_CID_AUDIO_MUTE;
   res = ioctl(g_radio_fd,VIDIOC_G_CTRL,&vctrl);
   if(res < 0)
   {
     printf("Failed to get mute mode\n");
     return res;
   }

   printf("%s\n",g_mutemodes[vctrl.value]);
   return 0;
}
int fmapp_rx_seek(int seek_direction)
{
   struct v4l2_hw_freq_seek frq_seek;
   int res;

   printf("Seeking %s..\n",seek_direction?"up":"down");
   frq_seek.seek_upward = seek_direction;
   errno = 0;
   res = ioctl(g_radio_fd,VIDIOC_S_HW_FREQ_SEEK,&frq_seek);
   if(errno == EAGAIN)
   {
     printf("Band limit reached\n");
   }
   else if(res <0)
   {
     printf("Seek operation failed\n");
     return res;
   }
   /* Display seeked freq */
   fmapp_get_rx_frequency();
   return 0;
}
int fmapp_start_audio()
{
	int err;

	snd_pcm_sw_params_alloca(&swparams);

	if (fm_aud_enable == 0){
		/* open alsa pcm device for playback */
		if ((err = snd_pcm_open(&phandle, pdevice,
				SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
			printf("Playback open error: %s\n", snd_strerror(err));
			exit(EXIT_FAILURE);
		}
		/* open alsa pcm device for capture */
		if ((err = snd_pcm_open(&chandle, cdevice,
				SND_PCM_STREAM_CAPTURE, 0)) < 0) {
			printf("Capture open error: %s\n", snd_strerror(err));
			exit(EXIT_FAILURE);
		}

		/* set hw params for alsa device
		 * this hw params indicate the format, number of bits, rate,
		 * number of channels, etc.
		 */
		if ((err = snd_pcm_set_params(phandle, SND_PCM_FORMAT_S32_LE,
				SND_PCM_ACCESS_RW_INTERLEAVED, 2, 48000, 0,
				15000000)) < 0) { /* 0.5sec */
			printf("Playback open error: %s\n", snd_strerror(err));
			exit(EXIT_FAILURE);
		}

		/* this calls prepare() in the driver - use this to start
		 * mcpdm downlink
		 */
		snd_pcm_sw_params_current(phandle, swparams);
		snd_pcm_sw_params_set_stop_threshold(phandle, swparams, -1);
		err = snd_pcm_sw_params(phandle, swparams);

		if ((err = snd_pcm_set_params(chandle, SND_PCM_FORMAT_S32_LE,
				SND_PCM_ACCESS_RW_INTERLEAVED, 2, 48000, 0,
				15000000)) < 0) { /* 0.5sec */
			printf("Playback open error: %s\n", snd_strerror(err));
			exit(EXIT_FAILURE);
		}
		snd_pcm_sw_params_current(chandle, swparams);
		snd_pcm_sw_params_set_stop_threshold(chandle, swparams, -1);
		err = snd_pcm_sw_params(chandle, swparams);

		/* this cals trigger() in the driver - use this to start
		 * mcpdm uplink
		 */
		snd_pcm_start(phandle);
		snd_pcm_start(chandle);

		fm_aud_enable = 1;
	}
	else {
		snd_pcm_drop(phandle);
		snd_pcm_drop(chandle);

		/* shutdown the pcm link */
		snd_pcm_close(phandle);
		snd_pcm_close(chandle);

		fm_aud_enable = 0;
	}

	printf("FM RX Audio Routing Done\n");
	return 0;
}
int fmapp_get_rx_rssi_lvl(void)
{
	struct v4l2_tuner vtun;
	float rssi_lvl;
	int res;

	vtun.index = 0;
	res = ioctl(g_radio_fd, VIDIOC_G_TUNER, &vtun);
	if(res < 0)
	{
		printf("Failed to get tunner attributes\n");
		return res;
	}
	rssi_lvl = ((float)vtun.signal / 0xFFFF) * 100;
	printf("Signal Strength: %d%%\n",(unsigned int)rssi_lvl);

	return 0;
}
int fmapp_set_stereo_mono_mode(void)
{
	struct v4l2_tuner vtun;
	int res = 0;

	vtun.index = 0;
	res = ioctl(g_radio_fd, VIDIOC_G_TUNER, &vtun);
	if(res < 0)
	{
		printf("Failed to set stereo-mono mode\n");
		return res;
	}

	if(V4L2_TUNER_MODE_STEREO == vtun.audmode)
		vtun.audmode = V4L2_TUNER_MODE_MONO;
	else
		vtun.audmode = V4L2_TUNER_MODE_STEREO;

	res = ioctl(g_radio_fd, VIDIOC_S_TUNER, &vtun);
	if(res < 0)
	{
		printf("Failed to set stereo-mono mode\n");
		return res;
	}
	printf("Audio Mode set to: %s\n",(vtun.audmode == V4L2_TUNER_MODE_STEREO) ? "STEREO":"MONO");

	return 0;
}
int fmapp_get_stereo_mono_mode(void)
{
	struct v4l2_tuner vtun;
	int res;

	vtun.index = 0;
	res = ioctl(g_radio_fd, VIDIOC_G_TUNER, &vtun);
	if(res < 0)
	{
		printf("Failed to get tunner attributes\n");
		return res;
	}
	printf("Audio Mode: %s\n",(vtun.audmode == V4L2_TUNER_MODE_STEREO) ? "STEREO":"MONO");

	return 0;
}
int fmapp_get_rx_tunner_attributes(void)
{
   struct v4l2_tuner vtun;
   float sigstrength_percentage;
   int res;

   vtun.index = 0;
   res = ioctl(g_radio_fd,VIDIOC_G_TUNER,&vtun);
   if(res < 0)
   {
     printf("Failed to get tunner attributes\n");
     return res;
   }
   printf("-----------------------\n");
   printf("Tuner Name: %s\n",vtun.name);
   /* TODO: FM driver is not setting V4L2_TUNER_CAP_LOW flag , but its returning vtun.rangelow
    * and vtun.rangehigh ranges in HZ . This needs to be corrected in FM driver */
   printf("  Low Freq: %d KHz\n",
           (unsigned int )((float)vtun.rangelow * 0.0625));
   printf(" High Freq: %d KHz\n",
           (unsigned int) ((float)vtun.rangehigh * 0.0625));
   printf("Audio Mode: %s\n",(vtun.audmode == V4L2_TUNER_MODE_STEREO) ? "STEREO":"MONO");
   sigstrength_percentage = ((float)vtun.signal /0xFFFF) * 100;
   printf("Signal Strength: %d%%\n",(unsigned int)sigstrength_percentage);
   printf("-----------------------\n");
   return 0;
}

int fmapp_get_scan_valid_frequencies(void)
{
    int ret;
    struct v4l2_tuner vtun;
    struct v4l2_frequency vf;
    struct v4l2_control vctrl;
    float freq_multiplicator,start_frq,end_frq,
          freq,perc,threshold,divide_by;
    long totsig;
    unsigned char index;

    vtun.index = 0;
    ret = ioctl(g_radio_fd, VIDIOC_G_TUNER, &vtun); /* get frequency range */
    if (ret < 0) {
	printf("Failed to get frequency range");
	return ret;
    }
    freq_multiplicator = (62.5 * ((vtun.capability & V4L2_TUNER_CAP_LOW)
                              ? 1 : 1000));

    divide_by = (vtun.capability & V4L2_TUNER_CAP_LOW) ? 1000000 : 1000;
    start_frq = ((float)vtun.rangelow * freq_multiplicator)/divide_by;
    end_frq = ((float)vtun.rangehigh * freq_multiplicator)/divide_by;

    threshold = FMAPP_ASCAN_SIGNAL_THRESHOLD_PER;

    /* Enable Mute */
    vctrl.id = V4L2_CID_AUDIO_MUTE;
    vctrl.value = FM_MUTE_ON;
    ret = ioctl(g_radio_fd,VIDIOC_S_CTRL,&vctrl);
    if(ret < 0)
    {
      printf("Failed to set mute mode\n");
      return ret;
   }
   printf("Auto Scanning..\n");
   for(freq=start_frq;freq<=end_frq;freq+=0.1)
   {
	vf.tuner = 0;
	vf.frequency = rint(freq*1000);
	ret = ioctl(g_radio_fd, VIDIOC_S_FREQUENCY, &vf);	/* tune */
        if (ret < 0) {
		printf("failed to set freq");
		return ret;
	}
	totsig = 0;
	for(index=0;index<FMAPP_ASCAN_NO_OF_SIGNAL_SAMPLE;index++)
	{
		vtun.index = 0;
		ret = ioctl(g_radio_fd, VIDIOC_G_TUNER, &vtun);	/* get info */
		if (ret < 0) {
			printf("Failed to get frequency range");
			return ret;
		}
		totsig += vtun.signal;
		perc = (totsig / (65535.0 * index));
		usleep(1);
        }
	perc = (totsig / (65535.0 * FMAPP_ASCAN_NO_OF_SIGNAL_SAMPLE));
	if ((perc*100.0) > threshold)
	   printf("%2.1f MHz(%d%%)\n",freq,((unsigned short)(perc * 100.0)));
   }
   /* Disable Mute */
   vctrl.id = V4L2_CID_AUDIO_MUTE;
   vctrl.value = FM_MUTE_OFF;
   ret = ioctl(g_radio_fd,VIDIOC_S_CTRL,&vctrl);
   if(ret < 0)
   {
      printf("Failed to set mute mode\n");
      return ret;
   }
   printf("Scan Completed\n");
   return 0;
}
int fmapp_get_rds_onoff(void)
{
	struct v4l2_tuner vtun;
	int res = 0;

	vtun.index = 0;
	res = ioctl(g_radio_fd, VIDIOC_G_TUNER, &vtun);
	if(res < 0)
	{
		printf("Failed to read RDS state\n");
		return res;
	}
	printf("RDS is: %s\n",(vtun.rxsubchans & V4L2_TUNER_SUB_RDS) ? "ON":"OFF");

	return 0;
}
void fmapp_rds_decode(int blkno, int byte1, int byte2)
{
    static char rds_psn[9];
    static char rds_txt[65];
    static int  rds_pty,ms_code;
    static int group,spare,blkc_byte1,blkc_byte2;

    switch (blkno) {
    case 0: /* Block A */
        printf("----------------------------------------\n");
        printf("block A - id=%d\n",(byte1 << 8) | byte2);
	break;
    case 1: /* Block B */
	printf("block B - group=%d%c tp=%d pty=%d spare=%d\n",
		    (byte1 >> 4) & 0x0f,
		    ((byte1 >> 3) & 0x01) + 'A',
		    (byte1 >> 2) & 0x01,
		    ((byte1 << 3) & 0x18) | ((byte2 >> 5) & 0x07),
		    byte2 & 0x1f);
	group = (byte1 >> 3) & 0x1f;
	spare = byte2 & 0x1f;
	rds_pty = ((byte1 << 3) & 0x18) | ((byte2 >> 5) & 0x07);
        ms_code = (byte2 >> 3)& 0x1;
	break;
    case 2: /* Block C */
        printf("block C - 0x%02x 0x%02x\n",byte1,byte2);
	blkc_byte1 = byte1;
	blkc_byte2 = byte2;
	break;
    case 3 : /* Block D */
	printf("block D - 0x%02x 0x%02x\n",byte1,byte2);
	switch (group) {
	case 0: /* Group 0A */
	    rds_psn[2*(spare & 0x03)+0] = byte1;
	    rds_psn[2*(spare & 0x03)+1] = byte2;
	    if ((spare & 0x03) == 0x03)
		    printf("PSN: %s, PTY: %s, MS: %s\n",rds_psn,
                            pty_str[rds_pty],ms_code?"Music":"Speech");
	    break;
	case 4: /* Group 2A */
	    rds_txt[4*(spare & 0x0f)+0] = blkc_byte1;
	    rds_txt[4*(spare & 0x0f)+1] = blkc_byte2;
	    rds_txt[4*(spare & 0x0f)+2] = byte1;
	    rds_txt[4*(spare & 0x0f)+3] = byte2;
            /* Display radio text once we get 16 characters */
//	    if ((spare & 0x0f) == 0x0f)
	    if (spare > 16)
            {
	        printf("Radio Text: %s\n",rds_txt);
//              memset(&rds_txt,0,sizeof(rds_txt));
            }
	    break;
         }
         printf("----------------------------------------\n");
         break;
     default:
         printf("unknown block [%d]\n",blkno);
    }
}
void *rds_thread(void *data)
{
  unsigned char buf[600];
  int radio_fd;
  int ret,index;
  radio_fd = (int)data;

  while(!g_rds_thread_terminate)
  {
    ret = read(radio_fd,buf,500);
    if(ret < 0)
       break;
    else if( ret > 0)
    {
       for(index=0;index<ret;index+=3)
         fmapp_rds_decode(buf[index+2] & 0x7,buf[index+1],buf[index]);
    }
  }
/* TODO: Need to conform thread termination.
 * below msg is not coming ,have a doubt on thread termination.
 * Fix this later.  */
  printf("RDS thread exiting..\n");
  return NULL;
}
int fmapp_set_rds_onoff(unsigned char fmapp_mode)
{
	struct v4l2_tuner vtun;
	int ret;
	static unsigned char rds_mode = FM_RDS_DISABLE;

	vtun.index = 0;
	ret = ioctl(g_radio_fd, VIDIOC_G_TUNER, &vtun);
	if(ret < 0)
	{
		printf("Failed to set rds on/off status\n");
		return ret;
	}
	if(rds_mode == FM_RDS_DISABLE) {
		vtun.rxsubchans |= V4L2_TUNER_SUB_RDS;
		rds_mode = FM_RDS_ENABLE;
	} else {
		vtun.rxsubchans &= ~V4L2_TUNER_SUB_RDS;
		rds_mode = FM_RDS_DISABLE;
	}

	ret = ioctl(g_radio_fd, VIDIOC_S_TUNER, &vtun);
	if(ret < 0)
	{
		printf("Failed to set rds on/off status\n");
		return ret;
	}
	/* Create rds receive thread once */
	if(fmapp_mode == FM_MODE_RX && rds_mode == FM_RDS_ENABLE &&
		g_rds_thread_running == 0)
	{
		g_rds_thread_running = 1;
		pthread_create(&g_rds_thread_ptr,NULL,rds_thread,(void *)g_radio_fd);
	}

	printf("RDS %s\n",g_rds_modes[rds_mode]);
	return 0;
}

void fmapp_execute_tx_get_command(char *cmd)
{
	switch(cmd[0])
	{
		case 'f':
			fmapp_get_tx_frequency();
			break;
		case 'e':
			fmapp_get_premphasis_filter_mode();
			break;
		case 'p':
			fmapp_get_tx_power_level();
			break;
		case 'i':
			fmapp_get_tx_ant_imp();
			break;
		default:
			printf("unknown command; type 'h' for help\n");
	}

}
void fmapp_execute_rx_get_command(char *cmd)
{
   switch(cmd[0])
   {
     case 'f':
          fmapp_get_rx_frequency();
          break;
     case 'r':
	  fmapp_get_rx_rssi_lvl();
          break;
     case 't':
          fmapp_get_rds_onoff();
          break;
     case 'v':
	  fmapp_get_rx_volume();
          break;
     case 'm':
          fmapp_get_rx_mute_mode();
          break;
#if 0
     case 'b':
          fmapp_get_band(fm_snd_ctrl);
          break;
     case 'd':
	  fmapp_get_rfmute(fm_snd_ctrl);
          break;
     case 'z':
          fmapp_get_rds_operation_mode(fm_snd_ctrl);
	  break;
     case 'c':
          fmapp_get_rx_af_switch(fm_snd_ctrl);
	  break;
     case '?':
	  fmapp_get_rx_rssi_threshold(fm_snd_ctrl);
	  break;
#endif
     case 's':
          fmapp_get_stereo_mono_mode();
          break;
#if 0
     case 'e':
          fmapp_get_rx_deemphasis_filter_mode(fm_snd_ctrl);
          break;
#endif
     case 'a':
	  fmapp_get_rx_tunner_attributes();
          break;
#if 0
     case 'n':
	  fmapp_get_scan_valid_frequencies();
	  break;
#endif
     default:
          printf("unknown command; type 'h' for help\n");
   }
}
void fmapp_execute_rx_other_command(char *cmd)
{
   switch(cmd[0])
   {
#if 0
     case 'p':
          fmapp_change_rx_power_mode(fm_snd_ctrl);
          break;
#endif
     case 'f':
          fmapp_set_rx_frequency(cmd+1);
          break;
     case 't':
          fmapp_set_rds_onoff(FM_MODE_RX);
          break;
     case '+':
	  fmapp_rx_increase_volume();
          break;
     case '-':
          fmapp_rx_decrease_volume();
          break;
     case 'v':
	  fmapp_set_rx_volume(cmd+1,FMAPP_INTERACTIVE,0);
	  break;
     case 'm':
          fmapp_set_rx_mute_mode();
          break;
     case '<':
          fmapp_rx_seek(FM_SEARCH_DIRECTION_DOWN);
	  break;
     case '>':
          fmapp_rx_seek(FM_SEARCH_DIRECTION_UP);
	  break;
#if 0
     case 'b':
          fmapp_set_band(fm_snd_ctrl);
          break;
#endif
     case 'h':
          fmapp_display_rx_menu();
          break;
#if 0
     case 'd':
	  fmapp_set_rfmute(fm_snd_ctrl);
          break;
     case 'z':
	  fmapp_set_rds_operation_mode(fm_snd_ctrl);
	  break;
     case 'c':
          fmapp_set_rx_af_switch(fm_snd_ctrl);
	  break;
     case '?':
	  fmapp_set_rx_rssi_threshold(fm_snd_ctrl,cmd+1);
	  break;
#endif
     case 's':
          fmapp_set_stereo_mono_mode();
          break;
#if 0
     case 'e':
          fmapp_set_rx_deemphasis_filter_mode(fm_snd_ctrl);
          break;
#endif
     case 'A':
	  fmapp_start_audio();
	  break;
  }
}

void fmapp_execute_tx_other_command(char *cmd)
{
	switch(cmd[0])
	{
		case 'f':
			fmapp_set_tx_frequency(cmd+1);
			break;
		case 'e':
			fmapp_set_premphasis_filter_mode(cmd+1);
			break;
		case 'p':
			fmapp_set_tx_power_level(cmd+1);
			break;
		case 'i':
			fmapp_set_tx_ant_imp(cmd+1);
			break;
		case 'h':
			fmapp_display_tx_menu();
			break;
	}
}
/* Switch to RX mode before accepting user commands for RX */
void fmapp_execute_rx_command(void)
{
	char cmd[100];
	struct v4l2_tuner vtun;
	int ret;

	vtun.index = 0;
	vtun.audmode = V4L2_TUNER_MODE_STEREO;
	vtun.rxsubchans = V4L2_TUNER_SUB_RDS;
	ret = ioctl(g_radio_fd, VIDIOC_S_TUNER, &vtun);
	if(ret < 0)
	{
		printf("Failed to set RX mode\n");
		return;
	}

	printf("Switched to RX menu\n");
	printf("type 'h' for help\n");

	while(1)
	{
		fgets(cmd, sizeof(cmd), stdin);
		switch(cmd[0]) {
		case 'g':
			fmapp_execute_rx_get_command(cmd+1);
			break;
		case 'q':
			printf("quiting RX menu\n");
			return;
		default:
			fmapp_execute_rx_other_command(cmd);
			break;
		}
	}
}
void fmapp_execute_tx_command(void)
{
	char cmd[100];
	struct v4l2_modulator vmod;
	int ret;

	vmod.index = 0;
	vmod.txsubchans = V4L2_TUNER_SUB_STEREO;

	ret = ioctl(g_radio_fd, VIDIOC_S_MODULATOR, &vmod);
	if(ret < 0)
	{
		printf("Failed to set TX mode\n");
		return;
	}

	printf("Switched to TX menu\n");
	printf("type 'h' for help\n");

	while(1)
	{
		fgets(cmd, sizeof(cmd), stdin);
		switch(cmd[0]) {
		case 'g':
			fmapp_execute_tx_get_command(cmd+1);
			break;
		case 'q':
			printf("quiting TX menu\n");
			return;
		default:
			fmapp_execute_tx_other_command(cmd);
			break;
		}
	}
}

int fmapp_read_anddisplay_capabilities(void)
{
  struct v4l2_capability cap;
  int res;

  res = ioctl(g_radio_fd,VIDIOC_QUERYCAP,&cap);
  if(res < 0)
  {
    printf("Failed to read %s capabilities\n",DEFAULT_RADIO_DEVICE);
    return res;
  }
  if((cap.capabilities & V4L2_CAP_RADIO) == 0)
  {
    printf("%s is not radio devcie",DEFAULT_RADIO_DEVICE);
    return -1;
  }
  printf("\n***%s Info ****\n",DEFAULT_RADIO_DEVICE);
  printf("Driver       : %s\n",cap.driver);
  printf("Card         : %s\n",cap.card);
  printf("Bus          : %s\n",cap.bus_info);
  printf("Capabilities : 0x%x\n",cap.capabilities);

  return 0;
}

static void sig_handler()
{
  if(g_rds_thread_running)
      g_rds_thread_terminate = 1;

  close(g_radio_fd);
  printf("Terminating..\n\n");
  exit(1);
}
int main()
{
   char choice[100];
   char exit_flag;
   int ret;
   struct sigaction sa;

   printf("** TI Kernel Space FM Driver Test Application **\n");

   printf("Opening device '%s'\n",DEFAULT_RADIO_DEVICE);
   g_radio_fd = open(DEFAULT_RADIO_DEVICE, O_RDWR);
   if(g_radio_fd < 0)
   {
       printf("Unable to open %s \nTerminating..\n",DEFAULT_RADIO_DEVICE);
       return 0;
   }
   ret = fmapp_read_anddisplay_capabilities();
   if(ret< 0)
   {
     close(g_radio_fd);
     return ret;
   }
   /* to handle ctrl + c and kill signals */
   memset(&sa, 0, sizeof(sa));
   sa.sa_handler = sig_handler;
   sigaction(SIGTERM, &sa, NULL);
   sigaction(SIGINT,  &sa, NULL);

   exit_flag = 1;
   while(exit_flag)
   {
       printf("1 FM RX\n");
       printf("2 FM TX\n");
       printf("3 Exit\n");
       fgets(choice, sizeof(choice), stdin);

       switch(atoi(choice))
       {
	       case 1: /* FM RX */
		       fmapp_execute_rx_command();
		       break;
	       case 2: /* FM TX */
		       fmapp_execute_tx_command();
		       break;
	       case 3:
		       printf("Terminating..\n\n");
		       exit_flag = 0;
		       break;
	       default:
		       printf("Invalid choice , try again\n");
		       continue;
       }
   }
   if(g_rds_thread_running)
	   g_rds_thread_terminate = 1; // Terminate RDS thread

   if (fm_aud_enable == 1){
      snd_pcm_drop(phandle);
      snd_pcm_drop(chandle);

      /* shutdown the pcm link */
      snd_pcm_close(phandle);
      snd_pcm_close(chandle);
    }

   close(g_radio_fd);
   return 0;
}



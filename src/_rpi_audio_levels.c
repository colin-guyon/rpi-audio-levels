/*
Uses the great GPU FFT lib made by Andrew Holme:
http://www.aholme.co.uk/GPU_FFT/Main.htm
*/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#include "mailbox.h"
#include "gpu_fft.h"

static struct GPU_FFT *fft;
static int fft_size;
static int mb = 0;

/* Initial method to call before being able to compute audio levels */
int prepare(int size) {
    int jobs = 1;
    int ret;

    fft_size = size;
    if (mb == 0) {
        mb = mbox_open();
    }
    ret = gpu_fft_prepare(mb, size, GPU_FFT_REV, jobs, &fft);
    switch(ret) {
        case -1: printf("Unable to enable V3D. Please check your firmware is up to date.\n"); return -1;
        case -2: printf("size=%d not supported.  Try between 8 and 22.\n", size);             return -1;
        case -3: printf("Out of memory.  Try a smaller batch or increase GPU memory.\n");     return -1;
        case -4: printf("Unable to map Videocore peripherals into ARM memory space.\n");      return -1;
        case -5: printf("Can't open libbcm_host.\n");                                         return -1;
    }
    return ret;
}


/* Get the audio levels for the given data and frequency-band indexes, using
   the GPU_FFT lib to compute the fft. This can be called as much as needed
   (after a preliminary call to the prepare function) */
float *compute(float *data, int bands_count, int **bands_indexes) {
    struct GPU_FFT_COMPLEX *base;
    int N = 1 << fft_size; // FFT length
    float *result;
    int i, j;
    float s;

    base = fft->in; // + j*fft->step; // input buffer

    for (i=0; i < N; i++) {
        base[i].re = data[i];
        base[i].im = 0; // we use real data
    }

    gpu_fft_execute(fft);

    base = fft->out; // + j*fft->step; // output buffer

    result = (float *) malloc(bands_count * sizeof(float));

    for (i=0; i < bands_count; i++) {
        s = 0.0;
        for (j=bands_indexes[i][0]; j < bands_indexes[i][1]; j++) {
            // abs(fft) ^ 2
            s += base[j].re * base[j].re + base[j].im * base[j].im;
        }
        // take the log10 of the resulting sum to approximate how human ears
        // perceive sound levels
        result[i] = log10(s);
    }
    return result;
}


/* Free ressources when computing is not needed anymore */
int release(void) {
    gpu_fft_release(fft); // Videocore memory lost if not freed !
    if (mb > 0) {
        mbox_close(mb);
    }
    return 0;
}

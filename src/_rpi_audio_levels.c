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
static int BANDS_COUNT;
static float* all_results;

// http://stackoverflow.com/questions/215557/most-elegant-way-to-implement-a-circular-list-fifo
#define QUEUE_ELEMENTS 50
#define QUEUE_SIZE (QUEUE_ELEMENTS + 1)
float *Queue[QUEUE_ELEMENTS];
int QueueIn;

void QueueInit(int bands_count)
{
    int i,j;
    for (i=0; i < QUEUE_ELEMENTS; i++) {
        Queue[i] = (float*) malloc(bands_count * sizeof(float));
        for(j=0; j < bands_count; j++) {
            Queue[i][j] = 12.0;
        }
    }
    QueueIn = 0;
}

void QueueRelease()
{
    int i,j;
    for (i=0; i < QUEUE_ELEMENTS; i++) {
        free(Queue[i]);
    }
}

int QueuePut(float* new)
{
    Queue[QueueIn] = new;
    QueueIn = (QueueIn + 1) % QUEUE_ELEMENTS;
    return 0;
}

float* QueuePop()
{
    return Queue[QueueIn];
}



/* Initial method to call before being able to compute audio levels */
int prepare(int size, int bands_count) {
    printf("prepare gpu fft\n");
    int ret;
    int jobs = 1;
    if (mb == 0) {
        mb = mbox_open();
    }
    fft_size = size;

    ret = gpu_fft_prepare(mb, size, GPU_FFT_REV, jobs, &fft);
    switch(ret) {
        case -1: printf("Unable to enable V3D. Please check your firmware is up to date.\n"); return -1;
        case -2: printf("size=%d not supported.  Try between 8 and 22.\n", size);             return -1;
        case -3: printf("Out of memory.  Try a smaller batch or increase GPU memory.\n");     return -1;
        case -4: printf("Unable to map Videocore peripherals into ARM memory space.\n");      return -1;
        case -5: printf("Can't open libbcm_host.\n");                                         return -1;
    }

    BANDS_COUNT = bands_count;

    QueueInit(bands_count);
    // buffer to store levels + current mean for each band + current stabndard deviation for each band
    all_results = (float*) malloc(3 * bands_count * sizeof(float));

    return ret;
}


/* Get the audio levels for the given data and frequency-band indexes, using
   the GPU_FFT lib to compute the fft. This can be called as much as needed
   (after a preliminary call to the prepare function) */
float* compute(float* data, int** bands_indexes) {
    struct GPU_FFT_COMPLEX *base;
    int N = 1 << fft_size; // FFT length
    float *result;
    int i, j;
    float s;
    float mean, std_deviation;

    base = fft->in; // + j*fft->step; // input buffer

    for (i=0; i < N; i++) {
        base[i].re = data[i];
        base[i].im = 0; // we use real data
    }

    gpu_fft_execute(fft);

    base = fft->out; // + j*fft->step; // output buffer

    //result = (float *) malloc(BANDS_COUNT * sizeof(float));
    // TODO: allocate all Queue at startup

    result = QueuePop();

    for (i=0; i < BANDS_COUNT; i++) {
        s = 0.0;
        for (j=bands_indexes[i][0]; j < bands_indexes[i][1]; j++) {
            // abs(fft) ^ 2
            s += base[j].re * base[j].re + base[j].im * base[j].im;
        }
        // take the log10 of the resulting sum to approximate how human ears
        // perceive sound levels
        result[i] = all_results[i] = log10(s);
    }

    QueuePut(result);

    //if (QueueIn == 1) {
        for(i=0; i < BANDS_COUNT; i++) {
            mean = 0.0;
            std_deviation = 0.0;

            // compute mean
            for(j=0; j < QUEUE_ELEMENTS; j++) {
                if (Queue[j][i] > 0) {
                    mean += Queue[j][i];
                }
            }
            mean = mean / QUEUE_ELEMENTS;

            // comupute standard deviation
            // numpy.std: std = sqrt(mean(abs(x - x.mean())**2))
            for(j=0; j < QUEUE_ELEMENTS; j++) {
                if (Queue[j][i] > 0) {
                    std_deviation += (Queue[j][i] - mean) * (Queue[j][i] - mean);
                }
            }
            std_deviation = sqrt(std_deviation / QUEUE_ELEMENTS);

            // store result
            all_results[BANDS_COUNT + i] = mean;
            all_results[(BANDS_COUNT << 1) + i] = std_deviation;
        }
    //}

    return all_results;
}


/* Free ressources when computing is not needed anymore */
int release(void) {
    printf("release gpu fft\n");
    gpu_fft_release(fft); // Videocore memory lost if not freed !
    if (mb > 0) {
        mbox_close(mb);
        mb = 0;
    }
    free(all_results);
    QueueRelease();
    return 0;
}


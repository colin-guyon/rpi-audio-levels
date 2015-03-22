#ifndef __RPI_AUDIO_LEVELS__
#define __RPI_AUDIO_LEVELS__

int prepare(int size); // 10 -> 2^10 fft samples

float *compute(float *data, int bands_count, int **bands_indexes);

int release(void);

#endif // __RPI_AUDIO_LEVELS__

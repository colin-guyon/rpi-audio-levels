
#cython: wraparound=False
#cython: boundscheck=False
#cython: cdivision=True

import numpy as np
cimport numpy as np
cimport cython
from libc.stdlib cimport malloc, free

FLOAT32_DTYPE = np.float32
ctypedef np.float32_t FLOAT32_DTYPE_t


cdef extern from "_rpi_audio_levels.h":
    int prepare(int size, int bands_count)
    float* compute(float* data, int** bands_indexes)
    int release()


cdef class AudioLevels:
    """ Allows to calculate audio levels by frequency, using the GPU FFT lib.

    a = AudioLevels(10, <bands_count>)
    ...
    frequency_bands_indexes = [[0,200], [200,400], ...]
    levels = a.compute(<float_numpy_data_1D>, frequency_bands_indexes)
    ...
    """

    def __cinit__(self, int fft_size, int bands_count):
        prepare(fft_size, bands_count)

    def __dealloc__(self):
        release()

    @cython.boundscheck(False)
    @cython.wraparound(False)
    def compute(self,
                np.ndarray[FLOAT32_DTYPE_t, ndim=1, mode='c'] data not None,
                list bands_indexes not None):
        cdef:
            unsigned int i
            int **c_bands_indexes
            unsigned int bands_count = len(bands_indexes)
            float[:] data_memview = data
            float *c_data = &data_memview[0] # c array corresponding to the numpy data
            float *result # the raw result from C code

            py_levels = [None] * bands_count # the result to return to python
            py_means = [None] * bands_count # the result to return to python
            py_stds = [None] * bands_count # the result to return to python

        # create a C array from python array (maybe there is a better way to do this ?)
        c_bands_indexes = <int**> malloc(bands_count * sizeof(int*))
        for i in range(bands_count):
            c_bands_indexes[i] = <int*> malloc(2 * sizeof(int))
            c_bands_indexes[i][0] = bands_indexes[i][0]
            c_bands_indexes[i][1] = bands_indexes[i][1]

        # execute the C code
        result = compute(c_data, c_bands_indexes)

        for i in range(bands_count):
            py_levels[i] = result[i]
            py_means[i] = result[bands_count + i]
            py_stds[i] = result[(bands_count << 1) + i]

        # free the float array returned by C since we copied its data
        #free(result)

        # free the C array that was previously allocated
        for i in range(bands_count):
            free(c_bands_indexes[i]);
        free(c_bands_indexes);

        return py_levels, py_means, py_stds

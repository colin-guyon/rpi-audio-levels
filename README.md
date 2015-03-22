# rpi-audio-levels

## Description

Python binding allowing to retrieve audio levels by frequency bands given audio samples, on
a raspberry pi.
It uses the GPU FFT lib (see http://www.aholme.co.uk/GPU_FFT/Main.htm).

I compared implementations using cython and ctypes, and the ctypes solution was slower
(overhead due to the input data transformation for the C call).

In my case it's 7 times faster than using pure python with Numpy.

## Dependencies

    The GPU FFT lib sources which can be found here http://www.aholme.co.uk/GPU_FFT/Main.htm.
    It is also directly in raspbian.
    You will also need Cython.

## Installation

    $ python setup.py build_ext --inplace

## Usage

    from rpi-audio-levels import AudioLevels
    DATA_SIZE = 12  # -> we will give chunks of 2^12 audio samples
    # Preliminary call to prepare things
    audio_levels = AudioLevels(DATA_SIZE)

    # Then retrieve audio levels each time you have new data
    levels = audio_levels.compute(data, band_indexes)
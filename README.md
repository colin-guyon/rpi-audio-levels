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

This generates the `rpi_audio_levels.so`, be sure to add its directory to the PYTHONPATH
(or install it using ``sudo python setup.py install`` instead of the above command).

## Usage

```python
from rpi_audio_levels import AudioLevels
DATA_SIZE = 11  # -> we will give chunks of 2^11 audio samples
# Preliminary call to prepare things
audio_levels = AudioLevels(DATA_SIZE)

# Then retrieve audio levels each time you have new data
bands_indexes = ((0,100), (100,200), (200,600), (600,700), (700,800), (800,1024)) # example of 6 frequency bands
levels = audio_levels.compute(data, bands_indexes)
# data must be an numpy array of real data with float dtype (np.float32), with only 1 dimmension.
```

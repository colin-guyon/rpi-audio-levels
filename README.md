# rpi-audio-levels

## Description

Python binding allowing to retrieve audio levels by frequency bands given audio samples, on
a raspberry pi (aka power spectrum).
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
DATA_SIZE = 11  # -> we will give chunks of 2**11 audio samples
BANDS_COUNT = 6  # -> we will give 6 ranges of band indexes for each computation
# Preliminary call to prepare things
audio_levels = AudioLevels(DATA_SIZE, BANDS_COUNT)

# example of 6 arbitrary frequency bands. Indexes must be between 0 and 2**(DATA_SIZE - 1)
bands_indexes = [[0,100], [100,200], [200,600], [600,700], [700,800], [800,1024]]

# Then retrieve audio levels each time you have new data
levels = audio_levels.compute(data, bands_indexes)
# data must be an numpy array of 2**DATA_SIZE real data with float dtype (np.float32), with only 1 dimmension.
```

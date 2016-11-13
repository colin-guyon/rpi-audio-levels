# rpi-audio-levels

## Description

Python binding allowing to retrieve audio levels by frequency bands given audio samples, on
a raspberry pi (aka power spectrum).
It uses the GPU FFT lib from Andrew Holme (see http://www.aholme.co.uk/GPU_FFT/Main.htm).

I compared implementations using cython and ctypes, and the ctypes solution was slower
(overhead due to the input data transformation for the C call).

In my case it's 7 times faster than using pure python and Numpy.

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
# We will give chunks of 2**11 audio samples
DATA_SIZE = 11
# We want to extract levels for 6 frequency bands
BANDS_COUNT = 6
# Preliminary call to prepare things
audio_levels = AudioLevels(DATA_SIZE, BANDS_COUNT)

# Example of 6 arbitrary frequency bands. Indexes must be between 0 and 2**(DATA_SIZE - 1).
bands_indexes = [[0,100], [100,200], [200,600], [600,700], [700,800], [800,1024]]
# (Here the indexes come from nowhere, but you could have some algorithm to convert
# start/end frequencies of each band you want to their corresponding indexes within
# the FFT data)

# Then retrieve audio levels each time you have new data.
levels, means, stds = audio_levels.compute(data, bands_indexes)
```

The returned value is a tuple of tuple:
  - levels: power spectrum levels for each frequency band
  - means: means for each band
  - stds: standard deviation for each band

``data`` must be a numpy array of 2**DATA_SIZE real data with float dtype (np.float32),
with only 1 dimension.
You may want to preliminary multiply your audio data chunks with a hanning window (for example)
before providing them to the ``compute()`` method.

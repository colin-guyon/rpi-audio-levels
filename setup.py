import os
from Cython.Distutils import build_ext
from distutils.core import setup
from distutils.extension import Extension

gpu_fft_src_path = '/opt/vc/src/hello_pi/hello_fft/'  # gpu fft is directly included in Raspbian
gpu_fft_source_files = ('gpu_fft_base.c', 'gpu_fft.c', 'gpu_fft_shaders.c',
                        'gpu_fft_twiddles.c', 'gpu_fft_trans.c', 'mailbox.c')

sources = ['src/rpi_audio_levels.pyx',
           'src/_rpi_audio_levels.c']

sources.extend([gpu_fft_src_path + p for p in gpu_fft_source_files])

README = """Python binding allowing to retrieve audio levels by frequency bands given 
audio samples (power spectrum in fact), on a Raspberry Pi, using GPU FFT"""

setup(
    name = "rpi_audio_levels",
    version = '0.1.1',
    url = 'https://github.com/colin-guyon/rpi-audio-levels',
    author = 'Colin Guyon',
    description = 'Python binding for Raspberry Pi GPU FFTerface',
    long_description = README,
    install_requires=[
          'cython>=0.19.1',
          'numpy',
      ],
    cmdclass={'build_ext': build_ext},
    ext_modules=[
        Extension('rpi_audio_levels',
                  sources=sources,
                  extra_compile_args=['-I' + gpu_fft_src_path],
                  extra_link_args=['-lrt', '-lm'])]
)


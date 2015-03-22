import os
from Cython.Distutils import build_ext
from distutils.core import setup
from distutils.extension import Extension

os.chdir('src')
gpu_fft_src_path = '/opt/vc/src/hello_pi/hello_fft/'  # gpu fft is directly included in Raspbian
gpu_fft_source_files = ('gpu_fft_base.c', 'gpu_fft.c', 'gpu_fft_shaders.c',
                        'gpu_fft_twiddles.c', 'gpu_fft_trans.c', 'mailbox.c')

sources = ['rpi_audio_levels.pyx',
           '_rpi_audio_levels.c']
sources.extend([gpu_fft_src_path + p for p in gpu_fft_source_files])

setup(
    cmdclass={'build_ext': build_ext},
    ext_modules=[
        Extension('rpi_audio_levels',
                  sources=sources,
                  extra_compile_args=['-I' + gpu_fft_src_path],
                  extra_link_args=['-lrt', '-lm'])]
)


from distutils.core import setup, Extension

c_ext = Extension("tgvoip", ["tgvoip.cpp",], libraries=['tgvoip',])

setup(
    name='pytgvoip',
    ext_modules=[c_ext],
)

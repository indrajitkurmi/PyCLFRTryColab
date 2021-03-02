from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize
from Cython.Distutils import build_ext
import numpy
#ext = Extension('glesLFR', sources=["glesLFRPyth.pyx","src\glesLFR.cpp","src\LFGenerator.cpp","src\stb_image.cpp","src\glad.c"], language="c++",)

#setup(name="glesLFR", ext_modules = cythonize([ext]),cmdclass = {'build_ext': build_ext})

# Building
setup(
    cmdclass = {'build_ext': build_ext},
    ext_modules = [
    Extension("glesLFR_Indrajit", 
              sources=["glesLFR_Indrajit_Pyth.pyx","glad.c","stb_image.cpp"],
              libraries=["glfw3","assimp",],
              language="c++",
              include_dirs=[numpy.get_include()]),
              extra_compile_args=["-Wall","-Wextra", "-std=c++17", "-ggdb", "-lpthread","-I../include/","-L../lib/","-llibassimp.so.5"],
              extra_link_args=["-L../lib/"]
              )
    ]
)          


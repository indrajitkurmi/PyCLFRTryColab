from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize
from Cython.Distutils import build_ext

#ext = Extension('glesLFR', sources=["glesLFRPyth.pyx","src\glesLFR.cpp","src\LFGenerator.cpp","src\stb_image.cpp","src\glad.c"], language="c++",)

#setup(name="glesLFR", ext_modules = cythonize([ext]),cmdclass = {'build_ext': build_ext})

# Building
setup(
    cmdclass = {'build_ext': build_ext},
    ext_modules = [
    Extension("glesLFR", 
              sources=["glesLFRPyth.pyx","glad.c","stb_image.cpp","LFGenerator.cpp"],
              libraries=["glfw3","assimp",],
              language="c++",
              extra_compile_args=["-Wall","-Wextra", "-std=c++17", "-ggdb", "-lpthread","-I/home/pi/PyBindTest/TestMainFunDecomp/glesLFR/include/","-L../lib/","-llibassimp.so.5"],
              extra_link_args=["-L../lib/"]
              )
    ]
)          

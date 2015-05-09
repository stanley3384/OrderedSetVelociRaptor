'''

   A setup file for testing Cython with the report_generator.py file

Reference:
   http://www.linuxquestions.org/questions/programming-9/cython-924418/

   1. $sudo apt-get install python-dev
   2. Save report_generator.py as report_generator.pyx
   For building as a shared library.
       1. $python setup.py build_ext --inplace
       2. Run report_generator.so with
             $python
             >>>import report_generator
   For building the program.
       1. $gcc $CFLAGS -I/usr/include/python2.7 -o report_generator report_generator.c -lpython2.7 -lpthread -lm -lutil -ldl
       2. $./report_generator

'''
from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

setup(
    cmdclass = {'build_ext': build_ext},
    ext_modules = [Extension("report_generator", ["report_generator.pyx"])]
)

Welcome to rtfilter's documentation!
=====================================

This package provides a library written in C implementing realtime digital
filtering functions for multichannel signals (i.e. filtering multiple signal
with the same filter). The core library implements FIR and IIR filtering for
float and double data type. Additional functions are also provided to design
few usual filters: Butterworth, Chebyshev, windowed sinc...

In addition, the filter functions have been especially optimized for
multichannel signals. If data allows it (and if possible with the compilation
flags used), the library automatically switch to optimized SIMD (Single
Instruction Multiple Data) code, allowing to reduce by 3~4 the time spent
in processing the data.

.. toctree::
   :caption: API module list
   :titlesonly:
   :maxdepth: 2

   rtfilter.rst


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`


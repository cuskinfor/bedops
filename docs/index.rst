BEDOPS
======
.. image:: https://dl.dropboxusercontent.com/u/31495717/bedops/logo.png
    :align: right

BEDOPS is a suite of tools to address common questions raised in genomic studies — mostly with regard to overlap and proximity relationships between data sets. It aims to be scalable and flexible, facilitating the efficient and accurate analysis and management of large-scale genomic data.

Features
----------
The suite includes tools for set and statistical operations (``bedops``, ``bedmap`` and ``closest-features``) and compression of large inputs into a novel lossless format (``starch``) that can provide greater space savings and faster data extractions than current alternatives. We offer native support for this compression format to these and other BEDOPS tools.

BEDOPS also offers logarithmic speedups in access to per-chromosome regions in sorted BED data (``bedextract``, core BEDOPS tools). These tools make whole-genome analyses "embarassingly parallel", in that per-chromosome computations can be placed onto separate work nodes, with results collated at the end in `map-reduce <http://en.wikipedia.org/wiki/MapReduce>`_ fashion.
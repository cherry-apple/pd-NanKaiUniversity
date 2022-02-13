# pd-NanKaiUniversity
A Fast Clustering Algorithm for Data Cleaning In Auditory Anomaly Detection

                                            ** Note on the Source Codes **

This directory contains the source codes associated with a paper submitted for the proceedings aistats2022. All the sources are written in C++. Three programs appear in the following directories:

Directory 'markInformativeSegments'. Contains the source code of the redundancy removal algorithm, whose core is the fast clustering algorithm.

Directory 'calculateErrors'. Contains the source code to calculate the auditory measure, including the calculation of PNR measure.

Directory 'elkanKmeansCluster'. Contains the source code of the Elkan-kmeans implementation.



The above C++ program depend on the following libraries.

Source code of our proposed faster clustering algorithm. It depends on the following C++ libraries.

libhdf5 and h5pp. These two libraries respectively offer C/C++ interface for manipulating HDF5 format. Links of these two libraries have been mentioned in the submitted paper.

matplotpp. This library offers a convinient interface for drawing figure from within C++ programs. Link of this library has been mentioned in the submitted paper.

g3log. This is a loggin library. The reader may find its introduct from https://github.com/KjellKod/g3log/blob/master/API.markdown.

options. Souce code of this library is in the directory 'options'. This library is used to parse program options. This library is a revised version from an existing project whose introduction is in http://code.google.com/p/getoptpp/wiki/Documentation.

eigenHelper. Souce code of this library is in the directory 'eigen'. This library contains some auxilary functions to interact with the Eigen library.

stlHelper. Souce code of this library is in the directory 'stl'. This library contains some auxilary functions to help to interact with the standard C++ library STL.



We haven't prepared scripts to buid the libraries and programs. If the reader encounter problems for building them, please contactd with the author.

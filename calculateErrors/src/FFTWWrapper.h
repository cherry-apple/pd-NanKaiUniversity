#pragma once
#include <vector>
#include <complex>
#include <mutex>
#include "fftw3.h"
#include "yamc/alternate_shared_mutex.hpp"

using std::vector;
using std::complex;

// data required to drive FFTW.
struct FFTWWorkplace{
    int N;   // logical length of the DFT.
    //               FFT                  IFFT
    // realSequence ----> complexSequence ----> recoveredRealSequence
    float* realSequence;
    fftwf_complex* complexSequence;
    float* recoveredRealSequence;
    fftwf_plan forwardPlan, backwardPlan;
};

using SharedMutex = yamc::alternate::basic_shared_mutex<yamc::rwlock::WriterPrefer>;

class FFTWWrapper{
public:
    // Allocate a FFTWWorkplace structure on the heap, set its various members.
    // This is the first function that a client should call.
    static FFTWWorkplace* createWorkplace(int logicalTransformLength);

    // When a client does not need to any more transformation, it should call this
    // function to free the underlying data structures.
    static void destroyWorkplace(FFTWWorkplace* workplace);

    // suppose length of the input is N, length of output is N/2+1.
    static void forwardTransform(FFTWWorkplace* workplace,
                                 const vector<float>& in, vector<complex<float>>& out);

    // suppose length of the output is N, length of input is N/2+1.
    static void backwardTransform(FFTWWorkplace* workplace,
                                  const vector<complex<float>>&in, vector<float>& out);

private:
    // On the face of it, the mutex below is used to protect concurrently accessing of the
    // previous vector, but essentially, it protects concurrently calling of FFTW's
    // non-thread-safe routines. Since the accessing of the vector follows a SWMR pattern,
    // we use yamc's shared_mutex.
    static SharedMutex fftwMutex;
};

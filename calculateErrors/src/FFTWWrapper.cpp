#include <shared_mutex>
#include "FFTWWrapper.h"

using namespace std;

SharedMutex FFTWWrapper::fftwMutex;

FFTWWorkplace* FFTWWrapper::createWorkplace(int logicalTransformLength)
{
    const int N = logicalTransformLength;

    // lock for writing.
    lock_guard<SharedMutex> lg(fftwMutex);
    FFTWWorkplace* workplace = new FFTWWorkplace;
    workplace->N = N;
    workplace->realSequence    = (float*) fftw_malloc(sizeof(float) * N);
    workplace->complexSequence = (fftwf_complex*) fftw_malloc(sizeof(fftwf_complex) * (N/2+1));
    workplace->recoveredRealSequence = (float*) fftw_malloc(sizeof(float) * N);
    workplace->forwardPlan = fftwf_plan_dft_r2c_1d(
                N, workplace->realSequence, workplace->complexSequence,
                FFTW_ESTIMATE);
    workplace->backwardPlan = fftwf_plan_dft_c2r_1d(
                N, workplace->complexSequence, workplace->recoveredRealSequence,
                FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);

    return workplace;
}

void FFTWWrapper::destroyWorkplace(FFTWWorkplace* workplace)
{
    fftwf_destroy_plan(workplace->forwardPlan);
    fftwf_destroy_plan(workplace->backwardPlan);
    fftwf_free(workplace->realSequence);
    fftwf_free(workplace->complexSequence);
    fftwf_free(workplace->recoveredRealSequence);

    delete workplace;
}

void FFTWWrapper::forwardTransform(FFTWWorkplace* workplace,
                                   const vector<float>& in, vector<complex<float>>& out)
{
    // locking for reading.
    shared_lock<SharedMutex> lg(fftwMutex);

    const int N = workplace->N;

    // copy input data.
    assert( in.size() == N );
    for (int i=0; i<N; i++){
        workplace->realSequence[i] = in[i];
    }

    // executing the transform.
    fftwf_execute( workplace->forwardPlan );

    // collect output data.
    out.resize( N/2+1 );
    for (int i=0; i<out.size(); i++){
        out[i].real( workplace->complexSequence[i][0] );
        out[i].imag( workplace->complexSequence[i][1] );
    }
}

void FFTWWrapper::backwardTransform(FFTWWorkplace* workplace,
                                    const vector<complex<float>>&in, vector<float>& out)
{
    // locking for reading.
    shared_lock<SharedMutex> lg(fftwMutex);

    const int N = workplace->N;

    // copy complex data.
    assert( in.size() == N/2 + 1 );
    for (int i=0; i<in.size(); i++){
        workplace->complexSequence[i][0] = in[i].real();
        workplace->complexSequence[i][1] = in[i].imag();
    }

    // executing the transform.
    fftwf_execute( workplace->backwardPlan );

    // normalize and collect output data.
    out.resize( N );
    for (int i=0; i<out.size(); i++){
        // Since FFTW doesn't apply the operation of dividing by N, we need to
        // do it by ourself.
        out[i] = workplace->recoveredRealSequence[i] / N;
    }
}

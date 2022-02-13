#include <cassert>
#include <vector>
#include <matplot/matplot.h>
#include "FFTWWrapper.h"

using std::vector;
using std::complex;

class PeaksNoiseRatio{
public:
    PeaksNoiseRatio();
    ~PeaksNoiseRatio();

    float calculatePNR(const vector<float>& spectrum);
    void  disableDrawing();

private:
    void calculateEnvelope(const vector<float>& spectrum, vector<float>& envelope);
    void findPeaks(const vector<float>& spectrum, const vector<float>& envelope,
                   vector<int>&   peakPositions);
    void abs(const vector<complex<float>>& in, vector<float>& out);

private:
    FFTWWorkplace* fftwWorkplace;

    float pnrRelativeTreshould;
    float pnrAbsoluteTreshould;
};

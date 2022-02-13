#include <numeric>
#include "options.h"
#include "ErrorMeasures.h"
#include "PeaksNoiseRatio.h"

using namespace std;
using namespace options;

ErrorMeasures::ErrorMeasures()
{
    Options& ops = OptionsInstance::get();
    pnrWeight = ops.getDouble("pnrWeight", 2.0);
}

// given a residual spectrum, calculate various error measures, including
// mean of the spectrum, hnr measure, and final error.
void ErrorMeasures::getMeasures(const vector<float>& residuals_,
                 float& mean, float&pnr, float&error)
{
    // the input residual is the sqaured error, we calculate its magnitude, so that
    // data range becomes smaller.
    vector<float> residuals = residuals_;
    for (int i=0; i<residuals.size(); i++){
        residuals[i] = sqrt(residuals[i]);
    }

    mean  = calculateMean(residuals);
    pnr   = calculatePNR(residuals);
    error = mean + pnr * pnrWeight;
}

float ErrorMeasures::calculateMean(const vector<float>& residuals)
{
    float sum = accumulate(residuals.begin(), residuals.end(), 0);
    return sum/residuals.size();
}

float ErrorMeasures::calculatePNR(const vector<float>& residuals)
{
    PeaksNoiseRatio PeaksNoiseRatio;
    float hnr = PeaksNoiseRatio.calculatePNR(residuals);
    return hnr;
}

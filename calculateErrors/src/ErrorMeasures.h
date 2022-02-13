#pragma once
#include <vector>

using std::vector;

class ErrorMeasures{
public:
    ErrorMeasures();

    // given a residual spectrum, calculate various error measures, including
    // mean of the spectrum, hnr measure, and final error.
    void getMeasures(const vector<float>& residuals,
                     float& mean, float&pnr, float&error);

private:
    float calculateMean(const vector<float>& residuals);
    float calculatePNR(const vector<float>& residuals);

private:
    // The total error is a weighted sum of mean magnitude and pnr of a residual spectrum.
    // The following faction is the weight of prn relative mean(whose mean is fixed to be
    // 1). The bigger the value is, the more important of a role the prn plays.
    float pnrWeight;
};

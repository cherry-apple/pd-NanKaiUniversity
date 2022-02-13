#include "options.h"
#include "fftw3.h"
#include "PeaksNoiseRatio.h"

using namespace std;
using namespace matplot;
using namespace options;

static void checkVector(const vector<float>& v, const string& vectorName)
{
    for (int k=0; k<v.size(); k++){
        if (std::isnan( v[k]) ){
            cout << "the " << k << "-th element of " << vectorName << "\n";
            exit(-1);
        }
    }
}

template<typename T>
void printVector(const vector<T>& v, const string& vectorName)
{
    cout << vectorName << ":\n";
    for (int i=0; i<v.size(); i++){
        cout << v[i] << " ";
    }
    cout << "\n";
}

PeaksNoiseRatio::PeaksNoiseRatio()
{    
    fftwWorkplace = FFTWWrapper::createWorkplace(128);

    Options& ops = OptionsInstance::get();
    pnrRelativeTreshould = ops.getDouble("pnrRelativeTreshould", 0.25);
    pnrAbsoluteTreshould = ops.getDouble("pnrRelativeTreshould", 2.5);
}

PeaksNoiseRatio::~PeaksNoiseRatio()
{
    FFTWWrapper::destroyWorkplace(fftwWorkplace);
}

void PeaksNoiseRatio::abs(const vector<complex<float>>& in, vector<float>& out)
{
    out.resize( in.size() );
    for (int i=0; i<out.size(); i++){
        out[i] = std::abs(in[i]);
    }
}


void PeaksNoiseRatio::calculateEnvelope(const vector<float>& spectrum,
                                            vector<float>& envelope)
{
    // calculate cepstrum
    vector< complex<float> > cepstrum;
    FFTWWrapper::forwardTransform(fftwWorkplace, spectrum, cepstrum);

    // set the higher-order coefficents to zeros.
    const int L = 10;
    for (int i=L; i<cepstrum.size(); i++){
        cepstrum[i] *= 0;
    }

    // calculate spectrum envelope
    envelope.resize( spectrum.size() );
    FFTWWrapper::backwardTransform(fftwWorkplace, cepstrum, envelope);

    // drawing
    options::Options& ops = options::OptionsInstance::get();
    if ( ops.presents("showEnvelopeCalculation")){
        // spectrum
        subplot(3,1,0);
        plot(spectrum);

        // cepstrum magnitude with higher-order coeeficents are set to zeros
        subplot(3,1,1);
        vector<float> cepstrumMagnitude;
        abs(cepstrum, cepstrumMagnitude);
        plot(cepstrumMagnitude);

        // spectrum envelope
        subplot(3,1,2);
        plot(envelope);
    }
}

void PeaksNoiseRatio::findPeaks(const vector<float>& spectrum,
                                    const vector<float>& envelope,
                                    vector<int>&   peakPositions)
{
    peakPositions.clear();
    assert( spectrum.size() == envelope.size() );

    // determine the threshould
    float maxValue = * max_element(spectrum.begin(), spectrum.end() );    
    float threshold = std::max( maxValue * pnrRelativeTreshould,  pnrAbsoluteTreshould);

    for (int i=0; i<spectrum.size(); i++){
        float reference = std::max<float>(envelope[i], 0.0f);
        if (spectrum[i] <= reference + threshold )
            continue;
        if (i==0){
            if (spectrum[i] > spectrum[i+1])
                peakPositions.push_back(i);
        }else if (i==spectrum.size()-1){
            if (spectrum[i] > spectrum[i-1])
                peakPositions.push_back(i);
        }else{
            if ( spectrum[i] >= spectrum[i-1] &&
                 spectrum[i] >= spectrum[i+1] )
                peakPositions.push_back(i);
        }
    }
}

float PeaksNoiseRatio::calculatePNR(const vector<float>& spectrum)
{    
    checkVector(spectrum, "input spectrum");

    vector<float> envelope;
    calculateEnvelope(spectrum, envelope);
    checkVector(envelope, "envelope");

    vector<int> peakPositions;
    findPeaks(spectrum, envelope, peakPositions);
    //cout << "number of peaks = " << peakPositions.size() << "\n";

    // calculating PNR
    float pnr=0;
    for (int pos: peakPositions){
        pnr += spectrum[pos] - envelope[pos];
    }
    if (peakPositions.size() > 0)
        pnr /= peakPositions.size();
    //cout << "hnr = " << hnr << "\n";

    if (isnan(pnr)){
        cout << "calculated HNR is NaN\n";
        printVector(spectrum, "spectrum");
        printVector(envelope, "envelope");
        printVector(peakPositions, "peakPositions");
        exit(-1);
    }

    //drawing
    options::Options& ops = options::OptionsInstance::get();
    if ( ops.presents("showHNRCalculation") && peakPositions.size()>0 ){
        gcf(true)->position(400, 200, 1200, 400);
        plot(spectrum, "k");
        hold(on);

        plot(envelope, "r");
        vector<float> X, Y;
        for (int pos: peakPositions){
            X.push_back( pos + 1);
            Y.push_back( spectrum[pos] );
        }
        stem(X, Y, "--bo");
        gcf(true)->show();

        gca()->clear();
    }

    return pnr;
}

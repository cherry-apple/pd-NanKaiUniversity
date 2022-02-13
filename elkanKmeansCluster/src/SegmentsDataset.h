#include <h5pp/h5pp.h>
#include "cluster/Dataset.h"

class SegmentsDataset: public Dataset{
public:
    SegmentsDataset(h5pp::File& h5File);

    int getSegmentsNumber();
    void getSegmentRange(int segmentID, int& startX, int& endX);

    Eigen::RowVectorXf operator()(int pointIndex) override;

private:
    h5pp::File& h5File;

    int segmentsNumber;
    int framesPerSegment;

    // caching.
    int cachedSegment;
    Eigen::MatrixXf cachedMatrix;
};

#include <iostream>
#include "Dataset.h"
#include "ElkanKmeansClusterer.h"

using namespace std;
using namespace Eigen;

class TestDataset: public Dataset{
public:
    TestDataset(){
        int N = 1000;
        m = MatrixXf::Random(N, 2);
        setSize(N);
    };

    Eigen::RowVectorXf operator()(int pointIndex) override{
        return m.row(pointIndex);
    }

private:
    MatrixXf m;
};

void clusterSythesizedData()
{
    TestDataset dataset;

    ElkanKmeansClusterer clusterer(dataset, 4 /*k*/);
    clusterer.cluster();    
}

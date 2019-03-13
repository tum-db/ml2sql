import time
import regression
import functions

#include "ml2sql/examples/include/loadTaxiData.ml"

create tensor DATA from taxiData(fare, tips) 

y = DATA[: , 1]
X = DATA[: , 0]

float0 function pol(X , i){
    return (X[0,0]^i)
}

float0 function lossFunction(X , y , w){
    res = X * w - y
    loss = sum(res.T * res)
    return loss/len(X)
}

start_cross = time()

crossvalidate{
    minfun: w = regression_reg(X,y,reg)
    lossfun: lossFunction(X,y,w)
    kernel: pol:X:4
    data: X,y
    folds: 10
    n: len(X)
    test{
        reg=[0.0;0.000005]:0.0000005
    }
}

end_cross = time()
print 'Cross time %' , (end_cross - start_cross)
print  '%' , w
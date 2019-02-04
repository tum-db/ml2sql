import time
import regression
import functions
import distributions

#include "include/loadTaxiData.ml"
#include "include/loadTestTaxiData.ml"


create tensor DATA from taxiData(trip_seconds, trip_miles, pickup_community_area, dropoff_community_area, fare, tips) 
create tensor TEST_DATA from taxiDataTest(trip_seconds, trip_miles, pickup_community_area, dropoff_community_area, fare , tips) 

X = DATA[: , 0:4]
y = DATA[: , 5]

X_test = TEST_DATA[: , 0:4]
y_test = TEST_DATA[: , 5:5]


start_bias = time()
X = addBiasTerm(X)
X_test = addBiasTerm(X_test)
end_bias = time()

// Set start value to zero
w[6,1] : 0

start_reg = time()
gradientdescent {
    function:'pow((X*w-y),2)'
    data: X , y
    optimize: w
    learningrate: 0.0000001
    threshold: 0.0000000000000000000000001
    maxsteps: 10000
    batchsize: 1000
}
end_reg = time()

start_pred = time()
err = predict(X_test , w) - y_test
avgLoss = sum(err.T * err)/len(X_test)
end_pred = time()

print '%' , avgLoss
print 'Time bias: %' , (end_bias -  start_bias)
print 'Time reg: %' , (end_reg -  start_reg)
print 'Time pred: %' , (end_pred -  start_pred)
print 'Time total: %' , ( (end_bias -  start_bias) +  (end_reg -  start_reg) + (end_pred -  start_pred) )


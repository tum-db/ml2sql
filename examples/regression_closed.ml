import time
import regression
import functions

#include "ml2sql/examples/include/loadTaxiData.ml"
#include "ml2sql/examples/include/loadTestTaxiData.ml"

create tensor DATA from taxiData(trip_seconds, trip_miles, pickup_community_area, dropoff_community_area, fare, tips)
create tensor TEST_DATA from taxiDataTest(trip_seconds, trip_miles, pickup_community_area, dropoff_community_area, fare, tips)

X = DATA[: , 0:4]
y = DATA[: , 5]

X_test = TEST_DATA[: , 0:4]
y_test = TEST_DATA[: , 5:5]

start_bias = time()
X = addBiasTerm(X)
X_test = addBiasTerm(X_test)
end_bias = time()

start_reg = time()
w = regression(X , y)
end_reg = time()

start_pred = time()
err = predict(X_test , w) - y_test
avgLoss = sum(err.T * err)/len(X_test)
end_pred = time()

print 'AvgLoss: %' , avgLoss
print 'Time bias: %' , (end_bias -  start_bias)
print 'Time reg: %' , (end_reg -  start_reg)
print 'Time pred: %' , (end_pred -  start_pred)
print 'Time total: %' , ( (end_bias -  start_bias) +  (end_reg -  start_reg) + (end_pred -  start_pred) )

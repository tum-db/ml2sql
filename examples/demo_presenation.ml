import functions
import time

#include "include/loadTaxiData.ml"

create tensor DATA from taxiData(trip_seconds,trip_miles,fare ,tips) 

X = DATA[: , 0:2]
y = DATA[: , 3]

startTime = time()


bias[1,len(X,0)] : 1
X = (bias::X.T).T

Xt = X.T
w = (Xt*X)^(-1) * Xt * y

err = X * w - y
avgLoss = sum(err.T * err)/len(X)

endTime = time()

print 'Loss: %' , avgLoss
print 'Time: %' , endTime - startTime
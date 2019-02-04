import time
import distributions

start_load = time()
#include "include/loadTaxiData.ml"
end_load = time()

start_create = time()
create tensor X from taxiData(trip_seconds, trip_miles, pickup_community_area, dropoff_community_area, fare, tips) 
end_create = time()

start_trans = time()
Xt = X.T
end_trans = time()

start_mul = time()
A = Xt * X
end_mul = time()

start_add = time()
A = X + X
end_add = time()

start_sub = time()
A = X - X
end_sub = time()



print 'Time load: %' , (end_load -  start_load)
print 'Time create: %' , (end_create -  start_create)
print 'Time trans: %' , (end_trans -  start_trans) 
print 'Time mul: %' , (end_mul -  start_mul)
print 'Time add: %' , (end_add -  start_add)
print 'Time sub: %' , (end_sub -  start_sub)
print 'Time total: %' , ( (end_load -  start_load) + (end_create -  start_create) + (end_trans -  start_trans) +  (end_mul -  start_mul) + (end_add -  start_add) + (end_sub -  start_sub) )

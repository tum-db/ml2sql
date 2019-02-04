import time
import distributions

start_sample = time()
A [1000,1000] ~ Uniform(0,10)
end_sample = time()

start_pow = time()
A = A^(-2)
end_pow = time()

print 'Time sample: %' , (end_sample -  start_sample)
print 'Time pow: %' , (end_pow -  start_pow)
print 'Time total: %' , ((end_sample -  start_sample) + (end_pow -  start_pow))

A = [[1.1,0.98,87.3,3],[0.1,3.15,42.05,3.3],
     [100.5,26.8,10.1,225.1],
     [1097.5,23000,10.1,24850.1]]
X = A[: , 0:2]
y = A[: , 3]
bias[1,len(X,0)] : 1
X = (bias::X.T).T
Xt = X.T
w = (Xt*X)^(-1) * Xt * y
print '%' , w

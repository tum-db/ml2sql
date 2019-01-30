

float2 function regression(X , y){
	Xt = X.T
	return (Xt*X)^(-1) * Xt * y
}

float2 function addBiasTerm(X){
	bias[1,len(X,0)] : 1
	return (bias::X.T).T
}

float2 function regression_reg(X , y, reg_strength){
	Xt = X.T
	I_st = id(len(Xt,0))*reg_strength
    to_Inverse = Xt * X + I_st
    return to_Inverse^(-1) * Xt * y
}

float2 function predict(X , w){
	return X * w
}



/** 
 *   This file is there you show how a language features can be used!
*/

//Include Librarys
import functions
import distributions


// Read CSV
readcsv{
    name:SomeData
    file:'some.csv'
    columns: fare,tips
    delimiter: ','
    delete empty entries
}

// Create tensors from the data
create tensor X from SomeData(fare)
create tensor y from SomeData(tips)

//Create a Tensor of dim (10 x 10) filled with zeros
zeros[10,10] : 0



// Creating a function that returns a float matrix
float2 function regression(X , y){
    // Transpose matrix X
	Xt = X.T
	//Take inverse and do matrix multiplications
	return (Xt*X)^^(-1) * Xt * y
}


//Define a extern function
float extern sum(1){
    postgres=sum
    hyper=sum
    python=np.sum
}



//Calculate sum of tips
tipsSum = 0.0
for i form 0 to len(y){
    tipsSum = tipsSum + y[0][i]
}


//Calculate sum of tips
tipsSum = 0.0
for i in [0;len(y)-1]:1{
    tipsSum = tipsSum + y[0][i]
}



//Create random float sample from normal distribution
s ~Normal(0,1)

//Create random float tensor sample from normal distribution
zeros[10,10] ~Normal(0,1)


//Finds optimal weights using regression with regularization
float2 function regression_reg(X , y, reg_strength){
	Xt = X.T
	I_st = id(len(Xt,2))*reg_strength
    to_Inverse = Xt * X + I_st
    return to_Inverse^^(-1) * Xt * y
}

//Define a loss function:
float0 function lossFunction(X , y , w){
    loss = 0.0
    res = X * w - y
    return sum(res)/len(X)
}

//Use cross Validation to find perfect regularization parameter!
crossvalidate {
    minfun: w = regression_reg(X,y,reg)
    lossfun: lossFunction(X,y,w)
    data: X,y
    folds: 6
    n: len(X)
    test{
        reg=[0.001;1]:0.05
    }
}
//Now the perfect weights are saved in w
// and the best regularization term is saved in reg




float extern exp(1){
   postgres=exp
   hyper=exp
   python=np.exp
}

float extern ln(1){
    postgres=ln
    hyper=ln
    python=np.log
}

float extern sqrt(1){
    postgres=sqrt
    hyper=sqrt
    python=np.sqrt
}


float extern rand(0){
    postgres=random
    hyper=random
    python=np.random.random_sample
}

//--------------------------------------------------
//Normal:

float function normal_pdf(mu , var , x ){
    l = 1/sqrt(2 * 3.14159265359 * var )
    r = exp( ((x-mu)^2)/(2*var)  )
    return l * r
}
float function normal_pdf( x ){
    return normal_pdf(0 , 1 , x )
}


float extern normal_sample(2){
    postgres=sampleNormal
    hyper=sampleNormal
    python=np.random.normal
}

float function normal_sample(){
    return sampleNormal(0 , 1)
}


//--------------------------------------------------
//Exponential


float function exp_pdf(lamda , x){
    if(x >= 0){
        return lamda * exp(-1* lamda * x)
    }else{
        return 0.0
    }
}

float function exp_sample(lamda){
    return -1* ln(1-rand())/lamda
}



//--------------------------------------------------
//Uniform


float function unifrom_pdf(a, b , x){
    //Why would someone call that?!
    return 1/(b-a)
}

float function unifrom_sample(a,b){
    return a + (b-a) * rand()
}
float function unifrom_sample(b){
    return (b) * rand()
}
float function unifrom_sample(){
    return rand()
}


//--------------------------------------------------


//Normal(mu,var)  gives normal distribution with expectation mu and variance var
//Normal() gives standard normal distribution

Normal = distribution(normal_sample ,  normal_pdf)


Exponential = distribution(exp_sample ,  exp_pdf)

Uniform = distribution(unifrom_sample ,  unifrom_pdf)

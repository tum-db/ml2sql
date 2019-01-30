
float extern len(2){
    postgres=array_length
    hyper=array_length
    python=np.size
}


//This is kind of hacky (but it works).
//We need this because pythons indexing starts at 0 and sql's indexing starts at 1
#ifdef PYTHON
    #define len1(x) len(x,0)
    #define len2(x,i) len(x,i)
#else
    #define len1(x) len(x,1)
    #define len2(x,i) len(x,i + 1)
#endif


//Do NOT try to understand this, you will probably fail!
//If you still want to understand this google it!
#define GET_MACRO(_1,_2,NAME,...) NAME
#define len(...) GET_MACRO(__VA_ARGS__, len2 , len1)(__VA_ARGS__)
float extern len(2){
    postgres=array_length
    hyper=array_length
    python=np.size
}

#ifdef PYTHON
    #define len1(x) len(x,0)
    #define len2(x,i) len(x,i)
#else
    #define len1(x) len(x,1)
    #define len2(x,i) len(x,i + 1)
#endif

#define GET_MACRO(_1,_2,NAME,...) NAME
#define len(...) GET_MACRO(__VA_ARGS__, len2 , len1)(__VA_ARGS__)

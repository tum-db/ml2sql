

#ifdef PYTHON
inject""
    import time
""
#endif


#ifdef HYPER

float function currentTimeSeconds(){
inject""
    -- EXTRACT( hour from  now() )
    -- does not work in hyper, it selects the day..
    -- so do not do timings a full hours
    select (EXTRACT( second from  now() ) + EXTRACT( minute from  now() ) * 60) as result;
    return result;
""
}

#endif

float extern time(0){
    postgres=currentTimeSeconds
    hyper=currentTimeSeconds
    python=time.time
}
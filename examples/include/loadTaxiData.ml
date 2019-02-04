
readcsv{
        name:taxiData
        file:'/tmp/data/small.csv'
        columns: trip_seconds,trip_miles,pickup_community_area,dropoff_community_area,fare,tips
        delimiter: ','
        delete empty entries
}

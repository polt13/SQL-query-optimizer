sum=0
for i in {1..5}  
do
    time_harness=`./runRelease.sh | cut -d" " -f4`
    ((sum+=time_harness))
done

echo -n "Average time: " 
echo  "scale=2; $sum/5" | bc
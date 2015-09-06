for vpm in 8 16 32 64
do
TV=1000
for (( np=1; np <= 100; np++ ))
do
echo "VPM = $vpm, TV = $TV, np = $np";
LogFile="result-$vpm-$np.log"
 ./waf --run "vehicle-demo --totalVehicles=$TV --verboseVehicles=true --verboseController=true --np=$np --l0=$vpm --l1=$vpm --l2=$vpm --l3=$vpm --l4=$vpm --l5=$vpm --l6=$vpm --l7=$vpm" > result/$LogFile  2>&1
done
done

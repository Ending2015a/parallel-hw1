#!/bin/bash


prog='./HW1_103062372_advanced'
p='-p batch'
declare -a Narr=('1' '1' '1' '1' '4'  '4'  '4'  '4'  '4')
declare -a narr=('1' '2' '4' '8' '16' '24' '32' '40' '48')
number=100000000
cs='testcase/case100M'
ans='testcase/case100M.ans'
out='out100M'
log='log100M.txt'

total=${#Narr[@]}
pass=0
all_pass=1

rm $log
rm $out

for ((i=0;i<$total;++i)); do
    N=${Narr[i]}
    n=${narr[i]}
    echo "for case$i: -N=$N -n=$n"

    echo "N $N, n $n" >> $log
    { time srun $p -N $N -n $n $prog $number $cs $out >> $log ; } 2>> $log

    echo "case done -> verifying..."


    if ./tools/check $ans $out ; then
        let pass="$pass + 1"
        echo -e "case$i: [ \033[1mpass\033[1m ]"
    else
        echo -e "case$i: [ \033[1mfailed\033[1m ]"
        let all_pass=0
    fi
done

echo "pass $pass/$total"
if [ "${all_pass}" == "1" ] ; then
    echo -e "\033[1mALL PASS\033[1m"
fi

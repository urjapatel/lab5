x=0
while [ $x -lt $1 ]
do
echo $2
x=`expr $x + 1`
done

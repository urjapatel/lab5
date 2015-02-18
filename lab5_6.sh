
if [ $1 ]
then
n=$1
else
n=50
fi

a=`expr $n + 1 `
b=`expr $n \* $a `
c=`expr $b / 2 `
echo $c



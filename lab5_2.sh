if [ $# -gt 0 ]
then
x=`echo $1 | wc -c`
x=`expr $x - 1`
final=""
while [ $x -gt 0 ]
do
c=`echo $1 | cut -c $x`
x=`expr $x - 1`
final=$final$c
done
fi
if [ $final = $1 ]
then
echo String is palindrome
else
echo The string is not palindrome
fi

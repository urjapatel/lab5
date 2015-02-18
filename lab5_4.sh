max=0
for i in `ls -l|tr -s " "|cut -f 5 -d " "`
do
if [ $max -le $i ]
then
max=$i
fi
done
echo maximum size is: $max

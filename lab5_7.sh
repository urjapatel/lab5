echo enter character
read c
for i in `ls -l|tr -s " "|cut -d " " -f 9`
do
 x=`echo $i|cut -c 1`
if [ $x = $c ]
then echo $i
fi
done

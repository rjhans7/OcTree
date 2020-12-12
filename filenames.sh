i=1
cd $1
for file in Paciente*
do
    echo $file $i.BMP
    i=$(($i+1))
done
#! /bin/sh
case $# in
2) echo Numero di parametri corretti: passato $1 e $2;;
*) echo Numero di parametri non corretto perche\' $#
   exit 1;;
esac

NTIMES=$1	#numero di volte che invocheremo lo specifico programma
PROG=$2		#programma specifico

count=0		#count per il ciclo

while test $count -ne $NTIMES
do
	$PROG 
	count=`expr $count + 1`
done

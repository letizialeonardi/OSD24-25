#! /bin/sh
#questo script e' uguale all'altro a parte che prevede anche un terzo parametro per poter invocare gli eseguibili che hanno biosgno del numero di thread
case $# in
3) echo Numero di parametri corretti: passato $1 e $2;;
*) echo Numero di parametri non corretto perche\' $#
   exit 1;;
esac

NTIMES=$1	#numero di volte che invocheremo lo specifico programma
PROG=$2		#programma specifico
PAR=$3		#numero da passare al programma

count=0		#count per il ciclo

while test $count -ne $NTIMES
do
	$PROG $PAR > log_$count &  
	count=`expr $count + 1`
done

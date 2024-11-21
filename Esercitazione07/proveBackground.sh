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
	$PROG > log_$count &  #N.B. mandando in esecuzione in background possiamo provare anche soluzionie potrebbero andare in deadlock senza che si debba usare il ^C e quindi abortire l'esecuzione del programma e quindi anche dello script
        #poiche' le soluzioni di questa esercitazione non vanno in deadlock si e' utilizzata la printf invece che la write su 1!	
	count=`expr $count + 1`
done

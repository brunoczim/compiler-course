#!/usr/bin/env sh

./make.sh

for FLAGS in "" "-fdedup-movs" "-finc-decs" "-fpower-of-two" "-freuse-tmps" "-O"
do
    echo "     $FLAGS"
    src/etapa7 src/sample-md5.txt $FLAGS
    strip a.out
    stat a.out
    echo
    size a.out
    echo
    time ./a.out < md5-input-mid.txt
    echo
done

for SIZE in small mid big
do
    echo "    $SIZE - unoptimized"
    src/etapa7 src/sample-md5.txt
    echo
    time ./a.out < md5-input-$SIZE.txt
    echo
    echo "    $SIZE - -O"
    src/etapa7 src/sample-md5.txt -O
    echo
    time ./a.out < md5-input-$SIZE.txt
    echo
done

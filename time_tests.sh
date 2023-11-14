#!/bin/bash

n=3
sudo gcc -fopenmp main.c options.h options.c -o main
saida="saida.txt"

#30k
#================================================================================================================
sample="./main samples/30k1.fna samples/30k2.fna --match 2 --mismatch -3 --gap -5 --gap_seq -2 --blocks 1"
for ((i = 1; i <= n; i++)); do
    echo "Threads: 1 Tamanho: 30k Execução $i/3"
    echo "Threads: 1 Tamanho: 30k Execução $i/3" >> $saida
    echo $sample >> $saida && $sample >> $saida 2>&1
    echo "" >> $saida
done


sample="./main samples/30k1.fna samples/30k2.fna --match 2 --mismatch -3 --gap -5 --gap_seq -2 --blocks 20"
for ((i = 1; i <= n; i++)); do
    echo "Threads: 20 Tamanho: 30k Execução $i/3"
    echo "Threads: 20 Tamanho: 30k Execução $i/3" >> $saida
    echo $sample >> $saida && $sample >> $saida 2>&1
    echo "" >> $saida
done


sample="./main samples/30k1.fna samples/30k2.fna --match 2 --mismatch -3 --gap -5 --gap_seq -2 --blocks 40"
for ((i = 1; i <= n; i++)); do
    echo "Threads: 40 Tamanho: 30k Execução $i/3"
    echo "Threads: 40 Tamanho: 30k Execução $i/3" >> $saida
    echo $sample >> $saida && $sample >> $saida 2>&1
    echo "" >> $saida
done

#80k
#================================================================================================================
sample="./main samples/80k1.fna samples/80k2.fna --match 2 --mismatch -3 --gap -5 --gap_seq -2 --blocks 1"
for ((i = 1; i <= n; i++)); do
    echo "Threads: 1 Tamanho: 80k Execução $i/3"
    echo "Threads: 1 Tamanho: 80k Execução $i/3" >> $saida
    echo $sample >> $saida && $sample >> $saida 2>&1
    echo "" >> $saida
done


sample="./main samples/80k1.fna samples/80k2.fna --match 2 --mismatch -3 --gap -5 --gap_seq -2 --blocks 20"
for ((i = 1; i <= n; i++)); do
    echo "Threads: 20 Tamanho: 80k Execução $i/3"
    echo "Threads: 20 Tamanho: 80k Execução $i/3" >> $saida
    echo $sample >> $saida && $sample >> $saida 2>&1
    echo "" >> $saida
done


sample="./main samples/80k1.fna samples/80k2.fna --match 2 --mismatch -3 --gap -5 --gap_seq -2 --blocks 40"
for ((i = 1; i <= n; i++)); do
    echo "Threads: 40 Tamanho: 80k Execução $i/3"
    echo "Threads: 40 Tamanho: 80k Execução $i/3" >> $saida
    echo $sample >> $saida && $sample >> $saida 2>&1
    echo "" >> $saida
done

#100k
#================================================================================================================
sample="./main samples/100k1.fna samples/100k2.fna --match 2 --mismatch -3 --gap -5 --gap_seq -2 --blocks 1"
for ((i = 1; i <= n; i++)); do
    echo "Threads: 1 Tamanho: 100k Execução $i/3"
    echo "Threads: 1 Tamanho: 100k Execução $i/3" >> $saida
    echo $sample >> $saida && $sample >> $saida 2>&1
    echo "" >> $saida
done


sample="./main samples/100k1.fna samples/100k2.fna --match 2 --mismatch -3 --gap -5 --gap_seq -2 --blocks 20"
for ((i = 1; i <= n; i++)); do
    echo "Threads: 20 Tamanho: 100k Execução $i/3"
    echo "Threads: 20 Tamanho: 100k Execução $i/3" >> $saida
    echo $sample >> $saida && $sample >> $saida 2>&1
    echo "" >> $saida
done


sample="./main samples/100k1.fna samples/100k2.fna --match 2 --mismatch -3 --gap -5 --gap_seq -2 --blocks 40"
for ((i = 1; i <= n; i++)); do
    echo "Threads: 40 Tamanho: 100k Execução $i/3"
    echo "Threads: 40 Tamanho: 100k Execução $i/3" >> $saida
    echo $sample >> $saida && $sample >> $saida 2>&1
    echo "" >> $saida
done

#150k
#================================================================================================================
sample="./main samples/150k1.fna samples/150k2.fna --match 2 --mismatch -3 --gap -5 --gap_seq -2 --blocks 1"
for ((i = 1; i <= n; i++)); do
    echo "Threads: 1 Tamanho: 150k Execução $i/3"
    echo "Threads: 1 Tamanho: 150k Execução $i/3" >> $saida
    echo $sample >> $saida && $sample >> $saida 2>&1
    echo "" >> $saida
done


sample="./main samples/150k1.fna samples/150k2.fna --match 2 --mismatch -3 --gap -5 --gap_seq -2 --blocks 20"
for ((i = 1; i <= n; i++)); do
    echo "Threads: 20 Tamanho: 150k Execução $i/3"
    echo "Threads: 20 Tamanho: 150k Execução $i/3" >> $saida
    echo $sample >> $saida && $sample >> $saida 2>&1
    echo "" >> $saida
done


sample="./main samples/150k1.fna samples/150k2.fna --match 2 --mismatch -3 --gap -5 --gap_seq -2 --blocks 40"
for ((i = 1; i <= n; i++)); do
    echo "Threads: 40 Tamanho: 150k Execução $i/3"
    echo "Threads: 40 Tamanho: 150k Execução $i/3" >> $saida
    echo $sample >> $saida && $sample >> $saida 2>&1
    echo "" >> $saida
done

echo "Execuções concluídas. Resultados salvos em $saida"

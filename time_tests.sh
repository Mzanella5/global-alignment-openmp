#!/bin/bash

# Número de vezes que você deseja executar o programa
n=3

mkdir teste



# Nome do programa em C
programa1="./main seq/covid-vacine1.fna seq/covid-vacine2.fna --match 2 --mismatch -3 --gap -5 --gap_seq -2 --blocks 8"

gcc -fopenmp main.c options.h options.c -o main

# Nome do arquivo de saída
saida="saida.txt"

# Loop para executar o programa n vezes
for ((i = 1; i <= n; i++)); do
    echo "Execução $i:"
    $programa1 >> $saida
    echo "--------------------"
done


echo "Execuções concluídas. Resultados salvos em $saida"

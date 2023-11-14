#Auto

cores=12
times=3
saida="saida.txt"

sudo gcc -fopenmp main.c options.h options.c -o main

#30k
#================================================================================================================
for ((j = 1; j <= cores; j++)); do
    sample="./main samples/30k1.fna samples/30k2.fna --match 2 --mismatch -3 --gap -5 --gap_seq -2 --blocks $j"
    for ((i = 1; i <= times; i++)); do
        message="Threads: $j Tamanho: 30k Execução $i/3"
        echo $message
        echo $message >> $saida
        echo $sample >> $saida && $sample >> $saida 2>&1
        echo "" >> $saida
    done
done

#80k
#================================================================================================================
for ((j = 1; j <= cores; j++)); do
    sample="./main samples/80k1.fna samples/80k2.fna --match 2 --mismatch -3 --gap -5 --gap_seq -2 --blocks $j"
    for ((i = 1; i <= times; i++)); do
        message="Threads: $j Tamanho: 80k Execução $i/3"
        echo $message
        echo $message >> $saida
        echo $sample >> $saida && $sample >> $saida 2>&1
        echo "" >> $saida
    done
done

#100k
#================================================================================================================
for ((j = 1; j <= cores; j++)); do
    sample="./main samples/100k1.fna samples/100k2.fna --match 2 --mismatch -3 --gap -5 --gap_seq -2 --blocks $j"
    for ((i = 1; i <= times; i++)); do
        message="Threads: $j Tamanho: 100k Execução $i/3"
        echo $message
        echo $message >> $saida
        echo $sample >> $saida && $sample >> $saida 2>&1
        echo "" >> $saida
    done
done

#150k
#================================================================================================================
for ((j = 1; j <= cores; j++)); do
    sample="./main samples/150k1.fna samples/150k2.fna --match 2 --mismatch -3 --gap -5 --gap_seq -2 --blocks $j"
    for ((i = 1; i <= times; i++)); do
        message="Threads: $j Tamanho: 150k Execução $i/3"
        echo $message
        echo $message >> $saida
        echo $sample >> $saida && $sample >> $saida 2>&1
        echo "" >> $saida
    done
done

echo "Execuções concluídas. Resultados salvos em $saida"
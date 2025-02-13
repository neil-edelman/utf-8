set terminal pngcairo enhanced font "modern,14" fontscale 1.0 size 600, 400 
set output "filesizes.png"

set key left
set style data histogram
set style histogram
set style fill solid 0.3
set boxwidth 2
set ylabel "size (KB)"
set xtics rotate out
set bars front
ilrblue = "#4fa6dc";

plot "filesizes.tsv" using 2:xticlabels(1) linecolor rgb ilrblue title "file size of executable"

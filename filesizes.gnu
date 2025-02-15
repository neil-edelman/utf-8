set key left
set grid
set style data histogram
set style histogram
set style fill solid 0.3
set boxwidth 2
set ylabel "size (B)"
set xtics rotate out
set bars front
ilrblue = "#4fa6dc";

set terminal pngcairo enhanced font "modern,14" fontscale 1.0 size 600, 400 
set output "filesizes.png"
plot "filesizes.tsv" using 2:xticlabels(1) linecolor rgb ilrblue title "file size of executable"

set terminal svg size 600, 400 font "modern,14" enhanced fontscale 1.0 dynamic name "filesizes" #mouse standalone
set output "filesizes.svg"
replot

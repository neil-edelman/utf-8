set grid
set style data histogram
set style histogram errorbars
set style fill solid 0.3
set errorbars linewidth 1
set boxwidth 2
set ylabel "time (s)"
set xtics rotate out
set bars front
skyblue = "#87CEEB";
ilrblue = "#4fa6dc";

set terminal pngcairo enhanced font "modern,14" fontscale 1.0 size 600, 400 
set output "korean.png"
plot "korean.tsv" using 2:3:xticlabels(1) linecolor rgb ilrblue title "all bibles plain text 5.0\nKorean Bible"

set terminal svg size 600, 400 dynamic font "modern,14" enhanced fontscale 1.0 #mouse standalone
set output "korean.svg"
replot

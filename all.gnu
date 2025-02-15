set key at 6.0,3.0
set grid
set style data histogram
set style histogram errorbars
set style fill solid 0.3
set errorbars linewidth 1
set boxwidth 2
set ylabel "time (s)"
set xtics rotate out
set bars front
set label 1 "blue screen of death\nafter 10 minutes" at 1,0.2 rotate by 90
ilrblue = "#4fa6dc";

set terminal pngcairo enhanced font "modern,14" fontscale 1.0 size 600, 400 
set output "all.png"
plot "all.tsv" using 2:3:xticlabels(1) linecolor rgb ilrblue title "all bibles plain text 5.0\n65 combined\ntranslations\n302MB"

set terminal svg size 600, 400 dynamic font "modern,14" enhanced fontscale 1.0 #mouse standalone
set output "all.svg"
replot

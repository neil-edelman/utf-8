set terminal pngcairo enhanced font "modern,14" fontscale 1.0 size 600, 400 
set output "all.png"

set key at 5.0,3.2
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


plot "all.tsv" using 2:3:xticlabels(1) linecolor rgb ilrblue title "all bibles plain text 5.0\n65 combined\ntranslations\n302MB"

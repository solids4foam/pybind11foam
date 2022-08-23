# To run this script, execute:
# $> gnuplot plotAll.gnuplot -p

set term pdfcairo enhanced color
set output "plot.pdf"

set xlabel "Y Coordinate (in m)"
set ylabel "Temperature (in K)"
set key top left
set grid xtics ytics
set xtics 0.01
set ytics 10
set yrange [270:380]
plot \
    "postProcessing/singleGraph/5/line_T.xy" u 1:2 w lp pt 12 t "Embedded Python code", \
    "postProcessing.laplacianFoam/singleGraph/5/line_T.xy" u 1:2 w lp pt 11 t "laplacianFoam"
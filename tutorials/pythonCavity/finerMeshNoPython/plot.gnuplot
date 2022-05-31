# To run this script, execute:
# $> gnuplot plotAll.gnuplot -p

set term pdfcairo enhanced color
set output "plot.pdf"

set xlabel "X Coordinate (in m)"
set ylabel "X Component of Velocity (in m/s)"
set key top right
set grid xtics ytics
set xtics 0.01
set ytics 0.25
set yrange [-1.25:1.25]
plot \
    "movingWall.Ux.txt" u (0.1/40 + $0*0.1/20):1 w p pt 12 t "OpenFOAM", \
    sin(0.5*pi)*sin(x*40*pi) w l t "Analytical"


#"postProcessing/singleGraph/0.5/line_Ux.xy" u 1:2 w p pt 11 t "OpenFOAM", \

reset

FILE = "graph3.dat"

IdIdxs = XYs = ' '
stats FILE u (IdIdxs=IdIdxs.sprintf("%s:%d ",strcol(1),$0), \
              XYs=XYs.sprintf("%g %g ",$2,$3)) index 0 nooutput
Px(i) = real(word(XYs,2*i+1))
Py(i) = real(word(XYs,2*i+2))

getIdx(col) = (c0=strstrt(IdIdxs,sprintf(" %s:",strcol(col))), \
              c1=strstrt(IdIdxs[c0+1:],' ')+c0, \
              s0=IdIdxs[c0:c1], c2=strstrt(s0,':'), int(s0[c2+1:]))

set size ratio 1
set key noautotitle
set offsets 1.0,1.0,1.0,1.0
set style fill transparent solid 0.5 noborder

plot FILE index 1 u (idx0=getIdx(1),x0=Px(idx0)):(y0=Py(idx0)): \
         (idx1=getIdx(2),Px(idx1)-x0):(Py(idx1)-y0):3 w vec lw 2 lc rgb "pink" nohead, \
       '' index 0 u 2:3:4 w p pt 7 ps 6 lc rgb 220, \
       '' index 0 u 2:3 w p pt 6 ps 6 lc rgb "black", \
       '' index 0 u 2:3:1 w labels

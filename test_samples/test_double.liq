var d1 = 0.3
var d2 = -1.4
var d3 = 1.5e2
var d4 = 1.5e+2
var d5 = 1.5e-2
var d6 = 1.e2
displayln("%lf %lf %lf %lf %lf %lf", d1, d2, d3, d4, d5, d6)
var d7 = .5e1
var d8 = 1.e1
var d9 = .4
displayln("%lf %lf %lf", d7, d8, d9)

def aKlass
    int eins=1
    
aKlass xK
# a.eins has a pattern .e which could be a double.
displayln("%d", xK.eins)
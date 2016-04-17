
var myarray = [1,2,3,4]
int one = myarray[0]
displayln("One=%d", one)
var zwei = myarray[1]
displayln("Zwei=%d", zwei)
displayln("%d,%d,%d,%d", myarray[0],myarray[1],myarray[2],myarray[3])

var dinge = [ 1+2, "Otto", true ]
string b = "false"
if dinge[2] 
    b = "true"
displayln("%d, %s, %s", dinge[0], dinge[1], b )

var andere_dinge = [dinge, 5,6]
var tt = andere_dinge[0]
var tt2 = tt[0]
printvalue(tt2) # => 3

var abc = andere_dinge[0][0]
printvalue(abc) # => 3
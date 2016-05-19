# Test for simple list of numbers
var myarray = [1,2,3,4]
int one = myarray[0]
displayln("One=%d", one)
var zwei = myarray[1]
displayln("Zwei=%d", zwei)
displayln("%d,%d,%d,%d", myarray[0],myarray[1],myarray[2],myarray[3])

# Test list of elemts with different types
var dinge = [ 1+2, "Otto", true ]
displayln("%d, %s, %d", dinge[0], dinge[1], dinge[2] )

# Test list in list access.
var andere_dinge = [dinge, 5,6]
var tt = andere_dinge[0]
var tt2 = tt[0]
printvalue(tt2) # => 3

var abc = andere_dinge[0][0]
printvalue(abc) # => 3

#Test concatenate two lists.
var sachen = [2+3, "karl", 5.5]
var concatenate = dinge + sachen
displayln("%d,%s,%d,%d,%s,%lf", concatenate[0],concatenate[1],concatenate[2] ,concatenate[3],concatenate[4],concatenate[5])

myarray << 5
printvalue(myarray[4])
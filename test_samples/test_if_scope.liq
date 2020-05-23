# Test scope in if statement

var xNum = 1
if xNum > 0
	var yNum = 5
	xNum = y
else
	printvalue(yNum) # must fail

printvalue(xNum)
var zNum = yNum # must fail yNum out of scope
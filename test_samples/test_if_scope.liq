# Test scope in if statement

var x = 1
if x > 0
	var y = 5
	x = y
else
	printvalue(y) # must fail

printvalue(x)
var z = y # must fail y out of scope
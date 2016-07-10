@{ The only case which currently works@}
var l = [1,2]
var n = 3
l << n
n = n +1
l<<n
@{@}

@{ This doesn't work, since l is not reconstructed in the loop.
while n < 5
    l << n
    n = n +1
@}    

displayln("%d,%d,%d,%d", l[0],l[1],l[2],l[3])

#var l2 = [3::5] # => [3,4,5]
#displayln("%d,%d,%d", l2[0],l2[1],l2[2])

@{
def first()
    return 1
def second()
    return 3

var m = [ 1+2 -> 4] # => [3,4]
var n = [first()->second()] # => [1,2,3]
@}
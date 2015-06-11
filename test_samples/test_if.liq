
def max(int a, int b) : int
  if a > b 
    return a 
  else 
    return b

def min(int a, int b) : int
  if a > b
    return b
  return a

def testif(int a, int b) : int
    int x = 0 
    if a == b
        return a
    else
        x = b
    return x

def testifMax(int a, int b) : int
    int max = 0
    if a > b
        max = a
    else
        max = b
    return max

int m = max( 1,10)
display("Max is %d expected 10\n", m)
m = min( 1,10)
display("Min is %d\ expected 1n", m)
int n = testif( 1,10)
display("testif is %d expected 10\n", n)

n = testif( 5,5)
display("testif is %d expected 5\n", n)

display("testifMax is %d expected 10 \n", testifMax(1,10))
display("testifMax is %d expected 10 \n", testifMax(10,1))

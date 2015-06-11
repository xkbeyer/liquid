string s = " bla 1
bla '2' \n
bla 3 \"
---"
displayln(s)


s = 'He says:
  "Hello ol\' boy"
and leaves the place'
displayln(s)

int id = 0
while id > 0
   id = id - 1
   printvalue(id)
else
  display("upps nix\n")

id = 3
while id > 0
   id = id - 1
   printvalue(id)
else
  display("upps nix\n")

display("Hi : ")

@{ This is a block
of comments
@}
# While this is a one line comment 


boolean bool = true 
if bool 
  display("found true\n")
else
  display("found false\n")

def makeX(int x, int y) : int 
  display("makeX(%d, %d)\n", x,y)
  return x + y

int _abc = 1
int a = 1
int b = 2 
int a-b = a - b
boolean d = a and b or a-b
int e = a * b + a-b
display("e=%d\n", e)
e = makeX( a, b)

int i = 5
int j = not i
display("j=%d\n",j)
s = "Dies ist ein String."
displayln(s)


int x = makeX(3,4)
display("x=%d\n",x)
if x < 6  
  x = x + 6 
  x = x * 2
  printvalue(x)
else 
  x=4
double dval = 0.5
dval = sin(dval)
printdouble(dval)


def counter
    string _name
    int _count
    def init()
        self._count = 50
        self._name = "container of elements"
    def add(int element)
        self._count = self._count + element
    def sub(int element)
        self._count = self._count - element
    def get() : int
        return self._count
    def name() : string
        return self._name
    def _init(int count, string name)
        self._count = count
        self._name = name

def add2(counter c)
    c._count = c._count + 2

counter c
c._count = 7
c._name = "leer"
display("c.count=%d c.name=%s\n", c._count, c._name)
c.init()
display("Nach init() c.count=%d c.name=%s\n", c._count, c._name)
c.add(10)
display("Nach add() c.count=%d c.name=%s\n", c._count, c._name)
c.sub(10)
display("Nach sub() c.count=%d c.name=%s\n", c.get(), c._name)
c._init(5, "neu")
display("Nach _init() c.count=%d c.name=%s\n", c.get(), c.name())
add(c, 5)
display("Nach add(c,5) c.count=%d c.name=%s\n", c.get(), c.name())
sub(c, 5)
display("Nach sub(c,5) c.count=%d c.name=%s\n", c.get(), c.name())
c.add2()
display("Nach c.add2() c.count=%d c.name=%s\n", c._count, c._name)


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
display("Max is %d\n", m)
m = min( 1,10)
display("Min is %d\n", m)
int n = testif( 1,10)
display("testif is %d\n", n)

display("testifMax is %d\n", testifMax(1,10))

# Test class method which can be defined outside the class definition. Can be called w/ class object as parameter, too.
def pampa
    int myint
    def get() : int
        return self.myint

def f(pampa p)
    p.myint = 7

pampa p

p.myint = 5
p.f()
displayln("p.get() returns %d should be 7", p.get())
p.myint = 5

f(p)
displayln("p.get() returns %d should be 7", p.get())
a = get(p)
displayln("a returns %d should be 7", a)

p.myint = 5
pampa t = p
a = t.get()
displayln("a returns %d should be 5", a)

displayln("p.get() returns %d should be 5", p.get())

# Test class ctor (special method name)
def ups
    int ppp
    def __init__()
        self.ppp = 9
    def get() : int
        return self.ppp

ups u
displayln("u.get() returns %d should be 9", u.get())

# Test member init while class instantiation.
def memberinit
    int _count = 25
    string _name = "empty"

memberinit membInit

display("membInit.count=%d (25) membInit.name=%s (empty)\n", membInit._count, membInit._name)

# Test a getter function w/o parameters and therefore no braces needed.
def simple
    int myint = 5
    def get() : int
        return self.myint

simple ps

i = ps.get
displayln("ps.get returns %d should be 5", ps.get())

# Test var keyword
import test_var.liq

test_var()

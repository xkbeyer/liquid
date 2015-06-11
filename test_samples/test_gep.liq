
def counter
    string _name
    int _count

def add2(counter c)
    c._count = c._count + 2


counter c
c._count = 7
c._name = "leer"

display("c.count=%d (7) c.name=%s (leer)\n", c._count, c._name)

c.add2()
display("Nach c.add2(+2) c.count=%d c.name=%s\n", c._count, c._name)

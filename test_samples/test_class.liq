
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
@{
@}

def add2(counter c)
    c._count = c._count + 2

counter c
c._count = 7
c._name = "leer"
display("c.count=%d (7) c.name=%s (leer)\n", c._count, c._name)
c.init()
display("Nach init(50,container of elements) c.count=%d c.name=%s\n", c._count, c._name)
c.add(10)
display("Nach add(10) c.count=%d c.name=%s\n", c._count, c._name)
c.sub(10)
display("Nach sub(10) c.count=%d c.name=%s\n", c.get(), c._name)
c._init(5, "neu")
display("Nach _init(5,neu) c.count=%d c.name=%s\n", c.get(), c.name())
add(c, 5)
display("Nach add(c,5) c.count=%d c.name=%s\n", c.get(), c.name())
sub(c, 5)
display("Nach sub(c,5) c.count=%d c.name=%s\n", c.get(), c.name())
@{
@}
c.add2()
display("Nach c.add2(+2) c.count=%d c.name=%s\n", c._count, c._name)

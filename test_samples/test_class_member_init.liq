def counter_mem_init
    int _count = 25
    string _name = "empty"

counter_mem_init c_mem

display("c.count=%d (25) c.name=%s (empty)\n", c_mem._count, c_mem._name)
	
counter_mem_init c2_mem = c_mem
display("c2.count=%d (25) c2.name=%s (empty)\n", c2_mem._count, c2_mem._name)

def cl
	int _mem
	
cl d_cl
d_cl._mem = 2
display("d.mem=%d (2)\n", d_cl._mem)

import sys
import new

GT      = "0100"
XOR     = "0110"
NEQ     = XOR
OR      = "0111"
NOT     = "1000"
AND     = "0001"
NOT_AND = "0100"
ALL1    = "1111"
EQ      = "0011"

PHONY_WIRE = -1

class Gate:
    def __init__(self, lw, rw, ow, f):
        if (not type(lw)==type(rw)==type(ow)==int) or (not type(f)==str):
            sys.exit("!!! bad gate!")
        self.lw = lw
        self.rw = rw
        self.ow = ow
        self.f = f
    def __str__(self):
        return "2 1 %d %d %d %s"%(self.lw, self.rw, self.ow, self.f)

class Circuit:
    def __init__(self, num_parties, inputs_per_party_list):
        self.np = num_parties
        self.iipl = inputs_per_party_list
        self.ni = 0
        self.in_st = []
        for inputs in self.iipl:
            self.in_st.append(self.ni)
            self.ni += inputs
        self.free = self.ni
        self.gates = []
        self.ow = [] #list of output wires
    def add_gate(self, l, r, f):
        self.gates.append(Gate(l,r,self.free, f))
        outw = self.free
        self.free +=1
        return outw
    def add_to_output_wires(self, wire):
        if type(wire)==int:
            self.ow.append(wire)
        elif type(wire)==list:
            for w in wire:
                self.ow.append(w)
#     def replace_wire(self, orig, new):
#         for g in self.gates:
#             if g.lw == orig:
#                 g.lw = new
#             if g.rw == orig:
#                 g.rw = new
#             if g.ow == orig:
#                 g.ow = new
#     def switch_wires(self, first, second):
#         self.replace_wire(second, PHONY_WIRE)
#         self.replace_wire(first, second)
#         self.replace_wire(PHONY_WIRE, first)
#     def organize(self):
#         num_out_wires = len(self.ow)
#         ow0 = self.free-num_out_wires
#         for i in range(num_out_wires):
#             w = ow0+i
# #             print "switch %d with %d"%(w,self.ow[i])
#             self.switch_wires(w, self.ow[i])
#         self.ow = range(ow0,ow0+num_out_wires)
    def wire_outputs(self):
        new_ow = []
        for w in self.ow:
            new_ow.append(self.add_gate(w, w, EQ))
        self.ow = new_ow
    def __str__(self):
        #add num gates and num parties
        circuit_str = "%d\n%d\n"%(len(self.gates), self.np)
        #add input wires for evesecondry party
        for i in range(self.np):
            circuit_str += "%d %d\n"%(i+1,self.iipl[i]) 
            for j in range(self.in_st[i], self.in_st[i]+self.iipl[i]):
                circuit_str += "%d "%j
            circuit_str += "\n"
        #add output wires
        circuit_str += "%d\n"%len(self.ow)
        for w in self.ow:
            circuit_str += "%d "%w
        circuit_str += "\n"
        #add gates
        for g in self.gates:
            circuit_str += str(g)+"\n"
        return circuit_str

import circuit
import lt


def max_block(cir,nnumbers, nbits, x0, output_max=True, ign=None):
    """
    Computes and returns max(x1,x2,...,xk) where xi is a nbits unsigned number. 
    Also return nnumbers bits, for each 1 means maximum and 0 o/w. 
    :INPUT:
    :param cir: circuit object to work on
    :param nnumbers: how many numbers to compare
    :param nbits: number of bits each number has
    :param x0: list of the first wire of each of the numbers
    :param output_max: whether to output the wires that are corresponds to the 
            maximum value or otherwise output only the bitmap.
    :param ign: Wires that represent a bitmap of the number (one bit per number).
                Bit i (ign[i]) tells whther to ignore the number xi  
    
    :OUTPUT:
    :return bitmap: nnumbers wires s.t. 1 on the i wire means that the i-th number 
            is the maximum among those which are not in the ignore list ign  
    :return max: nbits wires which are copy of max(x1,x2,...,xk).
    
    :DESIGN:
    First conmputs the maximum:
        set max1 = x1
        For i=1...nnumbers-1:
            lti+1,maxi+1 = lt(maxi,xi+1)
    
    Then computes one wire per number by setting the rightest 1 lt wire as 1 and 
        zeroing all the others:
        set lt1 = 1
        set c = 0
        For i=nnumbers...1:
            biti = (NOT c) AND lti
            c = c OR biti
    """
    # if we have the ignore list then we transform xi to (1-ign[i])*xi
    # this way, if ign[i]==1 then the number that would be compared is zero.
    ng = len(cir.gates)
    if ign:
        newx0 = []
        for i in range(nnumbers):
            xi0 = x0[i]
            for j in range(nbits):
                transformed = cir.add_gate(ign[i],xi0+j, circuit.NOT_AND)
                if j==0:
                    newx0.append(transformed)
        x0 = newx0
#         print "gates for bitmap conversion: %d-%d"%(ng+1,len(cir.gates))
    #set the first lt_bit to one, because x0 is greater then nothing
    ONE = cir.add_gate(x0[0], x0[0], circuit.ALL1)
    lt_bit = {0:ONE}
    maximum0=x0[0]
    maximum = []
    for i in range(1,nnumbers):
        ng = len(cir.gates)
        if i==nnumbers-1 and output_max==False:
            lt_bit[i],maximum = lt.lt_block(cir, nbits, maximum0, x0[i], False)
        else:
            lt_bit[i],maximum = lt.lt_block(cir, nbits, maximum0, x0[i], True)
            maximum0 = maximum[0]
#         print "gates for lt number %d: %d-%d"%(i,ng+1,len(cir.gates))
    #create the bit map of is_max
    max_bit = {nnumbers-1:lt_bit[nnumbers-1]}
    max_found = max_bit[nnumbers-1]
    ng = len(cir.gates)
    for i in range(nnumbers-2,-1,-1):
        max_bit[i] = cir.add_gate(max_found,lt_bit[i], circuit.NOT_AND)
        if not i==0:
            max_found = cir.add_gate(max_found, lt_bit[i], circuit.OR)
#     print "gates for getting the bitmap: %d-%d"%(ng+1,len(cir.gates))
    
    #arrange the bitmap into a list to be returned
    max_bitmap = []
    for i in range(nnumbers):
        max_bitmap.append(max_bit[i])
    return (max_bitmap, maximum)


def build_circuit(bitlength, n_parties, output_max=True):
    num_inputs = [bitlength]*n_parties
    c = circuit.Circuit(n_parties, num_inputs)
    x0=[0]
    for i in range(1,n_parties):
        x0.append(x0[i-1]+num_inputs[i-1])
    bitmap,maximum = max_block(c, n_parties, bitlength, x0, output_max, None)
    if output_max == True:
        c.add_to_output_wires(maximum)
    c.add_to_output_wires(bitmap)
    c.wire_outputs()
    return str(c)

if __name__ == "__main__":
    bitlength = 16
    n_parties = 3
#     print str(build_circuit(bitlength,n_parties, True))
    print str(build_circuit(bitlength,n_parties, False))
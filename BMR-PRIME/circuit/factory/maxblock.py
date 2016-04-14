import circuit
import lt


def max_block(cir,nnumbers, nbits, x0):
    """
    Computes and returns max(x1,x2,...,xk) where xi is a nbits unsigned number. 
    Also return nnumbers bits, for each 1 means maximum and 0 o/w. 
    :INPUT:
    :param cir: circuit object to work on
    :param nnumbers: how many numbers to compare
    :param nbits: number of bits each number has
    :param x0: list of the first wire of each of the numbers
    
    :OUTPUT:
    :return bitmap: nnumbers wires s.t. 1 on the i wire means that the i-th number is the maximum  
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
    ONE = cir.add_gate(x0[0], x0[0], circuit.ALL1)
#     ZERO = cir.add_gate(x0[0], x0[0], circuit.XOR)
    #set the first lt_bit to one, because x0 is greater then nothing
    lt_bit = {0:ONE}
    maximum0=x0[0]
    maximum = []
    for i in range(1,nnumbers):
        lt_bit[i],maximum = lt.lt_block(cir, nbits, maximum0, x0[i], output_max=True)
        maximum0 = maximum[0]
    
    #create the bit map of is_max
    max_bit = {nnumbers-1:lt_bit[nnumbers-1]}
    max_found = max_bit[nnumbers-1]
    for i in range(nnumbers-2,-1,-1):
        max_bit[i] = cir.add_gate(max_found,lt_bit[i], circuit.NOT_AND)
        if not i==0:
            max_found = cir.add_gate(max_found, lt_bit[i], circuit.OR)
    
    #arrange the bitmap into a list to be returned
    max_bitmap = []
    for i in range(nnumbers):
        max_bitmap.append(max_bit[i])
    return (max_bitmap, maximum)


def build_circuit(bitlength, n_parties):
    num_inputs = [bitlength]*n_parties
    c = circuit.Circuit(n_parties, num_inputs)
    x0=[0]
    for i in range(1,n_parties):
        x0.append(x0[i-1]+num_inputs[i-1])
    bitmap,maximum = max_block(c, n_parties, bitlength, x0)
    
    c.add_to_output_wires(maximum)
    c.add_to_output_wires(bitmap)
    c.wire_outputs()
    return str(c)

if __name__ == "__main__":
    bitlength = 16
    n_parties = 3
    print str(build_circuit(bitlength,n_parties))
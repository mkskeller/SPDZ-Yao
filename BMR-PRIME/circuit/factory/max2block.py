import circuit
import maxblock


def max2_block(cir,nnumbers, nbits, x0, output_max=True):
    """
    Computes and returns second max(x1,x2,...,xk) where xi is a nbits unsigned number. 
    Also return nnumbers bits, for each 1 means maximum and 0 o/w. 
    :INPUTS @see maxblock.py:
    :OUTPUT @see maxblock.py:
    
    :DESIGN:
    First conmputs the maximum and the bitmap:
        bitmap,maximum = max_block(cir, nnumbers, nbits, x0)
    
    Then computs the second maximum number:
        second_bitmap,maximum = max_block(cir, nnumbers, nbits, x0, bitmap)
    """
    bitmap1,_ = maxblock.max_block(cir, nnumbers, nbits, x0, False, None)
    bitmap2,maximum = maxblock.max_block(cir, nnumbers, nbits, x0, output_max, bitmap1)
    return (bitmap2,maximum) 


def build_circuit(bitlength, n_parties, output_max=True):
    num_inputs = [bitlength]*n_parties
    c = circuit.Circuit(n_parties, num_inputs)
    x0=[0]
    for i in range(1,n_parties):
        x0.append(x0[i-1]+num_inputs[i-1])
    bitmap,maximum = max2_block(c, n_parties, bitlength, x0, output_max)
    
    c.add_to_output_wires(maximum)
    c.add_to_output_wires(bitmap)
    c.wire_outputs()
    return str(c)

if __name__ == "__main__":
    bitlength = 16
    n_parties = 3
    print str(build_circuit(bitlength,n_parties))
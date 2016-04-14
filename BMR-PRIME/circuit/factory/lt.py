
import circuit

def lt_block(cir, n, x0, y0, output_max=True):
    """
    Add the circuitry to compute the less then function on two n-bits numbers
    begins with x0 and y0.
    :param cir:  circuit object
    :param n:  bit length
    :param x0: first wire that represents the left operand
    :param y0: first wire that represents the right operand
    
    :ouputs: list of ouput wires:
        - first wire tells whether X<Y (1 if it does and 0 o/w)
        - next 16 wires are equal to max(X,Y) 

    :Design for achieving X<Y:
        X=x0,x1,...,x{n-1}; Y=y0,y1,...,y{n-1}
        a0 = x0 XOR x0
        b0 = a0
        c0 = y0  //( if we wanted to achieve X>Y we should have taken x0)
        ...lt_block
        ...
        ai= xi XOR yi
        bi = b_{i-1} OR ai  //(i.e. whether x1...xi NEQ y1...yi)
        ci = ( b_{i-1} AND c_{i-1} ) OR ( NOT b_{i-1} AND yi) // computed by the NOT_AND gate
    """
    a = {0:cir.add_gate(x0,y0, circuit.XOR)}
    b = {0:a[0]}
    c = {0:y0}
    
    for i in range(1,n):
        xi = x0+i
        yi = y0+i
        temp1 = cir.add_gate(b[i-1], c[i-1], circuit.AND)
        temp2 = cir.add_gate(b[i-1], yi, circuit.NOT_AND)
        c[i] = cir.add_gate(temp1, temp2, circuit.OR)
        if not i==(n-1):
            a[i] = cir.add_gate(xi,yi, circuit.XOR)
            b[i] = cir.add_gate(b[i-1], a[i], circuit.OR)
    lt = c[n-1]
    if not output_max:
        lt = cir.add_gate(lt, x0+n-1, circuit.LEFT)
    
    maximum=[]
    if output_max:
        temp1 = {}
        temp2 = {}
        max = {}
        for i in range(0,n):
            xi = x0+i
            yi = y0+i
            temp1[i] = cir.add_gate(lt, yi, circuit.AND)
            temp2[i] = cir.add_gate(lt, xi, circuit.NOT_AND)
        for i in range(0,n):
            max[i] = cir.add_gate(temp1[i], temp2[i], circuit.OR)
            maximum.append(max[i])
    
    return (lt,maximum)
    

if __name__ == "__main__":
    BIT_LENGTH = 16
    c = circuit.Circuit(2,[16,16])
    lt,maximum = lt_block(c, BIT_LENGTH, 0,BIT_LENGTH, True)
    c.add_to_output_wires(maximum)
    print str(c)

import sys
sys.path.insert(0, '../factory')
from subprocess import Popen, PIPE
from random import randint
import os

import maxblock
from gi.overrides.keysyms import numbersign

def run_boolean_circuit(circuit_file, inputs, output_len):
    process = Popen([BC_BINARY, circuit_file, inputs], stdout=PIPE)
    (output, err) = process.communicate()
    exit_code = process.wait()
    circuit_out = output[output_len*(-1)-1:-1] if exit_code==0 else "ERROR"
    return (exit_code, output, err, circuit_out)

TEST_FILE = "test.txt"
BC_BINARY = "./bc"
T = 5 #number of tests
def test_max(bitlength,n_parties):
    circuit_str = maxblock.build_circuit(bitlength,n_parties)
    
    # write the circuit to a file
    f = open(TEST_FILE,'w')
    f.write(circuit_str)
    f.close()
    
    # generate inputs/outputs and run the boolean circuit test on the file
    format_str = "#0%db"%(bitlength+2)
    for i in range(T):
        intput_str = ""
        output_str = ""
        numbers = []
        for p in range(n_parties):
            num = randint(0,2**bitlength-1)
            numbers.append(num)
            intput_str += format(num,format_str)[2:]
        maximum = max(numbers)
        output_str += format(maximum,format_str)[2:]
        output_str += ''.join(list(map((lambda x: str(int(x==maximum))), numbers)))
        
        exit_code, output, err, circuit_out = run_boolean_circuit(TEST_FILE, intput_str, len(output_str))
        if not exit_code==0:
            print ERROR
            print "numbers:"+ str(numbers)
            print '\n'.join(list(map( lambda x: format(x,format_str)[2:], numbers)))
            print "output expected: "+ output_str
            print "program execution:"
            print "exit code = %d"%exit_code
            print "output:\n%s\n"%output
            print "error:\n%s\n"%err
            exit(1)
        if not circuit_out == output_str:
            print "ERROR!"
            exit(1)
        print "SUCCEEDED"
    
if __name__=="__main__":
    test_max(16,100)
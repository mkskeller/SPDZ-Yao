import sys
sys.path.insert(0, '../factory')
from subprocess import Popen, PIPE
from random import randint
import os

import maxblock
import max2block
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

def in_out_for_max(bitlength,n_parties,output_max, k=1):
    """generate inputs and outputs for the k-max number function"""
    format_str = "#0%db"%(bitlength+2)
    input_str = ""
    output_str = ""
    numbers = []
    for p in range(n_parties):
        num = randint(0,2**bitlength-1)
        numbers.append(num)
        input_str += format(num,format_str)[2:]
    orig_numbers = numbers[:]
    for i in range(k-1):
        numbers.remove(max(numbers))
    maximum = max(numbers)
    if output_max == True:
        output_str += format(maximum,format_str)[2:]
    output_str += ''.join(list(map((lambda x: str(int(x==maximum))), orig_numbers)))
    return (orig_numbers, input_str, output_str)

def test_max(bitlength,n_parties, output_max):
    circuit_str = maxblock.build_circuit(bitlength,n_parties, output_max)
    
    # write the circuit to a file
    f = open(TEST_FILE,'w')
    f.write(circuit_str)
    f.close()
    
    # generate inputs/outputs and run the boolean circuit test on the file
    format_str = "#0%db"%(bitlength+2)
    for i in range(T):
        while True:
            orig_numbers, input_str, output_str = in_out_for_max(bitlength,n_parties, output_max, 1)
            if len(orig_numbers) == len(set(orig_numbers)):
                break
        exit_code, output, err, circuit_out = run_boolean_circuit(TEST_FILE, input_str, len(output_str))
        if not exit_code==0:
            print "ERROR"
            print "numbers:"+ str(orig_numbers)
            print '\n'.join(list(map( lambda x: format(x,format_str)[2:], orig_numbers)))
            print "output expected: "+ output_str
            print "program execution:"
            print "exit code = %d"%exit_code
            print "output:\n%s\n"%output
            print "error:\n%s\n"%err
            exit(1)
        if not circuit_out == output_str:
            print "ERROR!"
            print "numbers:"+ str(orig_numbers)
            print '\n'.join(list(map( lambda x: format(x,format_str)[2:], orig_numbers)))
            print "output expected: "+ output_str
            print "program execution:"
            print "output:\n%s\n"%output
            exit(1)
        print "SUCCEEDED"

def test_max2(bitlength,n_parties, output_max=True):
    circuit_str = max2block.build_circuit(bitlength,n_parties, output_max )
    
    # write the circuit to a file
    f = open(TEST_FILE,'w')
    f.write(circuit_str)
    f.close()
    
    # generate inputs/outputs and run the boolean circuit test on the file
    format_str = "#0%db"%(bitlength+2)
    for i in range(T):
        while True:
            orig_numbers, input_str, output_str = in_out_for_max(bitlength,n_parties, output_max, 2)
            if len(orig_numbers) == len(set(orig_numbers)):
                break
        exit_code, output, err, circuit_out = run_boolean_circuit(TEST_FILE, input_str, len(output_str))
        if not exit_code==0:
            print "ERROR"
            print "numbers:"+ str(orig_numbers)
            print '\n'.join(list(map( lambda x: format(x,format_str)[2:], orig_numbers)))
            print "output expected: "+ output_str
            print "program execution:"
            print "exit code = %d"%exit_code
            print "output:\n%s\n"%output
            print "error:\n%s\n"%err
            exit(1)
        if not circuit_out == output_str:
            print "ERROR!"
            print "numbers:"+ str(orig_numbers)
            print '\n'.join(list(map( lambda x: format(x,format_str)[2:], orig_numbers)))
            print "output expected: "+ output_str
            print "program execution:"
            print "output:\n%s\n"%output
            exit(1)
        print "numbers: \n%s\n%s"%(str(orig_numbers),str(list(map( lambda x: format(x,format_str)[2:], orig_numbers))))
        print "output: %s\n"%circuit_out
        print "SUCCEEDED"

if __name__=="__main__":
    test_max2(16,10,True)

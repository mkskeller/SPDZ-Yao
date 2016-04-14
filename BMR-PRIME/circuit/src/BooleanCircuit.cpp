
#include "BooleanCircuit.h"

static void throw_party_exists(pid_t pid, unsigned int pos) {
	std::cout << "ERROR: in circuit description" << std::endl
			  << "\tPosition: " << pos << std::endl
			  << "\tPlayer id " << pid << " already exists" << std::endl;
	throw std::invalid_argument( "player id error" );
}

static void throw_bad_circuit_file() {
	std::cout << "ERROR: could not read circuit file" << std::endl;
	throw std::invalid_argument( "bad circuit file" );
}
void BooleanCircuit::_parse_circuit(const char* desc_file)
{
	std::ifstream circuit_file(desc_file);
	if(!circuit_file.good()) throw_bad_circuit_file();

	circuit_file >> _num_gates;
		_gates.resize(_num_gates+1); //+1 since gates are indexed from 1...
	circuit_file >> _num_parties;
		_parties.resize(_num_parties+1); //+1 since parties are indexed from 1...

	int wire_id;
	/* Parse circuit input wires */
	for (int idx_party = 1; idx_party <= _num_parties; idx_party++)
	{
		unsigned int p_id, num_input_wires;
		circuit_file >> p_id >> num_input_wires;
		if ( p_id != idx_party )
			throw_party_exists(p_id, circuit_file.tellg());
		for (unsigned int i = 0; i < num_input_wires; i++)
			circuit_file >> wire_id; //ignore this wire_id

		_parties[p_id].init(_num_input_wires,num_input_wires); //note the difference between the two args
		std::cout << "party " << p_id << ": " <<  _parties[p_id].wires << " " << _parties[p_id].n_wires << std::endl;
		_num_input_wires += num_input_wires;
	}

	/* Parse output wires */
	circuit_file >> _num_output_wires;
	circuit_file >> _output_start;
	std::cout << "# output wires: " << _num_output_wires << " starts at " << _output_start << std::endl;
	int largest_output_wire = _output_start;
	for (unsigned int i = 1; i < _num_output_wires; i++) //NOTE: starts with i=1 !
		circuit_file >> largest_output_wire; // ignore all but the last one because the first one has been already read
	_wires.resize(_output_start, Wire(false));
	_wires.resize(largest_output_wire+1, Wire(true));
//	for(int j=0; j<_wires.size(); j++) {
//			_wires[j].print(j);
//		}

	/* Parse gates */
	for (gate_id_t gate_id = 1; gate_id <= _num_gates; gate_id++)
	{
		int fan_in, fan_out, left, right, out;
		std::string func;

		circuit_file >> fan_in >> fan_out >> left >> right >> out >> func;
		_gates[gate_id].init(left, right, out, func);
		_wires[left]._enters_to.push_back(gate_id);
		_wires[right]._enters_to.push_back(gate_id);
		_wires[out]._out_from = gate_id;

		if(left < _num_input_wires && right < _num_input_wires)
			_ready_gates_set.insert(gate_id);
	}

//	for (gate_id_t gate_id = 1; gate_id <= _num_gates; gate_id++)
//	{
//		_gates[gate_id].print(gate_id);
//	}

	circuit_file.close();
}

/*  - if less inputs are given then it autocompletes to zeros
 *  - if more is given then it is shortened */
void BooleanCircuit::RawInputs(std::string raw_inputs)
{
	raw_inputs.resize(_num_input_wires,'0');
	std::cout << "Input:" << std::endl << raw_inputs << std::endl;
	for (int i=0; i<_num_input_wires; i++)
		_wires[i].Sig(raw_inputs[i]=='0'?0:1);

//	for(int j=0; j<_wires.size(); j++) {
//		_wires[j].print(j);
//	}
}

void BooleanCircuit::Inputs(const char* inputs_file_path)
{
	std::ifstream inputs_file(inputs_file_path);
	std::string raw_inputs;
	getline (inputs_file, raw_inputs, (char) inputs_file.eof());
	inputs_file.close();

}

BooleanCircuit::BooleanCircuit(const char* desc_file)
:_num_evaluated_out_wires(0),_num_input_wires(0)
{
	_parse_circuit(desc_file);
	printf("wires size  = %d\n",_wires.size());
	_make_layers();
}



void BooleanCircuit::_make_layers()
{
	_max_layer_sz = 0;
	printf("wires size  = %d\n",_wires.size());
	for (wire_id_t w=_output_start; w<_wires.size(); w++) {
		gate_id_t output_gate = _wires[w]._out_from;
//		printf("\nmaking layers from gate %u\n",output_gate);
		__make_layers(output_gate);
	}
	_validate_layers();
}

int BooleanCircuit::__make_layers(gate_id_t g)
{
//	printf("gate %u\n",g);
	if(_gates[g]._layer != NO_LAYER){
//		printf("already in layer %d\n", _gates[g]._layer);
		return _gates[g]._layer;
	}

	if (_gates[g]._left < _num_input_wires && _gates[g]._right < _num_input_wires) {
//		printf("it's in layer 0\n");
		_gates[g]._layer = 0;
		_add_to_layer(0, g);
		return 0;
	}
	int layer_left, layer_right;
	if(_gates[g]._left == NULL)
		layer_left = -1;
	else
		layer_left = __make_layers(_wires[_gates[g]._left]._out_from);
	if(_gates[g]._right == NULL)
		layer_right = -1;
	else
		layer_right = __make_layers(_wires[_gates[g]._right]._out_from);

	int max = layer_left>layer_right? layer_left : layer_right;
	int my_layer = 1 + max;
//	printf("gate %u: left=%d, right=%d, my=%d\n", g, layer_left, layer_right, my_layer);
	_gates[g]._layer = my_layer;
	_add_to_layer(my_layer, g);
	return my_layer;
}

void BooleanCircuit::_add_to_layer(int layer, gate_id_t g)
{
//	printf("adding %u to layer %d\n", g, layer);
	if (_layers.size()<layer+1) {
//		printf("layer doesn't exist, creating it\n");
		_layers.resize(layer+1);
	}
	_layers[layer].push_back(g);
}

void BooleanCircuit::_print_layers()
{
	printf("num layers = %d\n",_layers.size());
	for(int i=0; i<_layers.size(); i++) {
		printf ("\nlayer %d size=%d\n",i,_layers[i].size());
		for(int j=0; j<_layers[i].size(); j++) {
			printf("%u  ", _layers[i][j]);
		}
		printf ("\n");
	}
}

void BooleanCircuit::_validate_layers()
{
	_max_layer_sz = 0;
	for (gate_id_t g=1; g<_num_gates; g++){
		if(_gates[g]._layer == NO_LAYER)
			printf("gate %d has no layer!\n",g);
		assert(_gates[g]._layer != NO_LAYER);
	}
	gate_id_t sum_gates_in_layers = 0;
	for(int i=0; i<_layers.size(); i++) {
		sum_gates_in_layers += _layers[i].size();
		if(_layers[i].size() > _max_layer_sz) {
			_max_layer_sz = _layers[i].size();
		}
	}
	assert(sum_gates_in_layers == _num_gates);
}

void BooleanCircuit::EvaluateByLayerLinearly() {
	for(int i=0; i<_layers.size(); i++) {
		for (int j=0; j<_layers[i].size(); j++) {
			gate_id_t gid = _layers[i][j];
			signal_t s = _eval_gate(gid);

			Wire* out_wire = &_wires[_gates[gid]._out];
			out_wire->Sig(s);
		}
	}
}



void BooleanCircuit::Evaluate(int num_threads) {
	printf("evaluate\n");
	boost::thread_group tg;
	for (int i=0; i<num_threads; i++) {
		boost::thread* evaluator = new boost::thread(&BooleanCircuit::_eval_thread, this);
		tg.add_thread(evaluator);
	}
	tg.join_all();
//	std::cout << "_num_eval_gates: " << _num_eval_gates <<  std::endl;
//	std::cout << "_num_inserted_gates: " << _num_inserted_gates <<  std::endl;
}

void BooleanCircuit::_eval_thread() {
	printf("num_outputs = %d\n",_num_output_wires);
	while (_num_evaluated_out_wires != _num_output_wires) {
		_ready_mx.lock();
		if (_ready_gates_set.empty()) {
			_ready_mx.unlock();
			continue;
		}
		gate_id_t g = *(_ready_gates_set.begin());
		_ready_gates_set.erase(g);
		_ready_mx.unlock();

		signal_t s = _eval_gate(g); //_num_eval_gates++;
		Wire* out_wire = &_wires[_gates[g]._out];
		if(out_wire->_is_output) {
			_num_evaluated_out_wires++;
			int temp = _num_evaluated_out_wires;
			printf("evaluated = %d\n",temp);
		}

		_ready_mx.lock();
		out_wire->Sig(s);
		gate_id_t next_gate;
		for(int i=0; i < out_wire->_enters_to.size(); i++) {
			next_gate = out_wire->_enters_to[i];
			if(is_gate_ready(next_gate)) {
				_ready_gates_set.insert(next_gate);
			}
		}
		_ready_mx.unlock();
	}
}

signal_t BooleanCircuit::_eval_gate(gate_id_t g)
{
//	printf("evaluate %d\n",g);
	signal_t l = _wires[_gates[g]._left].Sig();
	signal_t r = _wires[_gates[g]._right].Sig();
	signal_t s = _gates[g]._func[2*l+r];
	return s;
}


std::string BooleanCircuit::Output()
{
	std::cout << "output:" <<std::endl;
	std::stringstream ss;
	for( int i=0; i<_num_output_wires; i++ ) {
		ss << (int)(_wires[_output_start+i].Sig());
//		_wires[_output_start+i].print(_output_start+i);
	}
	std::cout << ss.str();
	std::cout<<std::endl;
	return ss.str();
}


#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;

class Neuron{ // Abstract base class
    protected:
    float a, z; // value (z), activated value (a)
    public:
    Neuron(){a=z=0;} // default constructor
    Neuron(float x){a=z=x;} // normal constructor
    virtual ~Neuron() {}; // vertial destructor. it is empty becouse there is no dynamic things in this class
    virtual float activate(float) = 0; // pure vertial functions
    void set_a(float x){a=x;} // it is not vertial becouse same for all derived class
    void showNeuron() const{cout << a << endl;} // it is not vertial becouse same for all derived class
};
class SigmoidNeuron:public Neuron{
    public:
    SigmoidNeuron(){a=z=0;} // default constructor
    SigmoidNeuron(float x){a=x;z=x;} // normal constructor
    ~SigmoidNeuron(){} // destructor. it is empty becouse there is no dynamic things in this class
    float activate(float x){ return 1 / (1 + exp(-x)); }
};
class ReluNeuron:public Neuron{
    public:
    ReluNeuron(){a=z=0;} // default constructor
    ReluNeuron(float x){a=x;z=x;} // normal constructor
    ~ReluNeuron(){} // destructor. it is empty becouse there is no dynamic things in this class
    float activate(float x){ return fmax(0, x); }
};
class LReluNeuron:public Neuron{
    public: 
    LReluNeuron(){a=z=0;} // default constructor
    LReluNeuron(float x){a=x;z=x;} // normal constructor
    ~LReluNeuron(){} // destructor. it is empty becouse there is no dynamic things in this class
    float activate(float x){ return fmax(0.1*x, x); }
};

class  Layer{
    private:
    Neuron **neurons; // dynamic array of Neuron
    int num_of_neurons; // number of neurons in the layer
    public:
    Layer(){neurons = 0;} // default constructor
    Layer(int *&, int *&, float *&, int); // normal constructor 
    ~Layer(){
        for (int i = 0; i < num_of_neurons; i++) delete neurons[i]; // deletes elements of neurons array
        delete [] neurons; // deletes neurons array
    }
    void showLayer() const { // prints the infos of neurons
        for (int i = 0; i < num_of_neurons; i++) neurons[i]->showNeuron();
    } 
};
float next_z = 0; // a global variable to keep next z for the next layer
Layer::Layer(int *&non, int *&ton, float *&iv, int c){// non:number of neurons, ton: types of neurons, iv: initial values for layer0, c: index of which layer is crerating
    float temp_a = 0;
    num_of_neurons = non[c];
    neurons = new Neuron*[num_of_neurons]; // dynamic array of neurons is created
    if(c == 0){ // if layer0 is creating
        for (int i = 0; i < num_of_neurons; i++){
            if(ton[c] == 0) neurons[i] = new SigmoidNeuron(iv[i]);
            else if(ton[c] == 1) neurons[i] = new LReluNeuron(iv[i]);
            else if(ton[c] == 2) neurons[i] = new ReluNeuron(iv[i]);
            temp_a += iv[i]; // sum of the elements of the layer0's A matrix (A matrix is iv here)
        }
        next_z = (temp_a * 0.1) + 0.1; // z value of next layer is calculated
        return; // layer0 is created. exit from constructor
    }
    else{ // if the creating layer is not layer0
        for (int i = 0; i < num_of_neurons; i++){
            if(ton[c] == 0) neurons[i] = new SigmoidNeuron(next_z); // next_z is calculated before. it is current z for this layer now
            else if(ton[c] == 1) neurons[i] = new LReluNeuron(next_z);
            else if(ton[c] == 2) neurons[i] = new ReluNeuron(next_z);
           
            neurons[i]->set_a(neurons[i]->activate(next_z));  // constructor of Neuron assigns input to both a and z so it is necessary to set a
            temp_a += neurons[i]->activate(next_z); // sum ofelements of layer's A matrix
        }
        next_z = (temp_a * 0.1) + 0.1; // it is not current z now, z value of next layer is calculated         
        return; // layer is created, exit from constructor
    }
}
class Network{
    Layer **layers; // dynamic array of layers
    int num_of_layers;
    public:
    Network(){layers = 0;num_of_layers = 0;} // default constructor
    Network(int, int *&, int *&, float *&);
    ~Network(){
        for (int i = 0; i < num_of_layers; i++) delete layers[i]; // deletes elements of layers array
        delete [] layers; // deletes layers array
    }
    void showNetwork();
};

Network::Network(int nol, int *&non, int *&ton, float *&iv){
    layers = new Layer*[nol];
    num_of_layers = nol;
    for (int i = 0; i < num_of_layers; i++){
        layers[i] = new Layer(non, ton, iv, i);
    }
}
void Network::showNetwork(){
    for (int i = 0; i < num_of_layers; i++){
        cout << "Layer " << i << ":" << endl;
        layers[i]->showLayer();
    }
}

int main(int argc, char **argv){
    if(argc != 2){
        cerr << "Usage: ./main inputfile" << endl;
        return 1;
    }
    ifstream input(argv[1]); // file name is input
	if(!input.is_open()){ // checks if file is opened
		cerr << "Failed to open file" << endl;
        return 1;
	}

    int num_of_layers;
    input >> num_of_layers; 

    int *num_of_neurons = new int[num_of_layers];
    int *types_of_neurons = new int[num_of_layers];
    for(int i = 0; i<num_of_layers; i++){
        input>>num_of_neurons[i];// gets number of neurons for each layer
    }
    for(int i = 0; i<num_of_layers; i++){
        try{ // cheks if types of neurons are exit(they must be 0, 1 or 2)
            input>>types_of_neurons[i];
            if (types_of_neurons[i] != 0 && types_of_neurons[i] != 1 && types_of_neurons[i] != 2) throw "Unidentified activation function!";
        }
        catch(const char *result){
            cout << result << endl; // prints error message 
            delete  [] num_of_neurons;
            delete [] types_of_neurons;
            return 1; //exit
        }
    }
    
    int layer0_neuron = num_of_neurons[0];//number of neurons in layer0
    float *initial_values = new float[layer0_neuron]; // stores initial values for layer0
    for(int i = 0; i<layer0_neuron; i++){
        try { // checks if the number of initial values less than the number of neurons int layer0 (they must be equal becouse)
            if(input.eof()) throw "Input shape does not match!";
            input>>initial_values[i];
        }
        catch(const char *result){
            cout << result << endl; // prints the error message
            delete [] types_of_neurons; // all dynamic arrays are deleting in this 3 lines
            delete[] num_of_neurons;
            delete [] initial_values;
            return 1; // exit
        }
        if(i+1 == layer0_neuron){ // if taken inputs are enough
            try {if(!input.eof()) throw "Input shape does not match!";} // checks if the number of initial values more than the number of neurons
            catch(const char *result){
                cout << result << endl; // prints the error message
                delete [] types_of_neurons; // all dynamic arrays are deleting in this 3 lines
                delete[] num_of_neurons;
                delete [] initial_values;
                return 1; // exit
            }
        }
    }
    
    Network *myNetwork = new Network(num_of_layers, num_of_neurons, types_of_neurons, initial_values); // creates network
    myNetwork->showNetwork(); // prints the result 
    input.close(); // closes the input file
    delete myNetwork; // Network destructor
    delete [] types_of_neurons; // all dynamic arrays are deleting in this 3 lines
    delete[] num_of_neurons;
    delete [] initial_values;
    return 0;
}
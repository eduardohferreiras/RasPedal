#include <JackClient.H>
#include <iostream>
#include <new>
#include <cmath>
#include <string>

using namespace std;

#include <unistd.h> // for sleep

//Parametro Fuzz
float volume_fuzz;
float teto_fuzz;
void add_fuzz (jack_default_audio_sample_t *out, int j)
{
	teto_fuzz = pow(2.0 , 22.0)/pow(2.0 , 32.0); 
	volume_fuzz = 4;
			
	if(out[j] > teto_fuzz){
		out[j] = volume_fuzz*teto_fuzz;
	}
	else if(out[j] < -1*teto_fuzz){
		out[j] = -volume_fuzz*teto_fuzz;
	}
	else {
			out[j] = volume_fuzz*out[j];
	}
}

//Paramentros Delay
float *cauda_delay;
int tamanho_cauda;
int counter;
float mix;
float feedback;
void add_delay (jack_default_audio_sample_t *out, int j)
{
	cauda_delay[counter] = out[j] + feedback*cauda_delay[counter];
	counter ++;
	counter = counter % tamanho_cauda;
			
	out[j] = (out[j] + mix*cauda_delay[counter]);
}

//Parametros Tremolo
int contador_tremolo; //aprox 1 segundo entre os zeros
int velocidade_tremolo;
int subindo;
void add_tremolo (jack_default_audio_sample_t *out, int j)
{
	out[j] = out[j]*(0.1 + 1.0*contador_tremolo/velocidade_tremolo);
}

//Parametros Filtro
float *filtro;
int counter_filtro;
int tamanho_filtro;
void add_filtro (jack_default_audio_sample_t *out, int j)
{
	filtro[counter_filtro] = out[j];
	out[j] = 0.2308*out[j] + 0.1929*filtro[(counter_filtro - 1)%tamanho_filtro] + 0.1929*filtro[(counter_filtro - 2)%tamanho_filtro] + 0.2308*filtro[(counter_filtro - 3)%tamanho_filtro];
	counter_filtro ++;
	counter_filtro = counter_filtro % tamanho_filtro;
}

//Variaveis de controle

int fuzz_on;
int tremolo_on;
int delay_on;
int filtro_on;

class JackFullDuplex : public JackClient {
	
	//processAudio é chamada a cada 1024 samples (0,021 segundo)
    int processAudio(jack_nframes_t nframes) {	
		
		if (outputPorts.size()!=inputPorts.size()){
			cout<<"Different input and output port count, don't know how to copy. In port cnt="<<inputPorts.size()<<" output port count="<<outputPorts.size()<<endl;
			return -1;
		}

		jack_default_audio_sample_t *out = ( jack_default_audio_sample_t* ) jack_port_get_buffer ( outputPorts[0], nframes ); // the output sample buffer
        jack_default_audio_sample_t *in = ( jack_default_audio_sample_t* ) jack_port_get_buffer ( inputPorts[0], nframes ); // the inputs sample buffer
        
        for (uint j=0; j<nframes; j++){
			
			out[j]= 4*in[j];
			if(fuzz_on == 1) add_fuzz(out, j);
			if(tremolo_on == 1) add_tremolo(out, j);
			if(delay_on == 1) add_delay(out, j);
			if(filtro_on == 1) add_filtro(out, j);
			out[j] = 2*out[j];
		
		}
		
		//Contado base do tremolo
		if(subindo == 1){
			contador_tremolo ++;
			if (contador_tremolo == velocidade_tremolo) subindo = 0;
		}
		else {
			contador_tremolo --;
			if (contador_tremolo == 0) subindo = 1;
		}
		return 0;
    }
};

int main(int argc, char *argv[]) {
		
	//Inicialização e paramentros do Tremolo
	contador_tremolo = 0;
	velocidade_tremolo = 10;
	subindo = 1;
	
	//Inicialização e Parametros do Delay
	counter = 0;
	float delay_time = 0.3;
	mix = 0.25;
	feedback = 0.5;
	
	tamanho_cauda = (int)floor(delay_time*48000);
	cauda_delay = new float[tamanho_cauda];
	for (uint j=0; j<tamanho_cauda; j++){
		cauda_delay[j] = 0;
	}
	
	// Inicializaçãoe parametros do Filtro
	tamanho_filtro = 4;
	filtro = new float[tamanho_filtro];
	for (uint j=0; j<tamanho_filtro; j++){
		filtro[j] = 0;
	}
	counter_filtro = 0;
	
	fuzz_on = 0;
	tremolo_on = 1;
	delay_on = 0;
	filtro_on = 0;

	
	//-------------------------------------------------------------------------------------//
	
	
    JackFullDuplex jackClient; // init the full duplex jack client

    // connect to the jack server
    int res=jackClient.connect("jack full duplex client");
    if (res!=0)
        return JackDebug().evaluateError(res);

    //cout<<"Jack : sample rate set to : "<<jackClient.getSampleRate()<<" Hz"<<endl;
    //cout<<"Jack : block size set to : "<<jackClient.getBlockSize()<<" samples"<<endl;

	// create the ports
    res=jackClient.createPorts("in ", 1, "out ", 1);
    if (res!=0)
        return JackDebug().evaluateError(res);

    // start the client connecting ports to system ports
    res=jackClient.startClient(1, 1, true);
    if (res!=0)
        return JackDebug().evaluateError(res);

	

	cout<<"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
	cout<<"                                                                                  ----- WELCOME TO RASPEDAL, YOUR FREE AUDIO DSP PLATAFORM ------\n\n\n\n\n\n\n";
	cout<< "Instructions:\n\n";
	cout << " - To toggle delay: d\n";
	cout << " - To toggle tremolo: t\n";
	cout << " - To toggle fuzz: f\n";
	cout << " - To toggle filter: t\n\n";


	char comando;
	while (1)
	{
		cout << ">>>";
		comando = "-";
		cin >> comando;
		
		if(comando == 'd'){
			(delay_on ++) % 2;
			if(delay_on == 1)cout << "Delay on!\n";
			if(delay_on == 0)cout << "Delay off!\n";
		}
		if(comando == 't') tremolo_on = tremolo_on ++ % 2;
		if(comando == 'f') fuzz_on = fuzz_on ++ % 2;
		if(comando == 't') filtro_on = filtro_on ++ % 2;
		
	}
	
	
	
    return 0;
}

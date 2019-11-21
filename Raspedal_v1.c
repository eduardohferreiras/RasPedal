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
float pteto_fuzz;
float octave_on;
void add_fuzz (jack_default_audio_sample_t *out, int j)
{
	teto_fuzz = pow(2.0 , pteto_fuzz)/pow(2.0 , 32.0); 
			
	if(out[j] > teto_fuzz){
		out[j] = teto_fuzz;
	}
	else if(out[j] < -1*teto_fuzz){
		out[j] = octave_on*teto_fuzz;
	}

}

//Paramentros Delay
float *cauda_delay;
int tamanho_cauda;
float delay_time;
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
	out[j] = 0.0502*out[j] + 0.0536*filtro[(counter_filtro - 1)%tamanho_filtro] + 0.0502*filtro[(counter_filtro - 2)%tamanho_filtro]; 
	out[j] = 8*out[j];
	counter_filtro ++;
	counter_filtro = counter_filtro % tamanho_filtro;
}

//Volume global
float volGlobal;


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
			
			out[j]= in[j];
			if(fuzz_on == 1) add_fuzz(out, j);
			if(tremolo_on == 1) add_tremolo(out, j);
			if(delay_on == 1) add_delay(out, j);
			if(filtro_on == 1) add_filtro(out, j);
			out[j] = 2*volGlobal*out[j];
		
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
	
	volGlobal = 1;
	
	//Inicialização e paramentros do Fuzz
	pteto_fuzz = 25.0;
	octave_on = -1.0;
		
	//Inicialização e paramentros do Tremolo
	contador_tremolo = 0;
	velocidade_tremolo = 7;
	subindo = 1;
	
	//Inicialização e Parametros do Delay
	counter = 0;
	delay_time = 0.3;
	mix = 0.25;
	feedback = 0.5;
	
	tamanho_cauda = (int)floor(delay_time*48000);
	cauda_delay = new float[tamanho_cauda];
	for (uint j=0; j<tamanho_cauda; j++){
		cauda_delay[j] = 0;
	}
	
	// Inicializaçãoe parametros do Filtro
	tamanho_filtro = 3;
	filtro = new float[tamanho_filtro];
	for (uint j=0; j<tamanho_filtro; j++){
		filtro[j] = 0;
	}
	counter_filtro = 0;
	
	fuzz_on = 0;
	tremolo_on = 0;
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
	cout << "- FUZZ:\n";
	cout << " - To toggle fuzz: q\n";
	cout << " - To increase fuzz threshold: w\n";
	cout << " - To decrease fuzz threshold: e\n";
	cout << " - To toggle Crazy Mode: r\n";
	cout << "- DELAY:\n";
	cout << " - To toggle delay: a\n";
	cout << " - To increase delay time: s\n";
	cout << " - To decrease delay time: d\n";
	cout << " - To increase delay feedback: f\n";
	cout << " - To decrease delay feedback: g\n";
	cout << " - To increase delay mix: h\n";
	cout << " - To decrease delay mix: j\n";
	cout << "- TREMOLO:\n";
	cout << " - To toggle tremolo: z\n";
	cout << " - To increase tremolo speed: x\n";
	cout << " - To decrease tremolo speed: c\n";
	cout << "- FILTER:\n";
	cout << " - To toggle filter: l\n";
	cout << "- MASTER VOLUME:\n";
	cout << " - To decrease master volume: n\n";
	cout << " - To increase master volume: m\n\n";


	char comando;
	while (1)
	{
		cout << ">>>";
		cin >> comando;
	
		//Comandos Fuzz
		
		if(comando == 'q'){
			if(fuzz_on == 1){fuzz_on = 0; cout << "Fuzz off!\n";}
			else if(fuzz_on == 0){fuzz_on = 1; cout << "Fuzz on!\n";}
		}
		
		if(comando == 'w'){
			if(pteto_fuzz >= 31.0){cout << "Fuzz threshold is already maximized!\n";}
			else {pteto_fuzz = pteto_fuzz + 1.0; cout << "Fuzz threshold raised!\n";}
		}
		
		if(comando == 'e'){
			if(pteto_fuzz <= 1.0){cout << "Fuzz threshold is already minimized!\n";}
			else{ pteto_fuzz = pteto_fuzz - 1.0; cout << "Fuzz threshold decreased!\n";}
		}
		
		if(comando == 'r'){
			octave_on = octave_on*-1.0;
			if(octave_on > 0){cout << "Crazy Mode on!\n";}
			else if(octave_on < 0){cout << "Crazy Mode off!\n";}
		}
		
		//Comandos Delay
		
		if(comando == 'a'){
			if(delay_on == 1){delay_on = 0; cout << "Delay off!\n";}
			else if(delay_on == 0){delay_on = 1; cout << "Delay on!\n";}
		}
		
		if(comando == 's'){
			if(delay_time >= 10.0){cout << "Delay time is already maximized!\n";}
			else {
				delay_time = delay_time + 0.05; cout << "Delay time raised!\n";
				tamanho_cauda = (int)floor(delay_time*48000);
				cauda_delay = new float[tamanho_cauda];
				for (uint j=0; j<tamanho_cauda; j++){
					cauda_delay[j] = 0;
				}
			}
		}
		
		if(comando == 'd'){
			if(delay_time <= 0.051){cout << "Delay time is already minimized!\n";}
			else {
				delay_time = delay_time - 0.05; cout << "Delay time decreased!\n";
				tamanho_cauda = (int)floor(delay_time*48000);
				cauda_delay = new float[tamanho_cauda];
				for (uint j=0; j<tamanho_cauda; j++){
					cauda_delay[j] = 0;
				}
			}
		}
		if(comando == 'f'){
			if(feedback >= 0.99){cout << "Delay feedback is already maximized!\n";}
			else {feedback = feedback + 0.05; cout << "Delay feedback raised!\n";}
		}
		if(comando == 'g'){
			if(feedback <= 0.005){cout << "Delay feedback is already minimized!\n";}
			else {feedback = feedback - 0.05; cout << "Delay feedback decreased!\n";}
		}
		if(comando == 'h'){
			if(mix >= 0.99){cout << "Delay mix is already maximized!\n";}
			else {mix = mix + 0.05; cout << "Delay mix raised!\n";}
		}
		if(comando == 'j'){
			if(mix <= 0.005){cout << "Delay mix is already minimized!\n";}
			else {mix = mix - 0.05; cout << "Delay mix decreased!\n";}
		}
		
		
		//Comandos Tremolo
		
		if(comando == 'z'){
			if(tremolo_on == 1){tremolo_on = 0; cout << "Tremolo off!\n";}
			else if(tremolo_on == 0){tremolo_on = 1; cout << "Tremolo on!\n";}
		}
		if(comando == 'x'){
			if(velocidade_tremolo == 1){cout << "Tremolo speed is already maximized!\n";}
			else {velocidade_tremolo = velocidade_tremolo - 1; cout << "Tremolo speed raised!\n";}
		}
		if(comando == 'c'){
			if(velocidade_tremolo == 100){cout << "Tremolo speed is already minimized!\n";}
			else {velocidade_tremolo = velocidade_tremolo + 1; cout << "Tremolo speed decreased!\n";}
		}
		
		//Comandos Filtro
		
		if(comando == 'l'){
			if(filtro_on == 1){filtro_on = 0; cout << "Filtro off!\n";}
			else if(filtro_on == 0){filtro_on = 1; cout << "Filtro on!\n";}
		}

		//Master Volume
		if(comando == 'm'){
			if(volGlobal >= 10){cout << "Master volume is already maximized!\n";}
			else {volGlobal = volGlobal + 0.5;; cout << "Master volume increased!\n";}
		}
		if(comando == 'n'){
			if(volGlobal <= 0.01){cout << "Master volume is already minimized!\n";}
			else {volGlobal = volGlobal - 0.5;; cout << "Master volume minimized!\n";}
		}

	}
	
	
	
    return 0;
}

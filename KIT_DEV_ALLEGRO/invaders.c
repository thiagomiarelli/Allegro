#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <stdlib.h>


const float FPS = 2;  

const int SCREEN_W = 1280;
const int SCREEN_H = 720;
const int GROUND_H = 60;
const int NAVE_H = 60;
const int NAVE_W = 80;
const int FLUTACAO_NAVE = 15;
const int DIST_NAVES_H = 30;
const int DIST_NAVES_W = 30;
const int ALIEN_W = 60;
const int ALIEN_H = 30;
const int MARGIN_W = 60;
const int MARGIN_H = 40;
const int QUEDA = 30;



//type definitions
typedef struct NAVE{
	ALLEGRO_COLOR cor;
	float ponta_x;
} NAVE;

typedef struct ALIEN{
	ALLEGRO_COLOR cor;
	float canto_x;
	float canto_y;
	int exist;

} ALIEN;

//Global variables
int altura = 0;
int velocidade = 2;


//function prototypes
void drawSpace();
void criaNave(NAVE *nave);
void drawNave(NAVE *nave);
int randInt(int min, int max);
void drawAlien(ALIEN *alien);
void BuildAlienGrid(int linhas, int colunas, ALIEN alien[linhas][colunas]);
void criaAlien(ALIEN *alien, float x, float y);
void criaMatrizAliens(int linhas, int colunas, ALIEN aliens[linhas][colunas]);
void updateAlien(ALIEN *alien);
void BuildAlienGrid(int linhas, int colunas, ALIEN alien[linhas][colunas]);
void testaCanto(ALIEN *alien);


 
int main(int argc, char **argv){
	
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;
   
	//----------------------- rotinas de inicializacao ---------------------------------------
    
	//inicializa o Allegro
	if(!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}
	
    //inicializa o módulo de primitivas do Allegro
    if(!al_init_primitives_addon()){
		fprintf(stderr, "failed to initialize primitives!\n");
        return -1;
    }	
	
	//inicializa o modulo que permite carregar imagens no jogo
	if(!al_init_image_addon()){
		fprintf(stderr, "failed to initialize image module!\n");
		return -1;
	}
   
	//cria um temporizador que incrementa uma unidade a cada 1.0/FPS segundos
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}
 
	//cria uma tela com dimensoes de SCREEN_W, SCREEN_H pixels
	display = al_create_display(SCREEN_W, SCREEN_H);
	if(!display) {
		fprintf(stderr, "failed to create display!\n");
		al_destroy_timer(timer);
		return -1;
	}

	//instala o teclado
	if(!al_install_keyboard()) {
		fprintf(stderr, "failed to install keyboard!\n");
		return -1;
	}
	
	//instala o mouse
	if(!al_install_mouse()) {
		fprintf(stderr, "failed to initialize mouse!\n");
		return -1;
	}

	//inicializa o modulo allegro que carrega as fontes
	al_init_font_addon();

	//inicializa o modulo allegro que entende arquivos tff de fontes
	if(!al_init_ttf_addon()) {
		fprintf(stderr, "failed to load tff font module!\n");
		return -1;
	}
	
	//carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
    ALLEGRO_FONT *size_32 = al_load_font("arial.ttf", 32, 1);   
	if(size_32 == NULL) {
		fprintf(stderr, "font file does not exist or cannot be accessed!\n");
	}

 	//cria a fila de eventos
	event_queue = al_create_event_queue();
	if(!event_queue) {
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
	}
   


	//registra na fila os eventos de tela (ex: clicar no X na janela)
	al_register_event_source(event_queue, al_get_display_event_source(display));
	//registra na fila os eventos de tempo: quando o tempo altera de t para t+1
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	//registra na fila os eventos de teclado (ex: pressionar uma tecla)
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	//registra na fila os eventos de mouse (ex: clicar em um botao do mouse)
	al_register_event_source(event_queue, al_get_mouse_event_source());  	


	
	int playing = 1;
	//inicia o temporizador
	al_start_timer(timer);
	
	//cria a nave;
	NAVE nave;
	criaNave(&nave);

	//cria aliens
	int colunas = 7;
	int linhas = 4;
	ALIEN aliens[linhas][colunas];
	criaMatrizAliens(linhas, colunas, aliens);

	while(playing) {
		ALLEGRO_EVENT ev;
		//espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);

		//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
		if(ev.type == ALLEGRO_EVENT_TIMER) {
			//atualiza a tela (quando houver algo para mostrar)
			drawSpace();
			drawNave(&nave);
			BuildAlienGrid(linhas, colunas, aliens);
			al_flip_display();
			if(al_get_timer_count(timer)%(int)FPS == 0)
				printf("\n%d segundos se passaram...", (int)(al_get_timer_count(timer)/FPS));
		}

		//se o tipo de evento for o fechamento da tela (clique no x da janela)
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			playing = 0;
		}
		//se o tipo de evento for um clique de mouse
		else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
			printf("\nmouse clicado em: %d, %d", ev.mouse.x, ev.mouse.y);
		}
		//se o tipo de evento for um pressionar de uma tecla
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			//imprime qual tecla foi
			printf("\ncodigo tecla: %d", ev.keyboard.keycode);
		}
		// faz nave andar para direita
		else if(ev.keyboard.keycode == 83){
			if(nave.ponta_x +10 <= (SCREEN_W - NAVE_W/2)){
				nave.ponta_x += 10;
			}
			
		}

		// faz nave andar para esquerda
		else if(ev.keyboard.keycode == 82){
			if(nave.ponta_x - 10 >= (NAVE_W/2)){
				nave.ponta_x -= 10;
			}
			
		}

	} //fim do while
     
	//procedimentos de fim de jogo (fecha a tela, limpa a memoria, etc)
	
 
	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
   
 
	return 0;
}

void drawSpace(){
	al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_filled_rectangle(0, SCREEN_H - GROUND_H, SCREEN_W, SCREEN_H,
   al_map_rgb(000, 133, 211));

}

void criaNave(NAVE *nave){
	nave -> cor =  al_map_rgb(241, 170, 25);
	nave -> ponta_x = SCREEN_W/2;
}

void drawNave(NAVE *nave){
	al_draw_filled_triangle( nave->ponta_x, SCREEN_H - (NAVE_H + FLUTACAO_NAVE), nave->ponta_x + NAVE_W/2, SCREEN_H - (FLUTACAO_NAVE),
   nave->ponta_x - NAVE_W/2, SCREEN_H - (FLUTACAO_NAVE), nave -> cor);
}
// inicia o Alien
void criaAlien(ALIEN *alien, float x, float y){
	alien -> canto_x = x;
	alien -> canto_y = y;
	alien -> cor = al_map_rgb(randInt(0,255), randInt(0,255), randInt(0,255));
	alien -> exist = 1;
}
//cria uma matriz de aliens
void criaMatrizAliens(int linhas, int colunas, ALIEN aliens[linhas][colunas]){
	for (int i = 0; i < linhas; i++)
	{
		for (int j = 0; j < colunas; j++)
		{
			criaAlien(&aliens[i][j], MARGIN_W + (j * ALIEN_W), MARGIN_H + (i * ALIEN_H));
		}
		
	}
	

}

void updateAlien(ALIEN *alien){
	alien -> canto_x += velocidade;
}

void drawAlien(ALIEN *alien){
	al_draw_filled_rectangle(alien->canto_x, alien->canto_y, alien ->canto_x + ALIEN_W, alien->canto_y - ALIEN_H, alien->cor );
}
// cria grade com aliens
void BuildAlienGrid(int linhas, int colunas, ALIEN alien[linhas][colunas]){

	//faca ele ir pro lado e desca qndo necessario
	for(int i = 0; i < linhas; i++){
		for(int j = 0; j < colunas; j++){
			drawAlien(&alien[i][j]);
			printf("\npassou por aqui\n");
			updateAlien(&alien[i][j]);
			alien[i][j].canto_y = MARGIN_H + (i * ALIEN_H) + altura;
			testaCanto(&alien[i][j]);
			drawAlien(&alien[i][j]);
		}
	}
}

void testaCanto(ALIEN *alien){
	if(alien -> canto_x + ALIEN_W > 0 || alien -> canto_x < 0){
		velocidade *= -1;
		altura += QUEDA;
	}
}

int randInt(int min, int max){
	return min + rand()%(max+1-min);
}


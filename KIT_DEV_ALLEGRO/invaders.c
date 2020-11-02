#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <stdlib.h>
#include <math.h>


const float FPS = 60;  

const int SCREEN_W = 1280;
const int SCREEN_H = 720;
const int GROUND_H = 60;
const int NAVE_W = 80;
const int FLUTACAO_NAVE = 110;
const int DIST_NAVES_H = 30;
const int DIST_NAVES_W = 30;
const int ALIEN_W = 90;
const int ALIEN_H = 54;
const int MARGIN_W = 30;
const int MARGIN_H = 40;
const int QUEDA = 60;



//type definitions
typedef struct NAVE{
	ALLEGRO_COLOR cor;
	float ponta_x;
	ALLEGRO_BITMAP *skin;
	ALLEGRO_BITMAP *nave_animation[3];
} NAVE;

typedef struct ALIEN{
	ALLEGRO_COLOR cor;
	float canto_x;
	float canto_y;
	int exist;
	ALLEGRO_BITMAP *skin;
	ALLEGRO_BITMAP *alien_options[7];

} ALIEN;

typedef struct TIRO
{
	float x;
	float y;
	ALLEGRO_COLOR cor;
	int exist;
	int raio;

} TIRO;

//Global variables
	// a altura da array de aliens
	int altura = 0;
	// velocidade aliens
	float velocidade_inicial = 4;
	float velocidade = 4;
	// velocidade do tiro
	int vel_tiro = 15;
	//raio tiro
	int raio_tiro = 6;
	//pontos
	int points = 0;
	char pontos[10];
	int recorde = 0;

	//velociadde de deslocamento nave
	int velocidadeNave = 25;

	//perdeu?
	int lost_status = 0;

	//game screen
	int splashScreen = 1;
	int gameScreen = 0;
	int endScreen = 0;



//function prototypes
void drawSpace(ALLEGRO_BITMAP *background);
void criaNave(NAVE *nave);
void drawNave(NAVE *nave, int frame);
int randInt(int min, int max);
void drawAlien(ALIEN *alien);
void BuildAlienGrid(int linhas, int colunas, ALIEN alien[linhas][colunas], int seconds);
int testaCanto(ALIEN *alien);
void criaMatrizAliens(int linhas, int colunas, ALIEN aliens[linhas][colunas]);
void criaAlien(ALIEN *alien, float x, float y);
void updateTiro(TIRO *tiro);
void atirar(TIRO *tiro, NAVE *nave);
void drawTiro(TIRO *tiro);
void criaTiro(TIRO *tiro);
void colisao(TIRO *tiro, int linhas, int colunas, ALIEN alien[linhas][colunas]);
int bateu(ALIEN *alien, TIRO *tiro);
void perdeu(int linhas, int colunas, ALIEN alien[linhas][colunas], NAVE *nave);
int perdeu_nave(ALIEN *alien, NAVE *nave);
void reinicia(int linhas, int colunas, ALIEN alien[linhas][colunas]);
void testRecord(FILE *file, int recorde, int pontos);
void repopulate(int linhas, int colunas, ALIEN alien[linhas][colunas]);
void abreImagens(ALLEGRO_BITMAP *splashImage, ALLEGRO_BITMAP *background);

//debug function
void imageDebug (ALLEGRO_BITMAP *image){
	if(image == NULL){
		printf("\n image couldnt be load");
	}
}
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
    ALLEGRO_FONT *splashFont = al_load_font("fonts/ethnocentricrg.ttf", 32, 1);
	ALLEGRO_FONT *comunication = al_load_font("fonts/mensager.ttf", 25, 1);
	if(splashFont == NULL) {
		fprintf(stderr, "font file does not exist or cannot be accessed!\n");
	}
	if(comunication == NULL) {
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
   
   		printf("\n allegro was inicialized");



	//registra na fila os eventos de tela (ex: clicar no X na janela)
	al_register_event_source(event_queue, al_get_display_event_source(display));
	//registra na fila os eventos de tempo: quando o tempo altera de t para t+1
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	//registra na fila os eventos de teclado (ex: pressionar uma tecla)
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	//registra na fila os eventos de mouse (ex: clicar em um botao do mouse)
	al_register_event_source(event_queue, al_get_mouse_event_source());  	

	//abre o arquivo de recordes
	FILE *recorde_file;
    recorde_file = fopen("recorde.txt", "r");
	int recorde = 0;
	fscanf(recorde_file, "%d", &recorde);
	fclose(recorde_file);

	
	int playing = 1;
	//inicia o temporizador
	al_start_timer(timer);
	
	printf("\n trying to create images in NAVE...");

	//cria a nave;
	NAVE nave;
	criaNave(&nave);
	imageDebug (nave.nave_animation[2]);


	printf("\n SUCESS");
	printf("\n trying to create images in ALIENS...");


	//cria aliens
	int colunas = 5;
	int linhas = 4;
	ALIEN aliens[linhas][colunas];
	criaMatrizAliens(linhas, colunas, aliens);
	imageDebug (aliens[1][2].alien_options[6]);
	printf("\n trying to create images in SUCESS...");


	//cria tiro
	TIRO tiro;
	criaTiro(&tiro);


	//imagens
	ALLEGRO_BITMAP *splashImage, *background;
	abreImagens(splashImage, background);

	printf("\n static images were uploaded");

	


	while(playing) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);

		FILE *recorde_file;
    	recorde_file = fopen("recorde.txt", "r");
		fscanf(recorde_file, "%d", &recorde);
		fclose(recorde_file);

		FILE *update_record;
		update_record = fopen("recorde.txt", "w");
		testRecord(update_record, recorde, points);
		fclose(update_record);
		char recorde_char[10];
		itoa(recorde, recorde_char, 10);
		printf("\n record files uploaded");



		//se o tipo de evento for o fechamento da tela (clique no x da janela)
		printf("\n trying to run game mode");

		if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			playing = 0;
		}
		else if(splashScreen){
			printf("\n set splash screen");

			if(ev.type == ALLEGRO_EVENT_TIMER) {
				al_draw_bitmap(splashImage, 0, 0, 0);
				printf("\n bitmap drawed");
				al_flip_display();
				if(al_get_timer_count(timer)%(int)FPS == 0)
					printf("\n%d segundos se passaram...", (int)(al_get_timer_count(timer)/FPS));
			}

			else if(ev.keyboard.keycode == 19){
				splashScreen = 0;
				gameScreen = 1;
				
			}

		
		}
		// modo de jogo "jogando"
		else if(gameScreen)
		{

			//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
			if(ev.type == ALLEGRO_EVENT_TIMER) {
				itoa(points, pontos, 10);
				drawSpace(background);
				al_draw_text(comunication, al_map_rgb(255, 255, 255), 50, SCREEN_H - 35, 0, pontos);
				drawNave(&nave, (int)(al_get_timer_count(timer)));
				BuildAlienGrid(linhas, colunas, aliens, (int)(al_get_timer_count(timer)/2));
				updateTiro(&tiro);
				colisao(&tiro, linhas, colunas, aliens);
				repopulate(linhas, colunas, aliens);
				perdeu(linhas, colunas, aliens, &nave);
				al_flip_display();
				if(al_get_timer_count(timer)%(int)FPS == 0)
					printf("\n%d segundos se passaram...", (int)(al_get_timer_count(timer)/FPS));
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
				if(nave.ponta_x + velocidadeNave <= (SCREEN_W - NAVE_W/2)){
					nave.ponta_x += velocidadeNave;
				}
				
			}

			// faz nave andar para esquerda
			else if(ev.keyboard.keycode == 82){
				if(nave.ponta_x - velocidadeNave >= (NAVE_W/2)){
					nave.ponta_x -= velocidadeNave;
				}
				
			}
	
			// atira espaco
			else if(ev.keyboard.keycode == 75){
				atirar(&tiro, &nave);
				
			}

		} 
		else if(endScreen)
		{

			if(ev.type == ALLEGRO_EVENT_TIMER) {
				
				al_clear_to_color(al_map_rgb(0,0,0));
				al_draw_text(splashFont, al_map_rgb(200, 0, 30), SCREEN_W/3, SCREEN_H/2, 0, "Looser");
				al_draw_text(comunication, al_map_rgb(200, 0, 30), SCREEN_W/3, SCREEN_H/2 + 80, 0, pontos);
				al_draw_text(comunication, al_map_rgb(200, 0, 30), SCREEN_W/3, SCREEN_H/2 + 120, 0, recorde_char);
				

				al_flip_display();

				if(al_get_timer_count(timer)%(int)FPS == 0)
					printf("\n%d segundos se passaram...", (int)(al_get_timer_count(timer)/FPS));
			}

			else if(ev.keyboard.keycode == 75){
				endScreen = 0;
				gameScreen = 1;
				points = 0;
				reinicia(linhas, colunas, aliens);


				
			}
		}

	} //fim do while
     
	//procedimentos de fim de jogo (fecha a tela, limpa a memoria, etc)
	
 
	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
   
 
	return 0;
}

void abreImagens(ALLEGRO_BITMAP *splashImage, ALLEGRO_BITMAP *background){
	// background jogo
	splashImage = al_load_bitmap("images/splashscreen.jpg");
	background = al_load_bitmap("images/background1.jpg");
}

void drawSpace(ALLEGRO_BITMAP *background){
	al_clear_to_color(al_map_rgb(0, 0, 0));
	al_draw_bitmap(background, 0, 0, 0);


    

}

void criaNave(NAVE *nave){
	nave -> cor =  al_map_rgb(241, 170, 25);
	nave -> ponta_x = SCREEN_W/2; 
	nave -> nave_animation[0] = al_load_bitmap("images/nave1_1.png");
	nave -> nave_animation[1] = al_load_bitmap("images/nave1_2.png");
	nave -> nave_animation[2] = al_load_bitmap("images/nave1_3.png");
}

void drawNave(NAVE *nave, int frame){

   
	nave -> skin = nave -> nave_animation[(int)((frame % (int)FPS)/20)];
	al_draw_bitmap(nave -> skin, nave -> ponta_x - NAVE_W/2, SCREEN_H - FLUTACAO_NAVE, 0);


}
// inicia o Alien
void criaAlien(ALIEN *alien, float x, float y){
	alien -> canto_x = x;
	alien -> canto_y = y;
	alien -> cor = al_map_rgb(randInt(0,255), randInt(0,255), randInt(0,255));
	alien -> exist = 1;
	alien -> alien_options[0] = al_load_bitmap("images/fb_ship.png");
	alien -> alien_options[1] = al_load_bitmap("images/am_ship.png");
	alien -> alien_options[2] = al_load_bitmap("images/gg_ship.png");
	alien -> alien_options[3] = al_load_bitmap("images/ig_ship.png");
	alien -> alien_options[4] = al_load_bitmap("images/ms_ship.png");
	alien -> alien_options[5] = al_load_bitmap("images/tk_ship.png");
	alien -> alien_options[6] = al_load_bitmap("images/tt_ship.png");
	int skin_number = randInt(0, 6);
	alien -> skin = alien -> alien_options[skin_number];

}
//cria uma matriz de aliens
void criaMatrizAliens(int linhas, int colunas, ALIEN aliens[linhas][colunas]){
	for (int i = 0; i < linhas; i++)
	{
		for (int j = 0; j < colunas; j++)
		{
			criaAlien(&aliens[i][j], MARGIN_W + (j * (ALIEN_W + DIST_NAVES_W)), MARGIN_H + (i * (ALIEN_H + DIST_NAVES_H)));
		}
		
	}
	

}

void drawAlien(ALIEN *alien){
	al_draw_bitmap(alien -> skin, alien->canto_x, alien->canto_y, 0);
}
// cria grade com aliens
void BuildAlienGrid(int linhas, int colunas, ALIEN alien[linhas][colunas], int seconds){
	int i, j;
	for(i = 0; i < linhas; i++){
		for(j = 0; j < colunas; j++){
			alien[i][j].canto_x += velocidade;
			alien[i][j].canto_y = MARGIN_H + (i * (ALIEN_H + DIST_NAVES_H)) + altura;
			if(alien[i][j].exist == 1){
				drawAlien(&alien[i][j]);
			}

			if(testaCanto(&alien[i][j])){
				velocidade *= -1;
				altura += QUEDA;
				//quebra nested loops
				i = j = linhas + colunas;
				printf("\nDeu como verdade o testa canto");
			}
		}
	}

}

int randInt(int min, int max){
	return min + rand()%(max+1-min);
}

int testaCanto(ALIEN *alien){
	if((alien -> canto_x + ALIEN_W > SCREEN_W || alien -> canto_x < 0) && (alien -> exist)){
		return 1;
	}
	return 0;
}

void drawTiro(TIRO *tiro){
	al_draw_filled_circle(tiro -> x, tiro -> y, tiro -> raio, tiro->cor);
}

void atirar(TIRO *tiro, NAVE *nave){
	//se houver outro tiro no ar, nao atire
	if(tiro -> exist == 1){
		printf("\n tiro bloqueado");
		return;
	}
	// coloca o tiro na ponta da nave
	printf("\n atirou");
	tiro -> exist = 1;
	tiro -> x = nave -> ponta_x;
	tiro -> y = (SCREEN_H - FLUTACAO_NAVE);
	tiro -> raio = raio_tiro; 
}

void updateTiro(TIRO *tiro){
	tiro -> y -= vel_tiro;
	if(tiro -> exist == 1){
		drawTiro(tiro);
	}
}

void criaTiro(TIRO *tiro){
	tiro -> x = 0;
	tiro -> y = 0;
	tiro -> cor = al_map_rgb(255, 255, 255);
	tiro -> exist = 0;
}

void colisao(TIRO *tiro, int linhas, int colunas, ALIEN alien[linhas][colunas]){
	//se bater no topo da tela
	if(tiro -> y < 0){
		tiro -> exist = 0;
	}
	int i, j;
	for (i = 0; i < linhas; i++)
	{
		for (j = 0; j < colunas; j++)
		{
			if(bateu(&alien[i][j], tiro)){
				alien[i][j].exist = 0;
				tiro -> exist = 0;
				points++;
				//quebra o loop
				i = j = i + j;
				
			}
		}
		
	}
	
}

int bateu(ALIEN *alien, TIRO *tiro){
	int vertical_hit = 0;
	int horizontal_hit = 0;
	int existance = 0;
	if(tiro -> y + tiro -> raio - (alien -> canto_y + ALIEN_H) <= 6 ){
		horizontal_hit = 1;
	}

	if(alien -> exist == 1 && tiro -> exist == 1){
		existance = 1;
	}

	if(tiro -> x >= alien -> canto_x && tiro -> x <= alien -> canto_x + ALIEN_W){
		vertical_hit = 1;
	}

	if(vertical_hit && horizontal_hit && existance){
		return 1; 
	}
	return 0;

}

int perdeu_nave(ALIEN *alien, NAVE *nave){
	int lado = 0;
	int topo = 0;
	if (abs(alien -> canto_x - nave -> ponta_x) < 3 || abs((alien -> canto_x + ALIEN_W) - nave -> ponta_x) < 3)
		lado = 1;
	if((SCREEN_H - (FLUTACAO_NAVE)) - (alien -> canto_y + ALIEN_H) < 2)
		topo = 1;
	if(topo && lado && alien ->exist){
		return 1;
	}
		return 0;
	
	
}

void perdeu(int linhas, int colunas, ALIEN alien[linhas][colunas], NAVE *nave){
	//se bater na base
	int i, j;
	for (i = 0; i < linhas; i++)
	{
		for (j = 0; j < colunas; j++)
		{
			if(((alien[i][j].canto_y + ALIEN_H > SCREEN_H - (float)(GROUND_H-4)) && alien[i][j].exist) || perdeu_nave(&alien[i][j], nave)){
				gameScreen = 0;
				endScreen = 1;
				return;
			}
		}
		
	}	
}

void reinicia(int linhas, int colunas, ALIEN alien[linhas][colunas]){
	velocidade = velocidade_inicial;
	altura = 0;
	criaMatrizAliens(linhas, colunas, alien);

}

void testRecord(FILE *file, int recorde, int pontos){
	if(pontos > recorde){
		fprintf(file, "%d", pontos);
	} else {
		fprintf(file, "%d", recorde);
	}

}

void repopulate(int linhas, int colunas, ALIEN alien[linhas][colunas]){
	int sobrou = 0;
	for (int i = 0; i < linhas; i++)
	{
		for (int j = 0; j < colunas; j++)
		{
			if(alien[i][j].exist){
				sobrou = 1;
				i = j = i+j;
			}
		}
		
	}
	
	if(!sobrou){
	velocidade *= 1.2;
	altura = 0;
	criaMatrizAliens(linhas, colunas, alien);
	}

}

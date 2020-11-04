/* --------------
       THIS COMMIT IS THE FINAL VERSION WITH STORE. EVERYTHING IS WORKING
	   													----------------------- */



#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdlib.h>
#include <math.h>


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
	int velocidadeTiro;
	int pontuacao_equivalente;

} ALIEN;

typedef struct TIRO
{
	float x;
	float y;
	ALLEGRO_COLOR cor;
	int exist;
	int raio;

} TIRO;



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
void atirar(TIRO *tiro, NAVE *nave, ALLEGRO_SAMPLE *tiro_sound);
void drawTiro(TIRO *tiro);
void criaTiro(TIRO *tiro);
void colisao(TIRO *tiro, int linhas, int colunas, ALIEN alien[linhas][colunas], ALLEGRO_SAMPLE *hit_audio);
int bateu(ALIEN *alien, TIRO *tiro);
void perdeu(int linhas, int colunas, ALIEN alien[linhas][colunas], NAVE *nave, int recorde, int *frase_sorteada);
int perdeu_nave(ALIEN *alien, NAVE *nave);
void reinicia(int linhas, int colunas, ALIEN alien[linhas][colunas]);
void testRecord(FILE *file, int recorde, int pontos);
void repopulate(int linhas, int colunas, ALIEN alien[linhas][colunas]);
void atualizaMoedas(FILE *moedas_file, int valor, char modo);
void drawLoja(ALLEGRO_BITMAP *background);
void getPowerupData(FILE *powerups);
int compraPowerup(FILE *powerups, char tipo);
int buttonClick(int mouse_x, int mouse_y, int x1, int y1, int x2, int y2);
void preenchePowerUp();
void criaTiroAlien(TIRO *tiro);
void alienAtira(TIRO *tiro, ALIEN *alien);
void algumAtira(TIRO *tiro, int linhas, int colunas, ALIEN aliens[linhas][colunas], int timer);
void updateTiroAlien(TIRO *tiro);
void colisaoTiroAlien(TIRO *tiro, NAVE *nave, int *frase_sorteada);


// constants

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
const int POWERUP_PRICE = 250;
const int HORIZONTAL_POWERUP_VALUE = 5;

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
	char gameMode = 's';

	//loja
	int moedas = 0;
	FILE *moedas_file = NULL;
	FILE *powerup_file;
	char moedas_char[7];
	int horizontal_powerup = 1;
	int tiro_powerup = 1;

	//frases finais
	const char *frases_finais[10] = {"Em Deus nós acreditamos, todos os outros devem trazer dados.", "É... Todo mundo já teve um nude vazado né...", "Eu começaria a pesquisar essas coisas na aba anônima", "Não é nada que o Snowden não tenha visto antes", "Da próxima vez bate na porta.", "Você ia postar isso no twitter de qualquer jeito"};



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
	
	// Inicializa o audio //
    if (!al_install_audio()){
    fprintf(stderr, "Falha ao inicializar áudio.\n");
    return false;
  }

    if (!al_init_acodec_addon()){
    fprintf(stderr, "Falha ao inicializar codecs de áudio.\n");
    return false;
  }

    if (!al_reserve_samples(1)){
    fprintf(stderr, "Falha ao alocar canais de áudio.\n");
    return false;
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










	//----------------------- FONTES ---------------------------------------

	//carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
    ALLEGRO_FONT *splashFont = al_load_font("fonts/8bit.ttf", 40, 1);
	ALLEGRO_FONT *comunication = al_load_font("fonts/mensager.ttf", 28, 1);
	ALLEGRO_FONT *message = al_load_font("fonts/mensager.ttf", 16, 1);

	if(splashFont == NULL) {
		fprintf(stderr, "font file does not exist or cannot be accessed!\n");
	}
	if(comunication == NULL) {
		fprintf(stderr, "font file does not exist or cannot be accessed!\n");
	}
	
	
	
	//----------------------- CRIANDO VARIAVEIS DE JOGO ---------------------------------------

	//cria a nave;
	NAVE nave;
	criaNave(&nave);

	//cria aliens
	int colunas = 5;
	int linhas = 4;
	ALIEN aliens[linhas][colunas];
	criaMatrizAliens(linhas, colunas, aliens);

	//cria tiros
	TIRO tiro;
	criaTiro(&tiro);

	TIRO tiro_alien;
	criaTiroAlien(&tiro_alien);
	

	//numero sorteado
	int frase_sorteada = randInt(0, 5);

	
	//----------------------- IMAGENS ---------------------------------------

	//imagens
	ALLEGRO_BITMAP *splashImage, *background, *new_record, *end_game, *fundo_loja, *purchase_button, *purchase_not_available;
	splashImage = al_load_bitmap("images/splashscreen.jpg");
	background = al_load_bitmap("images/background1.jpg");
	new_record = al_load_bitmap("images/endscreen.jpg");
	end_game = al_load_bitmap("images/endscreen_nr.jpg");
	fundo_loja = al_load_bitmap("images/LOJA.jpg");
	purchase_button = al_load_bitmap("images/pricing.png");
	purchase_not_available = al_load_bitmap("images/no-money.png");

	printf("\n static images were uploaded");

	


	//----------------------- AUDIOS ---------------------------------------
	//audio samples carregados
	 ALLEGRO_SAMPLE *tiro_sound = al_load_sample("soundtrack/tiro.ogg");
  	 ALLEGRO_SAMPLE *hit_sound = al_load_sample("soundtrack/hit.ogg");
  	 ALLEGRO_SAMPLE *begin_sound = al_load_sample("soundtrack/developers.ogg");
 	 ALLEGRO_SAMPLE *record_sound = al_load_sample("soundtrack/tiro.mp3");
	
	//background music
	 ALLEGRO_SAMPLE *theme_song = al_load_sample("soundtrack/game_theme.ogg");
	 ALLEGRO_SAMPLE_INSTANCE *theme_instance = al_create_sample_instance(theme_song);
	 al_set_sample_instance_playmode(theme_instance, ALLEGRO_PLAYMODE_LOOP);
	 al_set_sample_instance_gain(theme_instance, 0.6);
	 al_attach_sample_instance_to_mixer(theme_instance, al_get_default_mixer()); 
	 al_play_sample_instance(theme_instance);
	//number of channels playing simoutainously
	 al_reserve_samples(5);
	




	//----------------------- LOOP PRINCIPAL ---------------------------------------


	int playing = 1;
	//inicia o temporizador
	al_start_timer(timer);

	while(playing) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);

		/* ---------------> ARQUIVOS DE RECORDE E ATUALIZACAO <--------------- */
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
		itoa(moedas, moedas_char, 10);


		/* ---------------> pega cliques de mouse - debbuging <--------------- */

		if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
				printf("\nmouse clicado em: %d, %d", ev.mouse.x, ev.mouse.y);
			}


		/* ---------------> FECHAR JANELA <--------------- */

		if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			playing = 0;
		}

		/* ---------------> SPLASH SCREEN <--------------- */
		else if(gameMode == 's'){

			if(ev.type == ALLEGRO_EVENT_TIMER) {
				al_draw_bitmap(splashImage, 0, 0, 0);
				al_flip_display();
				if(al_get_timer_count(timer)%(int)FPS == 0)
					printf("\n%d segundos se passaram...", (int)(al_get_timer_count(timer)/FPS));
			}

			else if(ev.keyboard.keycode == 19){
				gameMode = 'g';
				al_play_sample(begin_sound, 1.0, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);

				
			}

			else if(ev.keyboard.keycode == 16){
				gameMode = 'p';				
			}

		
		}
		

		/* ---------------> JOGATINA <--------------- */
		else if(gameMode == 'g')
		{

			//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
			if(ev.type == ALLEGRO_EVENT_TIMER) {
				itoa(points, pontos, 10);
				drawSpace(background);
				al_draw_text(comunication, al_map_rgb(255, 255, 255), 50, SCREEN_H - 35, 0, pontos);
				drawNave(&nave, (int)(al_get_timer_count(timer)));
				BuildAlienGrid(linhas, colunas, aliens, al_get_timer_count(timer));
				updateTiro(&tiro);

				//checagem de colisao
				colisao(&tiro, linhas, colunas, aliens, hit_sound);
				repopulate(linhas, colunas, aliens);
				perdeu(linhas, colunas, aliens, &nave, recorde, &frase_sorteada);

				//mecanismo de tiro do alien
				algumAtira(&tiro_alien, linhas, colunas, aliens, al_get_timer_count(timer));
				updateTiroAlien(&tiro_alien);
				colisaoTiroAlien(&tiro_alien, &nave, &frase_sorteada);


				al_flip_display();
				if(al_get_timer_count(timer)%(int)FPS == 0)
					printf("\n%d segundos se passaram...", (int)(al_get_timer_count(timer)/FPS));
			}

			//se o tipo de evento for um pressionar de uma tecla
			else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
				//imprime qual tecla foi
				printf("\ncodigo tecla: %d", ev.keyboard.keycode);
			}
			// faz nave andar para direita
			else if(ev.keyboard.keycode == 83){
				if(nave.ponta_x + velocidadeNave <= (SCREEN_W - NAVE_W/2)){
					nave.ponta_x += (velocidadeNave + (HORIZONTAL_POWERUP_VALUE * horizontal_powerup)) ;
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
				atirar(&tiro, &nave, tiro_sound);
				
			}

		} 
		
		/* ---------------> TELA FINAL SEM RECORDE <--------------- */
		else if(gameMode == 'e')
		{
			// criacao da tela
			if(ev.type == ALLEGRO_EVENT_TIMER) {
				al_clear_to_color(al_map_rgb(0,0,0));
				// carrega a imagem de fundo
				al_draw_bitmap(end_game, 0, 0, 0);
				al_draw_text(splashFont, al_map_rgb(255, 022, 0), 642, 185, ALLEGRO_ALIGN_CENTER, pontos);
				al_draw_text(comunication, al_map_rgb(255, 255, 255), 570, 435, 0, recorde_char);
				al_draw_text(comunication, al_map_rgb(255, 255, 255), 720, 435, 0, moedas_char);
				al_draw_text(message, al_map_rgb(15, 15, 15), 642, 326, ALLEGRO_ALIGN_CENTER, frases_finais[frase_sorteada]);

				

				al_flip_display();

				if(al_get_timer_count(timer)%(int)FPS == 0)
					printf("\n%d segundos se passaram...", (int)(al_get_timer_count(timer)/FPS));
			}

			else if(ev.keyboard.keycode == 19){
				gameMode = 'g';
				points = 0;
				al_play_sample(begin_sound, 1.0, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
				reinicia(linhas, colunas, aliens);


				
			}

			else if(ev.keyboard.keycode == 16){
				gameMode = 'p';				
			}
		}

		/* ---------------> TELA FINAL RECORDE <--------------- */
		else if(gameMode == 'r')
		{

			if(ev.type == ALLEGRO_EVENT_TIMER) {
				
				al_clear_to_color(al_map_rgb(0,0,0));
				al_draw_bitmap(new_record, 0, 0, 0);
				al_draw_text(splashFont, al_map_rgb(255, 157, 0), 642, 185, ALLEGRO_ALIGN_CENTER, pontos);
				al_draw_text(comunication, al_map_rgb(15, 15, 15), 730, 430, 0, moedas_char);
				al_draw_text(comunication, al_map_rgb(15, 15, 15), 558, 430, 0, recorde_char);
				

				al_flip_display();

				if(al_get_timer_count(timer)%(int)FPS == 0)
					printf("\n%d segundos se passaram...", (int)(al_get_timer_count(timer)/FPS));
			}

			else if(ev.keyboard.keycode == 19){
				gameMode = 'g';
				points = 0;
				al_play_sample(begin_sound, 1.0, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
				reinicia(linhas, colunas, aliens);
			}

			else if(ev.keyboard.keycode == 16){
				gameMode = 'p';				
			}
		}
		/* ---------------> TELA LOJA POWERUPS <--------------- */
		else if(gameMode == 'p')
		{	
			if(ev.type == ALLEGRO_EVENT_TIMER) {
				getPowerupData(powerup_file);
				drawLoja(fundo_loja);
				al_draw_text(comunication, al_map_rgb(255, 255, 255), 1074, 122, 0, moedas_char);
				preenchePowerUp();
				if(moedas >= POWERUP_PRICE){
					al_draw_bitmap(purchase_button, 986, 329, 0);
					al_draw_bitmap(purchase_button, 986, 543, 0);
				} else {
					al_draw_bitmap(purchase_not_available, 986, 329, 0);
					al_draw_bitmap(purchase_not_available, 986, 543, 0);
				}
				al_flip_display();
			}
			else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
				if(buttonClick(ev.mouse.x, ev.mouse.y, 984, 327, 1098, 378) && (horizontal_powerup <= 6)){
						printf("\n botao superior clicado");
						compraPowerup(powerup_file, 'h');
				}

				if(buttonClick(ev.mouse.x, ev.mouse.y, 984, 529, 1098, 620) && (tiro_powerup <= 6)){
						printf("\n botao superior clicado");
						compraPowerup(powerup_file, 't');
				}
			}
			else if(ev.keyboard.keycode == 19){
				gameMode = 'g';
				points = 0;
				al_play_sample(begin_sound, 1.0, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
				reinicia(linhas, colunas, aliens);

			}
	
		}
		
	} //fim do while
     

	//procedimentos de fim de jogo (fecha a tela, limpa a memoria, etc)
	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);

	// destroy bitmaps
	al_destroy_bitmap(splashImage);
	al_destroy_bitmap(background);
	al_destroy_bitmap(new_record);
	al_destroy_bitmap(end_game);
	al_destroy_bitmap(fundo_loja);

	for (int i = 0; i < 3; i++)
	{
		al_destroy_bitmap(nave.nave_animation[i]);
	}
   
   //destroy samples
   al_destroy_sample(hit_sound);
   al_destroy_sample(tiro_sound);
   al_destroy_sample(record_sound);
   al_destroy_sample(theme_song);
   al_destroy_sample_instance(theme_instance);
 
	return 0;
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
	alien -> exist = 1;
	alien -> alien_options[0] = al_load_bitmap("images/fb_ship.png");
	alien -> alien_options[1] = al_load_bitmap("images/am_ship.png");
	alien -> alien_options[2] = al_load_bitmap("images/gg_ship.png");
	alien -> alien_options[3] = al_load_bitmap("images/ig_ship.png");
	alien -> alien_options[4] = al_load_bitmap("images/ms_ship.png");
	alien -> alien_options[5] = al_load_bitmap("images/tk_ship.png");
	alien -> alien_options[6] = al_load_bitmap("images/tt_ship.png");
	int skin_number = randInt(0, 6);
	alien -> pontuacao_equivalente = skin_number + 1;
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

void atirar(TIRO *tiro, NAVE *nave, ALLEGRO_SAMPLE *tiro_sound){
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
	al_play_sample(tiro_sound, 1.0, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
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

void colisao(TIRO *tiro, int linhas, int colunas, ALIEN alien[linhas][colunas], ALLEGRO_SAMPLE *hit_audio){
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
				points += alien[i][j].pontuacao_equivalente;
				//quebra o loop
				i = j = i + j;
				al_play_sample(hit_audio, 1.0, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);

				
			}
		}
		
	}
	
}

int bateu(ALIEN *alien, TIRO *tiro){
	int vertical_hit = 0;
	int horizontal_hit = 0;
	int existance = 0;
	if((tiro -> y - tiro -> raio) <= (alien -> canto_y + ALIEN_H) && ((tiro -> y + tiro -> raio) > (alien -> canto_y + 2*ALIEN_H/3))){
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

void perdeu(int linhas, int colunas, ALIEN alien[linhas][colunas], NAVE *nave, int recorde, int *frase_sorteada){
	//se bater na base
	int i, j;
	for (i = 0; i < linhas; i++)
	{
		for (j = 0; j < colunas; j++)
		{
			if(((alien[i][j].canto_y + ALIEN_H > SCREEN_H && alien[i][j].exist) || perdeu_nave(&alien[i][j], nave))){
				atualizaMoedas(moedas_file, points, 'e');
				srand(points);
				*frase_sorteada = randInt(0, 5);
				printf("\nfrase sorteada %d ", *frase_sorteada);


				if(points >= recorde){
					gameMode = 'r';
					printf("\n declaracao de derrota");
					return;
				} else {
					gameMode = 'e';
					return;
				}
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

void atualizaMoedas(FILE *moedas_file, int valor, char modo){
	moedas_file = fopen("moedas.txt", "r");
	fscanf(moedas_file, "%d", &moedas);
	fclose(moedas_file);
	if(modo == 'e'){
		moedas_file = fopen("moedas.txt", "w");
		fprintf(moedas_file, "%d", moedas + valor);
		fclose(moedas_file);
	} else {
		moedas_file = fopen("moedas.txt", "w");
		fprintf(moedas_file, "%d", moedas - valor);
		fclose(moedas_file);
	}

	moedas_file = fopen("moedas.txt", "r");
	fscanf(moedas_file, "%d", &moedas);
	fclose(moedas_file);

}

void drawLoja(ALLEGRO_BITMAP *background){
	atualizaMoedas(moedas_file, 0, 'e');
	al_draw_bitmap(background, 0, 0, 0);
}

void getPowerupData(FILE *powerups){
	powerups = fopen("powerups.txt", "r");
	//quantidade de powerup de velocidade horizontal comprada
	fscanf(powerups, "%d", &horizontal_powerup);
	//quantidade de velocidade de tiro vertical comprado 
	fscanf(powerups, "%d", &tiro_powerup);
	fclose(powerups);
}

int compraPowerup(FILE *powerups, char tipo){
	if(moedas >= POWERUP_PRICE){
	powerups = fopen("powerups.txt", "w");
	atualizaMoedas(moedas_file, POWERUP_PRICE, 's');
	if(tipo == 'h')
		fprintf(powerups, "%d %d", horizontal_powerup + 1, tiro_powerup);
	else if(tipo == 't')
		fprintf(powerups, "%d %d", horizontal_powerup, tiro_powerup + 1);
	
	}
	fclose(powerups);
	return 1;
}

int buttonClick(int mouse_x, int mouse_y, int x1, int y1, int x2, int y2){
	if(mouse_y < y2 && mouse_y > y1){
		return 1;
	}
	
	return 0;
}

void preenchePowerUp(){
	for (int i = 0; i < horizontal_powerup - 1; i++)
	{
		al_draw_filled_rectangle(610 + (62*i), 324, 670 + (62*i), 360,
		al_map_rgb(255, 255, 255));
	}

	for (int i = 0; i < tiro_powerup - 1; i++)
	{
		al_draw_filled_rectangle(604+ (62*i), 545, 664 + (62*i), 581,
		al_map_rgb(255, 255, 255));
	}
}

void alienAtira(TIRO *tiro, ALIEN *alien){
	//se houver outro tiro no ar, nao atire
	tiro -> exist = 1;
	tiro -> x = alien -> canto_x + ALIEN_W/2;
	tiro -> y = alien -> canto_y + ALIEN_H;
	tiro -> raio = raio_tiro; 
}

void algumAtira(TIRO *tiro, int linhas, int colunas, ALIEN aliens[linhas][colunas], int timer){

	srand(points);
	int i = randInt(0, linhas - 1);
	int j = randInt(0, colunas - 1);
	if(aliens[i][j].exist && (timer%(int)(1.5*FPS) == 0)){
		alienAtira(tiro, &aliens[i][j]);
		tiro -> exist = 1;
	}
}

void updateTiroAlien(TIRO *tiro){
	tiro -> y += vel_tiro*0.5;
	if(tiro -> exist == 1){
		drawTiro(tiro);
	}
	if(tiro -> y > SCREEN_H)
		tiro -> exist = 0;
}

void criaTiroAlien(TIRO *tiro){
		tiro -> x = -20;
		tiro -> y = 0;
		tiro -> cor = al_map_rgb(255, 100, 0);
		tiro -> exist = 0;
	
}

void colisaoTiroAlien(TIRO *tiro, NAVE *nave, int *frase_sorteada){
	//se bater no topo da tela
	if(tiro -> y < 0){
		tiro -> exist = 0;
	}
	
	if((tiro -> y > FLUTACAO_NAVE && (tiro -> x > (nave -> ponta_x - NAVE_W/2) && tiro -> x < (nave -> ponta_x + NAVE_W/2))) && tiro ->exist){

		atualizaMoedas(moedas_file, points, 'e');
				srand(points);
				*frase_sorteada = randInt(0, 5);
				printf("\nfrase sorteada %d ", *frase_sorteada);


				if(points >= recorde){
					gameMode = 'r';
					printf("\n declaracao de derrota");
					return;
				} else {
					gameMode = 'e';
					return;
				}

	}
	
}
/*
#################################################
#	Universidade de Brasilia					#
#	Instituto de Ciencias Exatas				#
#	Departamento de Ciencia da Computacao		#
#	Algoritmos e Programação de Computadores 	#
#	1/2019										#
#	Aluno(a): Pedro de Tôrres Maschio			#
#	Matricula: 19/0018763						#
#	Turma: A									#
#	Versão do compilador: C99					#
#	Descricao: Um divertido jogo estilo Puzzle  #
#    Bobble. São 5 letras com cores diferentes, #
#   o objetivo e unir 4 letras de cores         #
#   iguais e fazer pontos!                      #
#                                               #
#################################################
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>


/* Expressões que mudam a cor dos caracteres no terminal. */
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define RESET "\x1b[0m"
#define RED "\x1b[31m"

/* Valores que serao requeridos na funcao limpaTela() */
#ifdef _WIN32
    #define CLEAR "cls"
#else
    #define CLEAR "clear"
#endif

#ifndef _WIN32
// Retorna 1 caso alguma tecla seja pressionada, 0 em caso contrário.
int kbhit(){    
    struct termios oldt, newt;
    int ch, oldf;tcgetattr(STDIN_FILENO,&oldt);
    newt = oldt;newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if(ch != EOF){
        ungetc(ch,stdin);
        return 1;
    }
    return 0;
}
// Espera alguma tecla ser pressionada e a retorna.
int getch(void) {
    int ch;
    struct termios oldt;
    struct termios newt;
    tcgetattr(STDIN_FILENO,&oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
    }
#else
    #include <conio.h>
#endif



/* Variáveis Globais */
char tabuleiro[10][17];
int altura=9, largura = 16, velocidade=60, perdeu = 0, pontuacao = 0;
double co_angular = 0;
char p; 



/*  Exibe o tabuleiro, rotacionando-o para que a linha fique
    posicionada corretamente, além de adicionar cores às
    letras.
*/
void exibeTabuleiro() {
    int i, j;

    printf("Pontos: %d\n\n", pontuacao);

    for(j = 8; j >= 0; j--) {
        for(i = 0; i < largura; i++) {
            if(tabuleiro[i][j] == 'A')
                printf(RED "%c" RESET, tabuleiro[i][j]);
            else if(tabuleiro[i][j] == 'B')
                printf(BLUE "%c" RESET, tabuleiro[i][j]);
            else if(tabuleiro[i][j] == 'C')
                printf(GREEN "%c" RESET, tabuleiro[i][j]);
            else if(tabuleiro[i][j] == 'D')
                printf(YELLOW "%c" RESET, tabuleiro[i][j]);
            else if(tabuleiro[i][j] == 'E')
                printf(CYAN "%c" RESET, tabuleiro[i][j]);
            else
                printf("%c", tabuleiro[i][j]);
        }
        printf("\n");
    }
}

// Adiciona as bordas do tabuleiro, bem como o caractere aleatório.
void preencheTabuleiro() {
    int i, j;
    for(i = 0; i < largura; i++) {
        for(j = 0; j < altura; j++) {
            if(i == 0 || j == 0 || j == 8 || i == 15)
                tabuleiro[i][j] = '#';
            else if(j == 1 && i == 8)
                tabuleiro[i][j] = p;
            else
                tabuleiro[i][j] = ' ';
        }
    }
}

// Limpa o tabuleiro para, posteriormente, atualizar a mira.
void limpaTabuleiro() {
    int i, j;

    memset(tabuleiro, ' ', sizeof(tabuleiro));
    preencheTabuleiro();
}

// Calcula e adiciona a mira ao tabuleiro.
void calculaMira() {

    limpaTabuleiro();

    int i, j;
    double aux;

    for(i = 0; i < largura; i++) {
        for(j = 2; j < altura-1; j++) {
            aux = co_angular*j + 8;

            int y = (int) aux;
                
            // Necessário para evitar que a mira extrapole-se.
            if(y > 0 && y < 15) {
                tabuleiro[y][j] = '-';
            }   
        }
    }
}

// Limpa a tela do terminal.
void limpaTela() {
    system(CLEAR);
}

// Cria o caractere aleatório da base do tabuleiro.
void criachar() {
    p = 'A' + rand() % 5;

}

// "Movimenta" o caractere pela mira até o seu destino final.
void atira() {
    int i, j;
    char anterior = p;
    criachar();
    preencheTabuleiro();
    calculaMira();

    for(i = 0; i < largura; i++) {
        for(j = 0; j < altura; j++) {

            if(tabuleiro[i][j] == '-') {
                tabuleiro[i][j] = anterior;
            }
        }
    }
}

int menuMain() {
    int opcao;
    
    printf("1 - Jogar\n");
    printf("2 - Instrucoes\n");
    printf("3 - Configuracoes\n");
    printf("4 - Ranking\n");
    printf("5 - Sair\n");
    printf("Informe o numero correspondente a opcao desejada: ");

    scanf("%d", &opcao);
    
    return opcao;
}


int main() {
    srand(time(0));
    criachar();

    limpaTela();
    preencheTabuleiro();
    calculaMira();
    exibeTabuleiro();
    while(1) {
        if(kbhit()) {
            limpaTela();
            int a = getchar();
            if(a == 97) {
                co_angular -= 0.14;
                calculaMira();
            } else if(a == 100) {
                co_angular += 0.14;
                calculaMira();
            } else if(a == 32) {
                atira();
            }
            exibeTabuleiro(); 
            usleep(500);
        }
    }

}
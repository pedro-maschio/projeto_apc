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
    int ch, oldf;
    tcgetattr(STDIN_FILENO,&oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
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
int yfinal = 1;
double co_angular = 0;
char p; 



/*  Exibe o tabuleiro, rotacionando-o para que a linha fique
    posicionada corretamente, além de adicionar cores às
    letras.
*/
void exibeTabuleiro() {
    int i, j;

    printf("Pontos: %d\n\n", pontuacao);

    for(j = altura-1; j >= 0; j--) {
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
            if(i == 0 || j == 0 || j == (altura-1) || i == (largura-1))
                tabuleiro[i][j] = '#';
            else if(j == 1 && i == (altura-1))
                tabuleiro[i][j] = p;
            else if(tabuleiro[i][j] != 'A' && tabuleiro[i][j] != 'B' && tabuleiro[i][j] != 'C' && tabuleiro[i][j] != 'D' && tabuleiro[i][j] != 'E')
                tabuleiro[i][j] = ' ';
        }
    }
}

// Limpa o tabuleiro para, posteriormente, atualizar a mira.
void limpaTabuleiro() {
    int i, j;

    for(i = 0; i < largura; i++) {
        for(j = 0; j < altura; j++) {
            if(tabuleiro[i][j] == '-')
                tabuleiro[i][j] = ' ';
        }
    }
}

// Calcula e adiciona a mira ao tabuleiro.
void calculaMira(int yfinal) { 

    limpaTabuleiro();

    int i, j;
    double aux;

    for(i = 0; i < largura; i++) {
        for(j = 2; j < altura-yfinal; j++) {
            aux = co_angular*j + (largura/2);

            int y = (int) aux;
                
            /* Necessário para evitar que a mira extrapole-se. E
            que ela sobrescreva os caracteres atirados.
            */
            if(tabuleiro[y][j] == 'A' || tabuleiro[y][j] == 'B' || tabuleiro[y][j] == 'C' || tabuleiro[y][j] == 'D' || tabuleiro[y][j] == 'E' || tabuleiro[i][j] == '*') 
                break;
            if(y > 0 && y < (largura-1)) {
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

void verificaPonto(char peca, int localx, int localy) {
    int i, j, soma, flag = 0;

    localy -=3;
    localx -=3;

    // i*j = 49, submatriz 7*7 em torno do ponto de tiro.
    for(i = localy; i < localy+3; i++) {
        for(j = localx; i < localx+3; j++) {
            if(localx == peca && flag == 0) {
                soma++;
            } else 
                flag = 1;
        }
    }
}

// "Movimenta" o caractere pela mira ate o seu destino final.
void atira() {
    int i, j, flag = 0, localx, localy;
    char anterior = p;
    // Altera a mira da base do tabuleiro.
    criachar();

    preencheTabuleiro();
    calculaMira(yfinal);

    for(i = 0; i < altura; i++) {
        for(j = 0; j < largura; j++) {
            if(tabuleiro[j][i] == '-') {
                if(flag == 0) {
                    tabuleiro[j][i] = anterior;
                    if(i > 2)
                        tabuleiro[localx][localy] = '-';
                    localy = i;
                    localx = j;
                    flag = 1;
                } else {
                    tabuleiro[localx][localy] = '-';
                    tabuleiro[j][i] = anterior;
                    localy = i;
                    localx = j;
                    flag = 0;
                }
            }
        }
        exibeTabuleiro();
        usleep(20000);
        limpaTela(); 
    }
}

// Desce a parece superior.
void desceParede() {
    int i, j, a;
    char linhaAux[17];
    for(i = altura-1; i >= 2; i--) {
        for(j = largura-1, a = 0; j >= 0; j--, a++) {
            linhaAux[a] = tabuleiro[j][i];
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

// Marca os 20 segundos para descer a parede superior do tabuleiro.
int temporizador(time_t inicio) {
    time_t fim;

    time(&fim);

    int diferenca = difftime(fim, inicio);


    return diferenca;
}




int main() {
    int flag = 0;
    time_t inicio;
    time(&inicio);

    srand(time(0));
    criachar();

    limpaTela();
    preencheTabuleiro();
    calculaMira(yfinal);
    exibeTabuleiro();


    // a = 97, d = 100, espaco= 100, f = 102
    while(1) {
        if(kbhit()) {
            limpaTela();
            int tecla = getch();
            if(tecla == 97) {
                co_angular -= 0.14;
                calculaMira(yfinal);
            } else if(tecla == 100) {
                co_angular += 0.14;
                calculaMira(yfinal);
            } else if(tecla == 32) {
                atira();
            } else if(tecla == 102) {
                desceParede();
            }
            exibeTabuleiro(); 
            usleep(10000);
        }

        /*if((temporizador(inicio)+1) % 20 == 0 && flag == 0) {
            printf("aaa");
            flag == 1;
        } else if((temporizador(inicio)+1) == 1 && flag == 1)
            flag = 0;
            */
    }

}
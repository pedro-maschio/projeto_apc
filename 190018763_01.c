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
#   Bobble. São 5 letras com cores diferentes,  #
#   o objetivo e unir 4 letras iguais e fazer   #
#   pontos!                                     #
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


/* Expressoes que mudam a cor dos caracteres no terminal. */
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
// Retorna 1 caso alguma tecla seja pressionada, 0 em caso contrario.
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



/* Variaveis Globais */
char tabuleiro[10][17], pecasc[10][17];
int altura = 9, largura = 16, velocidade=60, perdeu = 0, pontuacao = 0, conectadas = 0;
double co_angular = 0;
char p; 


/* Para remover os warnings, devido ao fato da funcao ser chamada
   antes de sua declaracao */
void menuMain();


// Exibe e adiciona cores ao tabuleiro.
void exibeTabuleiro() {
    int i, j;

    printf("Pontos: %d\nConectadas: %d\n\n", pontuacao, conectadas);

    for(i = 0; i < altura; i++) {
        for(j = 0; j < largura; j++) {
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

// Adiciona as bordas do tabuleiro, bem como o caractere aleatorio.
void preencheTabuleiro() {
    int i, j;
    for(i = 0; i < altura; i++) {
        for(j = 0; j < largura; j++) {
            if(i == 0 || j == 0 || j == (largura-1) || i == (altura-1))
                tabuleiro[i][j] = '#';
            else if(i != altura-2 || j != largura/2)
                tabuleiro[i][j] = ' ';
        }
    }
}

// Adiciona a peca aleatoria a base do tabuleiro.
void adicionaPecaBase(){
    tabuleiro[altura-2][largura/2] = p;
}

// Limpa o tabuleiro para, posteriormente, atualizar a mira.
void limpaTabuleiro() {
    int i, j;

    for(i = 0; i < altura; i++) {
        for(j = 0; j < largura; j++) {
            if(tabuleiro[i][j] == '-')
                tabuleiro[i][j] = ' ';
        }
    }
}

// Calcula e adiciona a mira ao tabuleiro.
void calculaMira() { 

    int j, y, flag = 0;
    double aux;

    limpaTabuleiro();

    for(j = 2; j < largura; j++) {
        aux = co_angular*j + (largura/2)+1;

        y = (int) aux;
                
        /* Necessario para evitar que a mira extrapole-se. E
        que ela sobrescreva os caracteres atirados.
        */
        if(tabuleiro[altura-j][-y] != ' ') 
            flag = 1;
        if(y < 17 && flag != 1)
            tabuleiro[altura-j][-y] = '-';
              
    }
        
    
}

// Limpa a tela do terminal.
void limpaTela() {
    system(CLEAR);
}

// Cria o caractere aleatorio da base do tabuleiro.
void criachar() {
    p = 'A' + rand() % 5;

}

// Cria o efeito de explosao das pecas do tabuleiro.
void explode() {
    int i, j, count = 0;;

    for(i = 0; i < altura; i++) {
        for(j = 0; j < largura; j++) {
            if(pecasc[i][j] == '*')
                count++;
        }
    }
    if(count >= 4) {
        for(i = 1; i < altura-3; i++) {
            for(j = 1; j < largura-1; j++) {
                if(pecasc[i][j] == '*')
                    tabuleiro[i][j] = '*';
            }
        }
        limpaTela();
        exibeTabuleiro();
        usleep(5000);

        for(i = 1; i < altura-3; i++) {
            for(j = 1; j < largura-1; j++) {
                if(tabuleiro[i][j] == '*')
                    tabuleiro[i][j] = ' ';
            }
        }
        limpaTela();
        /* Necessario para a mira preencher o local no qual estavam as pecas
        que foram explodidas.*/ 
        calculaMira();

        pontuacao += 10*count;
    }
}

// Verifica as conexoes das pecas.
void verifica(char peca, int i, int j) {
    if(tabuleiro[i][j] == peca) {
        pecasc[i][j] = '*';
        if(tabuleiro[i][j+1] == peca) {
            conectadas++;
            verifica(peca, i, j+1);
        }
        if(tabuleiro[i-1][j] == peca) {
            conectadas++;
            verifica(peca, i-1, j);
        }
        if(tabuleiro[i-1][j-1] == peca) {
            conectadas++;
            verifica(peca, i-1, j-1);
        }
        if(tabuleiro[i-1][j-2] == peca) {
            conectadas++;
            verifica(peca, i-1, j-2);
        }
    }
}

/* Tambem verifica as conexoes das pecas. Por algum motivo, ao
manter as duas em uma unica gerava segmentation fault.
*/
void verifica2(char peca, int i, int j) {
    if(tabuleiro[i][j] == peca) {
        pecasc[i][j] = '*';
        if(tabuleiro[i+1][j+1] == peca) {
            conectadas++;
            verifica2(peca, i+1, j+1);
        }
        if(tabuleiro[i][j-1] == peca) {
            conectadas++;
            verifica2(peca, i, j-1);
        }
    }
}  

// "Movimenta" o caractere pela mira ate o seu destino final.
void atira() {
    int i, j, flag = 0, localx, localy;
    char anterior = p;

    // Altera a letra da base do tabuleiro.
    criachar();
    adicionaPecaBase();
    calculaMira();

    for(i = altura-2; i >= 0; i--) {
        for(j = largura-1; j >= 0; j--) {
            if(tabuleiro[i][j] == '-') {
                if(flag == 0) {
                    tabuleiro[i][j] = anterior;
                    if(i < altura-4)
                        tabuleiro[localy][localx] = '-';
                    localy = i;
                    localx = j;
                    flag = 1;
                } else {
                    tabuleiro[localy][localx] = '-';
                    tabuleiro[i][j] = anterior;
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

    // Verificacoes de conexoes.
    conectadas = 0;
    memset(pecasc, ' ', sizeof(pecasc));
    verifica(anterior, localy, localx);
    verifica2(anterior, localy, localx);
    if(conectadas != 0)
        pecasc[localy][localx] = '*';
    explode();

}

// PARA TESTE APENAS, REMOVER
void exibeAsteriscos() {
    int i, j;

    limpaTela();
    printf("Pontos: %d\nConectadas: %d\n\n", pontuacao, conectadas);

    for(i = 0; i < altura; i++) {
        for(j = 0; j < largura; j++) {
            printf("%c", pecasc[i][j]);
        }
        printf("\n");
    }
    usleep(500000);
}

// Exibe as instrucoes do jogo.
void instrucoes() {
    limpaTela();
    printf(BLUE "\t\tSUPER EXPLODE LETRAS\n" RESET);
    printf("COMANDOS: \n");
    printf("\t Pressione a para mover a mira para a esquerda.\n");
    printf("\t Pressione d para mover a mira para a direita.\n");
    printf("\t Pressione espaco para atirar na posicao desejada.\n\n");
    printf("O jogador ganha uma quantidade de pontos ao formar 4 ou mais pecas de\n");
    printf("mesmo tipo. A quantidade de pontos eh proporcional ao numero de pecas conectadas.\n\n");
    printf("Aperte Enter para voltar ao Menu Principal...");

    getchar();

    if(getchar() == 10)
        menuMain();
}

// Funcao a ser implementada futuramente.
void configuracoes() {
    limpaTela();
    printf(BLUE "\t\tSUPER EXPLODE LETRAS\n" RESET);
    printf("Funcao ainda nao implementada.\n\n");
    printf("Aperte Enter para voltar ao Menu Principal...");

    getchar();

    if(getchar() == 10)
        menuMain();
}

// Funcao a ser implementada futuramente.
void ranking() {
    limpaTela();
    printf(BLUE "\t\tSUPER EXPLODE LETRAS\n" RESET);
    printf("Funcao ainda nao implementada.\n\n");
    printf("Aperte Enter para voltar ao Menu Principal...");

    getchar();

    if(getchar() == 10)
        menuMain();
}

// Desce a parede superior do tabuleiro.
void desceTabuleiro() {
    int i, j;

    char aux[10][17];

    for(i = 0; i < altura; i++) {
        for(j = 1; j < largura-1; j++) {
            aux[i][j] = tabuleiro[i][j];
        }
    }

    for(i = 1; i < altura-2; i++) {
        for(j = 1; j < largura-1; j++) {
            tabuleiro[i][j] = aux[i-1][j];
        }
    }
    
}

// Marca os 20 segundos para descer a parede superior do tabuleiro.
int temporizador(time_t inicio) {
    time_t fim;

    time(&fim);

    int diferenca = difftime(fim, inicio);

    
    return diferenca;
}

// Verifica se ha peca em uma das tres posicoes horizontais na frente da mira.
int pecaFrente() {
    if(tabuleiro[altura-3][largura/2] != '-' && tabuleiro[altura-3][largura/2] != ' ' || 
    tabuleiro[altura-3][largura/2-1] != ' ' && tabuleiro[altura-3][largura/2-1] != '-' ||
    tabuleiro[altura-3][largura/2+1] != ' ' && tabuleiro[altura-3][largura/2+1] != '-')
        return 1;
    return 0;
}

// Tela exibida apos encerrar a partida
void despedida() {
    limpaTela();
    printf("Obrigado por jogar!!\n");
    printf("Sua pontuacao final foi de: %d pontos.\n", pontuacao);
}

// Exibe inicialmente o tabuleiro e gerencia todas as funcoes de jogo 
void iniciaJogo() {
    
    int contatempo = 0, segundos = 0, tecla;

    time_t inicio;
    time(&inicio);


    limpaTela();
    preencheTabuleiro();
    adicionaPecaBase();
    calculaMira();
    exibeTabuleiro();
    
    do {
        
        if(kbhit()) {
            limpaTela();

            tecla = getch();
            
            /* Verifica o co_angular para evitar que ele estrapole-se.
               a = 97, A = 65, d = 100, D = 68, espaco = 32.
            */
            if((tecla == 97 || tecla == 65) && co_angular <= 3.51) {
                co_angular += 0.13;
                calculaMira();
            } else if((tecla == 100 || tecla == 68) && co_angular >= -2.6) {
                co_angular -= 0.13;
                calculaMira();
            } else if(tecla == 32) {
                atira();
            } else if(tecla == 102)
                exibeAsteriscos();

            exibeTabuleiro(); 
            usleep(10000);
        }

        // Desce a parede superior apos 20 segundos.
        limpaTela();
        contatempo = temporizador(inicio);
        if(contatempo == 20) {
            desceTabuleiro();
            time(&inicio);
            segundos++;
        }
        if(segundos > 5 || pecaFrente())
            perdeu = 1;

        exibeTabuleiro();

        usleep(10000);


    } while(perdeu != 1);
    
    despedida();
}

// Exibe a tela de boas vindas ao jogador
void boasVindas() {
    limpaTela();

    printf(BLUE "\n\n\t\tSUPER EXPLODE LETRAS\n\n" RESET);


    printf("Pressione qualquer tecla para continuar.\n");

    getchar();    
}

// Exibe o menu principal do jogo
void menuMain() {
    limpaTela();
    
    int opcao;

    boasVindas();
    limpaTela();

    printf("1 - Jogar\n");
    printf("2 - Instrucoes\n");
    printf("3 - Configuracoes\n");
    printf("4 - Ranking\n");
    printf("5 - Sair\n");
    printf("Informe o numero correspondente a opcao desejada: ");

    scanf("%d", &opcao);
    
    switch(opcao) {
        case 1:
            iniciaJogo();
            break;
        case 2: 
            instrucoes();
            break;
        case 3:
            configuracoes();
            break;
        case 4:
            ranking();
            break;
        case 5:
            limpaTela();
            printf("\nObrigado por jogar!\n");
            break;
    }
}

int main() {
    srand(time(0));
    criachar();

    menuMain();
}
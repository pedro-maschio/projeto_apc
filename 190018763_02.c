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
char tabuleiro[10][17];
char pecasc[10][17]; // matriz auxiliar para criar a explosao.
char nickname[11];
int altura = 9, largura = 16, velocidade=60, pontuacao = 0, conectadas, perdeu = 1 ;
int tempoDescida = 20, pontua = 4, ranq = 0; // numero de pecas para explodir
double co_angular = 0;
char p; 
FILE *replay, *config, *arqRank;

typedef struct {
    char nick[11];
    int score;
} Player;

/* Cabecalhos de funcoes para remover os warnings. */
void menuMain();
void despedida();
void configuracoes();


// Exibe e adiciona cores ao tabuleiro.
void exibeTabuleiro() {
    int i, j;

    printf("Pontos: %d\n\n", pontuacao);

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
void adicionaPecaBase(int repete){
    perdeu = 0; // caso o ja se tenha jogado uma partida, reseta o perdeu
    if(repete != 2)
        tabuleiro[altura-2][largura/2] = p;
    else if(repete == 2) {
        if((fscanf(replay, "%c", &p)) != EOF) {
            tabuleiro[altura-2][largura/2] = p;
        } else {
            perdeu = 1;
        }
    }
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

// Exibe o nome do jogo com as cores.
void nomeJogo() {
    printf("\n\n\t\t");
    printf(RED "S" RESET);
    printf(BLUE "U" RESET);
    printf(YELLOW "P" RESET);
    printf(GREEN "E" RESET);
    printf(MAGENTA "R" RESET);
    printf(CYAN " E" RESET);
    printf(RED "X" RESET);
    printf(BLUE "P" RESET);
    printf(YELLOW "L" RESET);
    printf(GREEN "O" RESET);
    printf(MAGENTA"D" RESET);
    printf(CYAN "E" RESET);
    printf(RED " L" RESET);
    printf(BLUE "E" RESET);
    printf(YELLOW "T" RESET);
    printf(GREEN "R" RESET);
    printf(MAGENTA "A" RESET);
    printf(CYAN "S" RESET);
    printf("\n\n");
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
    if(count >= pontua) {
        for(i = 1; i < altura-3; i++) {
            for(j = 1; j < largura-1; j++) {
                if(pecasc[i][j] == '*')
                    tabuleiro[i][j] = '*';
            }
        }
        limpaTela();
        exibeTabuleiro();
        usleep(velocidade*500);

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

// Salva a peca atirada no arquivo de texto
void salvaPeca(char peca) {
    fprintf(replay, "%c", peca);
}

// Para evitar segmentation fault na recursividade.
int ehSeguro(int i, int j) {
    if(i >= 0 && j >= 0 && i <= altura && j <= largura)
        return 1;
    return 0;
}

// Verifica as conexoes das pecas.
void verifica(char peca, int i, int j) {
    if(tabuleiro[i][j] == peca && ehSeguro(i, j)) {
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
    if(tabuleiro[i][j] == peca && ehSeguro(i, j)) {
        pecasc[i][j] = '*';
        if(tabuleiro[i+1][j+1] == peca) {
            conectadas++;
            verifica2(peca, i+1, j+1);
        }
        if(tabuleiro[i][j-1] == peca) {
            conectadas++;
            verifica2(peca, i, j-1);
        }
        if(tabuleiro[i+1][j] == peca) {
            conectadas++;
            verifica2(peca, i+1, j);
        }
        if(tabuleiro[i+1][j+2] == peca) {
            conectadas++;
            verifica2(peca, i+1, j+2);
        }
        if(tabuleiro[i+1][j-1] == peca) {
            conectadas++;
            verifica2(peca, i+1, j-1);
        }
    }
}  

// "Movimenta" o caractere pela mira ate o seu destino final.
void atira(int repete) {
    int i, j, flag = 0, localx, localy;
    char anterior;
    
    
    /* Somente gera um char aleatorio se a as opcoes selecionadas
    nao forem a 2 (que le o arquivo de replay).
    */
    anterior = p;
    if(repete != 2) {
        criachar();
    }
    
    adicionaPecaBase(repete);
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
        usleep(velocidade*400);
        limpaTela(); 
    }

    // Verificacoes de conexoes.
    conectadas = 0;
    memset(pecasc, ' ', sizeof(pecasc));
    verifica(anterior, localy, localx);
    verifica2(anterior, localy, localx);
    explode();

    if(repete == 1)
        salvaPeca(anterior); // para salvar no arquivo
}

// Exibe as instrucoes do jogo.
void instrucoes() {
    limpaTela();
    nomeJogo();
    printf("COMANDOS: \n");
    printf("\t Pressione a para mover a mira para a esquerda.\n");
    printf("\t Pressione d para mover a mira para a direita.\n");
    printf("\t Pressione espaco para atirar na posicao desejada.\n\n");
    printf("O jogador ganha uma quantidade de pontos ao formar 4 ou mais pecas de\n");
    printf("mesmo tipo. A quantidade de pontos eh proporcional ao numero de pecas conectadas.\n\n");
    printf("Aperte Enter para voltar ao Menu Principal...");

    getch();

    if(getch() == 10)
        menuMain();
}

// Tela para definicao das opcoes do usuario, que serao gravadas no arquivo de configuracoes.
void configPecas() {
    limpaTela();
    
    if(ranq != 1) {
        do {
            limpaTela();
            nomeJogo();
            printf("Quantidade de pecas para pontuar (min 3 && max 10): ");
            scanf("%d", &pontua);
        } while(pontua < 3 || pontua > 10);
    
        do {
            limpaTela();
            nomeJogo();
            printf("Tempo de descida do tabuleiro (min 10 && max 35): ");
            scanf("%d", &tempoDescida);
        } while(tempoDescida < 10 || tempoDescida > 35);


        if((config = fopen("configuracoes.txt", "w")) == NULL) {
            printf("Erro ao abrir o arquivo de configuracoes!");
        } else {
            fprintf(config, "%d %d", pontua, tempoDescida);
            fclose(config);
        }
        printf("\nPressione <Enter> para voltar.\n");
        getchar();   
        configuracoes();   
    } 
    else {
        limpaTela();
        nomeJogo();
        printf("Modo ranqueado ATIVADO. Utilizando configuracoes padrao:\n");
        printf("-> 5 pecas conectadas para pontuar;\n");
        printf("-> 15 segundos para o tabuleiro descer.\n"); 

        printf("\nPressione <Enter> para voltar.\n");
        getchar();   
    }
}

/* Esta funcao eh chamada ao inicio de cada jogo para abrir o arquivo
de texto gerado previamente para definir as configuracoes  da partida
atual*/
void verificaConfig() {
    if((config = fopen("configuracoes.txt", "r")) != NULL && ranq != 1) {
        fscanf(config, "%d %d", &pontua, &tempoDescida);
        fclose(config);
    }
}

// Altera e exibe as configuracoes que serao utilizadas na partida ranqueada ao jogador.
void ranqueado() {
    limpaTela();
    nomeJogo();
    printf("Modo ranqueado ATIVADO. Configuracoes de jogo atualizadas: \n");
    printf("-> 5 pecas conectadas para pontuar;\n");
    printf("-> 15 segundos para o tabuleiro descer.\n");

    pontua = 5;
    tempoDescida = 15;
    ranq = 1; // partida ranqueada

    printf("\nPressione <Enter> para voltar.");
    
    getchar();

}

// Solicita o nickname ao usuario e o informa que o modo ranqueado esta ativado.
void verificaRanq() {
    if(ranq) {
        do {
            limpaTela();
            nomeJogo();
            printf("MODO RANQUEADO ATIVADO\n\n");
            printf("Informe o seu nickname: ");
            scanf("%s", nickname);

            if(strlen(nickname) < 1 || strlen(nickname) > 10) {
                printf("O nickname deve ter mais que um caractere e menos que 10.");
            }
        } while(strlen(nickname) < 1 || strlen(nickname) > 10);
    }
}

void ordena(Player player[]) {
    int i, j;

    for(i = 0; i < 10; i++) {
        for(j = 0; j < 9; j++) {
            if(player[j].score > player[j+1].score) {
                int aux = player[j+1].score;
                player[j+1].score = player[j].score;
                player[j].score = aux;
            }
        }
    }
}

void registerMatch() {
    Player player;
    Player matriz[10];
    int i = 0;

    if((arqRank = fopen("ranking.bin", "rb")) != NULL) {
        while(fread(&player, sizeof(Player), 1, arqRank)) {
            sprintf(matriz[i].nick, "%s", player.nick);
            matriz[i].score = player.score;
            i++;
        }
        ordena(matriz);
        

        fclose(arqRank);
    } else {
        arqRank = fopen("ranking.bin", "wb");
        sprintf(player.nick, "%s", nickname);
        player.score = pontuacao;
        fwrite(&player, sizeof(Player), 1, arqRank);
        fclose(arqRank);
    }
    
}

// Exibe o nome e a pontuacao dos 10 melhores players.
void listarRanking() {
    limpaTela();
    nomeJogo();

    Player player;
    if((arqRank = fopen("ranking.bin", "rb")) == NULL) 
        printf("Ainda nao foi gerado um arquivo de ranking!");
    else {
        printf("NOME\t\tPONTUACAO\n");
        while(fread(&player, sizeof(Player), 1, arqRank)) { 
            printf("%s\t\t", player.nick);
            printf("%d", player.score);
            printf("\n");
        }
        fclose(arqRank);
    }
    printf("\nPressione <Enter> para voltar ao Menu Principal.");
    getchar();
    if(getch())
        menuMain();
}

// Implementa as configuracoes do jogo.
void configuracoes() {
    limpaTela();
    int opcao;

    nomeJogo();
    printf(GREEN "1. Pecas\n" RESET);
    printf(BLUE "2. Ativar Modo Ranqueado\n" RESET);
    printf(RED "3. Voltar\n" RESET);
    printf(YELLOW "Opcao escolhida: " RESET);

    scanf("%d", &opcao);

    if(opcao == 1) 
        configPecas();
    else if(opcao == 2)
        ranqueado();

    if(getch() == 10)
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
    
    // Comeca a copiar os elementos "por cima" do auxiliar.
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

//  Verifica se ha peca em uma das tres posicoes horizontais na frente da mira.
int pecaFrente() {
    if((tabuleiro[altura-3][largura/2] != '-' && tabuleiro[altura-3][largura/2] != ' ') || 
    (tabuleiro[altura-3][largura/2-1] != ' ' && tabuleiro[altura-3][largura/2-1] != '-') ||
    (tabuleiro[altura-3][largura/2+1] != ' ' && tabuleiro[altura-3][largura/2+1] != '-'))
        return 1;
    return 0;
}

// Tela exibida apos encerrar a partida
void despedida() {
    limpaTela();
    if(replay != NULL)
        fclose(replay);
    nomeJogo();
    printf("Obrigado por jogar!!\n");
    printf("Sua pontuacao final foi de: %d pontos.\n\n", pontuacao);

    if(ranq)
        registerMatch();

    printf("Aperte <Enter> para voltar ao menu principal.");
    if(getch() == 10) {
        menuMain();
    }

    
}

// Exibe inicialmente o tabuleiro e gerencia todas as funcoes de jogo 
void iniciaJogo(int repete) {
    
    int contatempo = 0, tecla;
    co_angular = 0; // tambem zera a mira, para centraliza-la
    pontuacao = 0; // caso o jogador queira jogar de novo, zera a pontuacao anterior.

    time_t inicio;
    time(&inicio);


    limpaTela();
    preencheTabuleiro();
    adicionaPecaBase(repete);
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
                atira(repete);
                
            } 

            exibeTabuleiro(); 
            usleep(velocidade*400);
           
        }
        // Desce a parede superior.
        limpaTela();
        contatempo = temporizador(inicio);
        if(contatempo == tempoDescida) {
            desceTabuleiro();
            time(&inicio);
        }
        exibeTabuleiro();
        usleep(velocidade*400);

        // Verifica se ha pecas na frente da letra da base para encerrar a partida.
        if(pecaFrente())
            perdeu = 1 ;
    } while(perdeu != 1);
    
    despedida();
}

// Exibe a tela de boas vindas ao jogador
void boasVindas() {
    limpaTela();

    nomeJogo();

    printf("\nPressione qualquer <Enter> para continuar.\n");

    getch();    
}

// Cria o arquivo .txt de replay para armazenar o estado do jogo
void criarReplay() {

    limpaTela();
    char filename[100];

    nomeJogo();
    printf("Informe o nome do arquivo a ser criado (adicione .txt ao final): ");
    scanf("%s", filename);

    if((replay = fopen(filename, "w")) == NULL) {
        printf("Erro ao criar o arquivo de texto!");
    }
    iniciaJogo(1);
}

// Le o arquivo .txt para iniciar o jogo
void usarReplay() {
    limpaTela();
    char filename[100];

    nomeJogo();
    printf("Informe o nome do arquivo a ser aberto (adicione .txt ao final): ");
    scanf("%s", filename);

    if((replay = fopen(filename, "r")) == NULL) {
        printf("Erro ao abrir o arquivo de texto!\n");
        menuMain();
    } else {
        iniciaJogo(2);
    }
}

// Exibe as opcoes de jogo em relacao aos arquivos
void opcoesJogo() {
    limpaTela();
    int opcao;

    nomeJogo();
    printf(GREEN "1. Criar arquivo de replay.\n" RESET);
    printf(BLUE "2. Utilizar arquivo de replay.\n" RESET);
    printf(RED "3. Jogar sem utilizar arquivo de replay.\n" RESET);
    printf(YELLOW "Informe como voce deseja jogar: " RESET);

    scanf("%d", &opcao);

    if(opcao == 1)
        criarReplay();
    else if(opcao == 2)
        usarReplay();
    else if(opcao == 3)
        iniciaJogo(3);

}

// Exibe o menu principal do jogo
void menuMain() {
    limpaTela();
    
    int opcao;

    limpaTela();

    printf(YELLOW "1 - Jogar\n" RESET);
    printf(GREEN "2 - Instrucoes\n" RESET);
    printf(RED "3 - Configuracoes\n" RESET);
    printf(BLUE "4 - Ranking\n" RESET);
    printf(MAGENTA "5 - Sair\n\n" RESET);
    printf(CYAN "Informe o numero correspondente a opcao desejada: " RESET);

    scanf("%d", &opcao);
    
    switch(opcao) {
        case 1:
            verificaRanq();
            verificaConfig();
            opcoesJogo();
            break;
        case 2: 
            instrucoes();
            break;
        case 3:
            configuracoes();
            break;
        case 4:
            listarRanking();
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

    boasVindas();
    menuMain();
}
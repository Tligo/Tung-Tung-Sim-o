#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cartas.h"
#include "interface.h"


/* Converte o caractere introduzido pelo jogador num índice de coluna (0-9).
   '1'-'9' → colunas 0-8 ; '0' → coluna 9. Devolve -1 se inválido. */
static int charParaColuna(char c) {
    if (c == '0')              return 9;
    if (c >= '1' && c <= '9') return c - '1';
    return -1;
}


/* Lê um único caractere do stdin e limpa o restante buffer de entrada.
   Devolve 1 se a leitura teve sucesso, 0 caso contrário. */
static int lerOpcao(char *opcao) {
    int lido = (scanf(" %c", opcao) == 1);
    while (getchar() != '\n'); /* descarta qualquer caractere extra até ao ENTER */
    return lido;
}


/* Executa o movimento entre as colunas e remove eventuais sequências completas.
   Se o movimento for inválido, informa o jogador e aguarda confirmação. */
static void executarMovimento(EstadoJogo *e, int origem, int destino) {
    int ok = moverSequencia(e, origem, destino);
    if (ok) {
        removerSequencias(e);
    } else {
        printf("\n[!] Jogada invalida! (Pressione ENTER para continuar)");
        getchar();
    }
}


/* Lê as colunas de origem e destino e tenta realizar o movimento.
   Valida os índices antes de chamar a lógica do jogo. */
static void processarMovimento(EstadoJogo *e) {
    char o_c = 0, d_c = 0;

    printf("Coluna de origem  (1-9, 0=C10): ");
    lerOpcao(&o_c);
    printf("Coluna de destino (1-9, 0=C10): ");
    lerOpcao(&d_c);

    int o = charParaColuna(o_c);
    int d = charParaColuna(d_c);

    if (o < 0 || d < 0 || o == d) {
        printf("\n[!] Coluna invalida! (Pressione ENTER para continuar)");
        getchar();
    } else {
        executarMovimento(e, o, d);
    }
}


/* Interpreta a opção lida e executa a acção correspondente.
   Devolve 0 se o jogador pediu para sair, 1 caso contrário. */
static int processarOpcao(EstadoJogo *e, char opcao) {
    if      (opcao == 'q' || opcao == 'Q') return 0;
    else if (opcao == 'm' || opcao == 'M') processarMovimento(e);
    else if (opcao == 'd' || opcao == 'D') calcularMelhorJogada(e);
    else {
        printf("\n[!] Opcao invalida! (Pressione ENTER para continuar)");
        getchar();
    }
    return 1;
}


/* Mostra o menu, lê a opção e processa-a.
   Devolve 0 se o jogador pediu para sair, 1 caso contrário. */
static int executarTurno(EstadoJogo *e) {
    printf("\n[m] Mover sequencia | [d] Dica | [q] Sair\nA tua jogada: ");
    char opcao = 0;
    if (lerOpcao(&opcao) == 0) return 1;
    return processarOpcao(e, opcao);
}


/* Verifica vitória/derrota e imprime a mensagem adequada.
   Devolve 0 se o jogo terminou, 1 se ainda há jogo. */
static int verificarFimJogo(EstadoJogo *e) {
    if (verificarVitoria(e)) {
        printf("\nPARABENS! Completaste as 4 sequencias e ganhaste o jogo!\n");
        return 0;
    }
    if (verificarDerrota(e)) {
        printf("\nSem mais jogadas possiveis. Boa tentativa!\n");
        return 0;
    }
    return 1;
}


/* Inicializa o EstadoJogo e prepara o tabuleiro para uma nova partida. */
static void iniciarEstado(EstadoJogo *e) {
    e->destaque_origem      = -1;
    e->destaque_destino     = -1;
    e->sequencias_removidas = 0;
    prepararBaralho(e);
    misturarBaralho(e);
    prepararMesa(e);
}


/* Ponto de entrada: inicializa o jogo e corre o ciclo principal. */
int main(void) {
    srand((unsigned int)time(NULL));

    EstadoJogo estado;
    iniciarEstado(&estado);

    int jogar = 1;
    while (jogar == 1) {
        system("clear");
        mostrarMesa(&estado);
        limparDestaques(&estado);
        jogar = verificarFimJogo(&estado);
        if (jogar == 1) jogar = executarTurno(&estado);
    }

    return 0;
}
#include <stdio.h>
#include "interface.h"

/* Repõe os destaques do estado a -1 (sem sugestão activa). */
void limparDestaques(EstadoJogo *e) {
    e->destaque_origem  = -1;
    e->destaque_destino = -1;
}


/* Devolve o tipo de destaque da coluna: 0=nenhum, 1=origem (roxo), 2=destino (verde). */
static int tipoDestaque(EstadoJogo *e, int coluna) {
    if (coluna == e->destaque_origem)  return 1;
    if (coluna == e->destaque_destino) return 2;
    return 0;
}


/* Emite o código de cor ANSI adequado antes de desenhar uma carta.
   Prioridade: destaque (dica) > cor do naipe. */
static void setCorCarta(Cartas c, int destaque) {
    if      (destaque == 1)                    printf(COR_ROXA);
    else if (destaque == 2)                    printf(COR_VERDE);
    else if (c.naipe == 1 || c.naipe == 2)     printf(COR_VERMELHA);
}


/* Repõe a cor do terminal se esta tiver sido alterada. */
static void resetCor(Cartas c, int destaque) {
    if (destaque != 0 || c.naipe == 1 || c.naipe == 2) printf(COR_RESET);
}


/* Desenha a linha superior (┌─────┐) das cartas na posição 'linha'. */
static void imprimirLinhaTopo(EstadoJogo *e, int linha) {
    FOR(i, NUM_COLUNAS) {
        if (linha <= e->colunas[i].topo) {
            Cartas c    = e->colunas[i].cartas[linha];
            int    dest = tipoDestaque(e, i);
            setCorCarta(c, dest);
            printf("\u250C\u2500\u2500\u2500\u2500\u2500\u2510"); /* ┌─────┐ */
            resetCor(c, dest);
        } else {
            printf("       "); /* 7 espaços para manter alinhamento */
        }
        printf(" ");
    }
    printf("\n");
}


/* Desenha a linha central (│ V N │) das cartas na posição 'linha'. */
static void imprimirLinhaMeio(EstadoJogo *e, int linha) {
    FOR(i, NUM_COLUNAS) {
        if (linha <= e->colunas[i].topo) {
            Cartas c    = e->colunas[i].cartas[linha];
            int    dest = tipoDestaque(e, i);
            setCorCarta(c, dest);
            /* %-2s garante 2 caracteres para o valor (alinha o símbolo do naipe) */
            printf("\u2502 %-2s%s \u2502", LETRAS_VALORES[c.valor], SIMBOLOS_NAIPES[c.naipe]);
            resetCor(c, dest);
        } else {
            printf("       ");
        }
        printf(" ");
    }
    printf("\n");
}


/* Desenha a linha inferior (└─────┘) das cartas na posição 'linha'. */
static void imprimirLinhaFundo(EstadoJogo *e, int linha) {
    FOR(i, NUM_COLUNAS) {
        if (linha <= e->colunas[i].topo) {
            Cartas c    = e->colunas[i].cartas[linha];
            int    dest = tipoDestaque(e, i);
            setCorCarta(c, dest);
            printf("\u2514\u2500\u2500\u2500\u2500\u2500\u2518"); /* └─────┘ */
            resetCor(c, dest);
        } else {
            printf("       ");
        }
        printf(" ");
    }
    printf("\n");
}


/* Devolve o número máximo de cartas em qualquer coluna (linhas a desenhar). */
static int calcularMaxLinhas(EstadoJogo *e) {
    int max = 0;
    FOR(i, NUM_COLUNAS) {
        if (e->colunas[i].topo + 1 > max) max = e->colunas[i].topo + 1;
    }
    return max;
}


/* Imprime os rótulos C1…C10, destacando a coluna de origem (roxo)
   e a de destino (verde) da dica activa. */
static void imprimirCabecalhosColunas(EstadoJogo *e) {
    FOR(i, NUM_COLUNAS) {
        int dest = tipoDestaque(e, i);
        if (dest == 1) printf(COR_ROXA);
        if (dest == 2) printf(COR_VERDE);
        printf("C%-2d     ", i + 1); /* 8 chars: alinha com a largura de cada carta */
        if (dest != 0) printf(COR_RESET);
    }
    printf("\n\n");
}


/* Imprime o cabeçalho com o título e o contador de sequências concluídas. */
static void imprimirCabecalho(EstadoJogo *e) {
    printf("\n================================================================");
    printf("================\n");
    printf("  Simple Simon  |  Sequencias completas: %d / 4\n",
           e->sequencias_removidas);
    printf("================================================================");
    printf("================\n\n");
}


/* Função principal da interface: desenha o estado completo do tabuleiro. */
void mostrarMesa(EstadoJogo *e) {
    imprimirCabecalho(e);
    imprimirCabecalhosColunas(e);

    int max = calcularMaxLinhas(e);
    FOR(linha, max) {
        imprimirLinhaTopo(e, linha);
        imprimirLinhaMeio(e, linha);
        imprimirLinhaFundo(e, linha);
    }

    printf("================================================================");
    printf("================\n");
}
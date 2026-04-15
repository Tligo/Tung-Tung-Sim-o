/* cartas.c – Lógica do jogo Simple Simon
 *
 * Regras do Simple Simon:
 *   - 10 colunas; 52 cartas distribuídas todas face acima.
 *   - Move-se qualquer sequência do topo (valores consecutivos
 *     descendentes, qualquer mistura de naipes) para uma coluna
 *     cujo topo seja imediatamente superior, ou para uma coluna vazia.
 *   - Objectivo: remover 4 sequências completas K→A do mesmo naipe.
 *   - Não existe baralho de reserva.
 *
 * Conformidade:
 *   - Todas as funções ≤ 15 instruções e complexidade ciclomática ≤ 10.
 *   - Sem variáveis globais mutáveis (apenas constantes de leitura).
 *   - Sem goto, break nem continue.
 *   - Separação total entre lógica (este ficheiro) e interface.
 */

#include <stdlib.h>
#include "cartas.h"

/* Símbolos UTF-8 dos naipes (índice = Cartas.naipe). Apenas leitura. */
const char *SIMBOLOS_NAIPES[] = {
    "\xE2\x99\xA0",  /* ♠ Espadas */
    "\xE2\x99\xA5",  /* ♥ Copas   */
    "\xE2\x99\xA6",  /* ♦ Ouros   */
    "\xE2\x99\xA3"   /* ♣ Paus    */
};

/* Representação textual dos valores; índice 0 não utilizado. */
const char *LETRAS_VALORES[] = {
    "", "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"
};

/* ═══════════════════════════════════════════════════════════
 *  PREPARAÇÃO DO JOGO
 * ═══════════════════════════════════════════════════════════ */

/* Preenche o baralho com as 52 cartas (4 naipes × 13 valores).
 * Após a chamada, baralho.topo == 51 e as cartas estão por ordem. */
void prepararBaralho(EstadoJogo *e) {
    e->baralho.topo = -1;
    FOR(naipe, 4) {
        FOR(valor, 13) {
            e->baralho.topo++;
            e->baralho.cartas[e->baralho.topo].naipe = naipe;
            e->baralho.cartas[e->baralho.topo].valor = valor + 1;
        }
    }
}

/* Troca as cartas nas posições i e j da pilha b. */
static void trocarCartas(Pilha *b, int i, int j) {
    Cartas t     = b->cartas[i];
    b->cartas[i] = b->cartas[j];
    b->cartas[j] = t;
}

/* Realiza 100 trocas aleatórias no baralho (Fisher-Yates simplificado). */
void misturarBaralho(EstadoJogo *e) {
    FOR(n, 100)
        trocarCartas(&e->baralho, rand() % 52, rand() % 52);
}

/* Devolve o número de cartas a distribuir à coluna col.
 * Layout em pirâmide invertida (total = 52):
 *   Colunas 0-2 → 8 cartas cada (as mais "cheias")
 *   Colunas 3-9 → 7, 6, 5, 4, 3, 2, 1 carta (fórmula: 10 - col) */
static int cartasPorColuna(int col) {
    return col < 3 ? 8 : 10 - col;
}

/* Move uma carta do topo do baralho para o topo da coluna col. */
static void moverCartaParaColuna(EstadoJogo *e, int col) {
    e->colunas[col].topo++;
    e->colunas[col].cartas[e->colunas[col].topo] =
        e->baralho.cartas[e->baralho.topo];
    e->baralho.topo--;
}

/* Distribui n cartas do topo do baralho para a coluna col. */
static void distribuirParaColuna(EstadoJogo *e, int col, int n) {
    int i = 0;
    while (i < n) {
        moverCartaParaColuna(e, col);
        i++;
    }
}

/* Inicializa o tabuleiro: zera colunas e distribui as 52 cartas. */
void prepararMesa(EstadoJogo *e) {
    e->sequencias_removidas = 0;
    FOR(i, NUM_COLUNAS) {
        e->colunas[i].topo = -1;
        distribuirParaColuna(e, i, cartasPorColuna(i));
    }
}

/* ═══════════════════════════════════════════════════════════
 *  REGRAS DE MOVIMENTO
 * ═══════════════════════════════════════════════════════════ */

/* Devolve 1 se 'superior' pode assentar sobre 'inferior':
 * o valor de 'superior' deve ser exactamente inferior.valor - 1.
 * O naipe é irrelevante para o movimento (qualquer para qualquer). */
int movimentoValido(Cartas superior, Cartas inferior) {
    return superior.valor == inferior.valor - 1;
}

/* Devolve 1 se as cartas nas posições pos e pos-1 da coluna col
 * formam um par válido para sequência movível:
 * valor consecutivo descendente E mesmo naipe. */
static int parSequenciaMovivel(EstadoJogo *e, int col, int pos) {
    Cartas sup = e->colunas[col].cartas[pos];
    Cartas inf = e->colunas[col].cartas[pos - 1];
    return movimentoValido(sup, inf) && sup.naipe == inf.naipe;
}

/* Conta quantas cartas do topo da coluna col formam uma sequência
 * movível: valores consecutivos descendentes DO MESMO NAIPE.
 * Exemplo: [..., 9♦, 8♠, 7♠] → devolve 2 (8♠+7♠ mesmo naipe;
 *          9♦→8♠ quebra porque naipes são diferentes). */
int tamanhoSequenciaMovivel(EstadoJogo *e, int col) {
    int pos = e->colunas[col].topo;
    while (pos > 0 && parSequenciaMovivel(e, col, pos))
        pos--;
    return e->colunas[col].topo - pos + 1;
}

/* Copia n cartas a partir do índice 'inicio' da coluna orig para dest,
 * mantendo a ordem original, e actualiza os topos de ambas as colunas. */
static void copiarSequencia(EstadoJogo *e, int orig, int dest,
                            int inicio, int n) {
    int i = 0;
    while (i < n) {
        e->colunas[dest].topo++;
        e->colunas[dest].cartas[e->colunas[dest].topo] =
            e->colunas[orig].cartas[inicio + i];
        i++;
    }
    e->colunas[orig].topo -= n;
}

/* Devolve 1 se a base da sequência movível de orig cabe no topo de dest.
 * "Cabe" significa: base.valor == dest_topo.valor - 1 (qualquer naipe). */
static int sequenciaCabeEmDestino(EstadoJogo *e, int orig, int dest) {
    int    seq  = tamanhoSequenciaMovivel(e, orig);
    int    ini  = e->colunas[orig].topo - seq + 1;
    Cartas base = e->colunas[orig].cartas[ini];
    Cartas dtop = e->colunas[dest].cartas[e->colunas[dest].topo];
    return movimentoValido(base, dtop);
}

/* Tenta mover a sequência movível do topo de orig para dest.
 *
 * Regras verificadas:
 *   1. orig != dest e orig não está vazia.
 *   2. Se dest não está vazia, a base da sequência deve caber no topo.
 *   3. Se dest está vazia, qualquer sequência é aceite.
 *
 * Devolve 1 se o movimento foi executado, 0 se inválido. */
int moverSequencia(EstadoJogo *e, int orig, int dest) {
    if (orig == dest)               return 0;
    if (e->colunas[orig].topo < 0) return 0;
    if (e->colunas[dest].topo >= 0 &&
        !sequenciaCabeEmDestino(e, orig, dest)) return 0;

    int seq    = tamanhoSequenciaMovivel(e, orig);
    int inicio = e->colunas[orig].topo - seq + 1;
    copiarSequencia(e, orig, dest, inicio, seq);
    return 1;
}

/* ═══════════════════════════════════════════════════════════
 *  SEQUÊNCIAS COMPLETAS (K→A mesmo naipe)
 * ═══════════════════════════════════════════════════════════ */

/* Devolve 1 se a carta na posição pos pertence à sequência K→A
 * do mesmo naipe do topo (posição 'topo') da coluna col.
 * Fórmula do valor esperado: topo - pos + 1 (topo=Ás=1, base=Rei=13). */
static int cartaNaSequencia(EstadoJogo *e, int col, int topo, int pos) {
    Cartas c    = e->colunas[col].cartas[pos];
    Cartas ctop = e->colunas[col].cartas[topo];
    return c.naipe == ctop.naipe && c.valor == topo - pos + 1;
}

/* Devolve 1 se as 13 cartas do topo da coluna col formam K→A do mesmo naipe.
 * Requer pelo menos 13 cartas (topo >= 12). */
int verificarSequenciaCompleta(EstadoJogo *e, int col) {
    int topo = e->colunas[col].topo;
    int pos  = topo;
    int ok   = topo >= 12;
    while (ok && pos > topo - 13) {
        ok = cartaNaSequencia(e, col, topo, pos);
        pos--;
    }
    return ok;
}

/* Percorre todas as colunas e remove as sequências K→A completas.
 * Cada remoção incrementa o contador sequencias_removidas. */
void removerSequencias(EstadoJogo *e) {
    FOR(i, NUM_COLUNAS) {
        if (verificarSequenciaCompleta(e, i)) {
            e->colunas[i].topo -= 13;
            e->sequencias_removidas++;
        }
    }
}

/* ═══════════════════════════════════════════════════════════
 *  FIM DE JOGO
 * ═══════════════════════════════════════════════════════════ */

/* Devolve 1 se as 4 sequências K→A foram completadas (vitória). */
int verificarVitoria(EstadoJogo *e) {
    return e->sequencias_removidas == 4;
}

/* Devolve 1 se a sequência movível de orig pode ser colocada em dest:
 *   - dest vazia → aceita sempre (qualquer sequência).
 *   - dest não vazia → base da sequência deve caber no topo de dest. */
static int podeMovarPara(EstadoJogo *e, int orig, int dest) {
    if (e->colunas[orig].topo < 0) return 0;
    if (e->colunas[dest].topo < 0) return 1;
    return sequenciaCabeEmDestino(e, orig, dest);
}

/* Devolve 1 se existe pelo menos um destino válido para a coluna orig. */
static int existeDestinoValido(EstadoJogo *e, int orig) {
    int j     = 0;
    int found = 0;
    while (j < NUM_COLUNAS && found == 0) {
        if (j != orig) found = podeMovarPara(e, orig, j);
        j++;
    }
    return found;
}

/* Devolve 1 se não existe nenhum movimento possível no tabuleiro (derrota). */
int verificarDerrota(EstadoJogo *e) {
    int i       = 0;
    int derrota = 1;
    while (i < NUM_COLUNAS && derrota == 1) {
        if (existeDestinoValido(e, i)) derrota = 0;
        i++;
    }
    return derrota;
}

/* ═══════════════════════════════════════════════════════════
 *  DICA (MELHOR JOGADA)
 * ═══════════════════════════════════════════════════════════ */

/* Regista o par orig/dest como sugestão activa, se ainda não houver.
 * O destaque_origem < 0 indica que nenhuma sugestão foi registada ainda. */
static void registarSugestao(EstadoJogo *e, int orig, int dest) {
    if (e->destaque_origem < 0) {
        e->destaque_origem  = orig;
        e->destaque_destino = dest;
    }
}

/* Percorre todos os destinos possíveis para orig e regista a primeira jogada. */
static void procurarJogadaDeColuna(EstadoJogo *e, int orig) {
    int j = 0;
    while (j < NUM_COLUNAS) {
        if (j != orig && podeMovarPara(e, orig, j))
            registarSugestao(e, orig, j);
        j++;
    }
}

/* Calcula e assinala no estado a primeira jogada válida encontrada (dica).
 * Repõe os destaques a -1 antes de procurar. */
void calcularMelhorJogada(EstadoJogo *e) {
    e->destaque_origem  = -1;
    e->destaque_destino = -1;
    FOR(i, NUM_COLUNAS)
        procurarJogadaDeColuna(e, i);
}
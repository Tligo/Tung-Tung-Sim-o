#include <stdlib.h>
#include "cartas.h"

/* Símbolos UTF-8: Espadas, Copas, Ouros, Paus. */
const char *SIMBOLOS_NAIPES[] = {
    "\xE2\x99\xA0", "\xE2\x99\xA5", "\xE2\x99\xA6", "\xE2\x99\xA3"
};

/* Representação textual dos valores (índice 0 não usado). */
const char *LETRAS_VALORES[] = {
    "", "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"
};


/* Preenche o baralho com as 52 cartas (4 naipes × 13 valores). */
void prepararBaralho(EstadoJogo *e) {
    e->baralho.topo = -1;
    FOR(naipe, 4) {
        FOR(valor, 13) {
            e->baralho.topo++;
            e->baralho.cartas[e->baralho.topo].valor = valor + 1;
            e->baralho.cartas[e->baralho.topo].naipe = naipe;
        }
    }
}


/* Realiza 100 trocas aleatórias para baralhar as cartas. */
void misturarBaralho(EstadoJogo *e) {
    FOR(n, 100) {
        int    i             = rand() % 52;
        int    j             = rand() % 52;
        Cartas t             = e->baralho.cartas[i];
        e->baralho.cartas[i] = e->baralho.cartas[j];
        e->baralho.cartas[j] = t;
    }
}


/* Devolve quantas cartas devem ser distribuídas à coluna 'col'.
   As colunas 0-1 recebem 6 cartas; as restantes 8 recebem 5 (total = 52). */
static int cartasPorColuna(int col) {
    return (col < 2) ? 6 : 5;
}


/* Retira n cartas do topo do baralho e coloca-as na coluna col. */
static void distribuirParaColuna(EstadoJogo *e, int col, int n) {
    FOR(j, n) {
        e->colunas[col].topo++;
        e->colunas[col].cartas[e->colunas[col].topo] =
            e->baralho.cartas[e->baralho.topo];
        e->baralho.topo--;
    }
}


/* Distribui as 52 cartas pelas 10 colunas e esvazia o baralho. */
static void distribuirColunas(EstadoJogo *e) {
    FOR(i, NUM_COLUNAS) {
        distribuirParaColuna(e, i, cartasPorColuna(i));
    }
}


/* Inicializa as colunas e distribui as cartas.
   No Simple Simon não existe baralho nem descarte visíveis. */
void prepararMesa(EstadoJogo *e) {
    FOR(i, NUM_COLUNAS) {
        e->colunas[i].topo = -1;
    }
    e->sequencias_removidas = 0;
    distribuirColunas(e);
}


/* Devolve 1 se 'origem' pode ser colocada em cima de 'destino':
   valor imediatamente inferior, qualquer naipe.
   Ex: 4♥ em cima de 5♦ é válido; 4♥ em cima de 6♦ não é. */
int movimentoValido(Cartas origem, Cartas destino) {
    return (origem.valor == destino.valor - 1) ? 1 : 0;
}


/* Conta quantas cartas do topo da coluna formam uma sequência
   consecutiva movível (valores adjacentes, qualquer naipe). */
int tamanhoSequenciaMovivel(EstadoJogo *e, int coluna) {
    int seq = 1;
    int pos = e->colunas[coluna].topo;

    while (pos > 0 &&
           movimentoValido(e->colunas[coluna].cartas[pos],
                           e->colunas[coluna].cartas[pos - 1])) {
        seq++;
        pos--;
    }
    return seq;
}


/* Copia n cartas a partir do índice 'inicio' da coluna origem para destino,
   mantendo a ordem, e actualiza os topos de ambas as colunas. */
static void copiarSequencia(EstadoJogo *e, int origem, int destino,
                            int inicio, int n) {
    FOR(i, n) {
        e->colunas[destino].topo++;
        e->colunas[destino].cartas[e->colunas[destino].topo] =
            e->colunas[origem].cartas[inicio + i];
    }
    e->colunas[origem].topo -= n;
}


/* Tenta mover a sequência movível do topo de 'origem' para 'destino'.
   Regra: a carta base da sequência deve ser imediatamente inferior ao
   topo do destino (qualquer naipe), ou o destino deve estar vazio.
   Devolve 1 se o movimento foi executado, 0 se inválido. */
int moverSequencia(EstadoJogo *e, int origem, int destino) {
    if (e->colunas[origem].topo < 0) return 0;

    int    seq    = tamanhoSequenciaMovivel(e, origem);
    int    inicio = e->colunas[origem].topo - seq + 1;
    Cartas base   = e->colunas[origem].cartas[inicio];

    if (e->colunas[destino].topo >= 0) {
        Cartas dest = e->colunas[destino].cartas[e->colunas[destino].topo];
        if (!movimentoValido(base, dest)) return 0;
    }

    copiarSequencia(e, origem, destino, inicio, seq);
    return 1;
}


/* Verifica se as 13 cartas do topo da coluna formam K→A do mesmo naipe.
   A carta do topo deve ser Ás (valor 1) e a 13.ª abaixo deve ser Rei (valor 13). */
int verificarSequenciaCompleta(EstadoJogo *e, int coluna) {
    if (e->colunas[coluna].topo < 12) return 0;

    int topo  = e->colunas[coluna].topo;
    int naipe = e->colunas[coluna].cartas[topo].naipe;
    int pos   = topo;
    int ok    = 1;

    while (pos > topo - 13 && ok) {
        Cartas c = e->colunas[coluna].cartas[pos];
        ok  = (c.naipe == naipe) && (c.valor == topo - pos + 1);
        pos--;
    }
    return ok;
}


/* Percorre todas as colunas e remove as sequências completas (K→A mesmo naipe).
   Cada remoção incrementa o contador de sequências. */
void removerSequencias(EstadoJogo *e) {
    FOR(i, NUM_COLUNAS) {
        if (verificarSequenciaCompleta(e, i)) {
            e->colunas[i].topo -= 13;
            e->sequencias_removidas++;
        }
    }
}


/* Devolve 1 se o jogo foi ganho (4 sequências completas removidas). */
int verificarVitoria(EstadoJogo *e) {
    return (e->sequencias_removidas == 4) ? 1 : 0;
}


/* Verifica se a coluna 'origem' consegue mover a sua sequência para 'destino'.
   Uma coluna vazia aceita qualquer sequência. */
static int podeMovarPara(EstadoJogo *e, int origem, int destino) {
    if (e->colunas[origem].topo < 0) return 0;
    if (e->colunas[destino].topo < 0) return 1;

    int    seq  = tamanhoSequenciaMovivel(e, origem);
    int    ini  = e->colunas[origem].topo - seq + 1;
    Cartas base = e->colunas[origem].cartas[ini];
    Cartas dest = e->colunas[destino].cartas[e->colunas[destino].topo];

    return movimentoValido(base, dest);
}


/* Devolve 1 se a coluna i tem pelo menos um destino válido. */
static int existeJogadaDeColuna(EstadoJogo *e, int i) {
    int encontrou = 0;
    FOR(j, NUM_COLUNAS) {
        if (j != i && podeMovarPara(e, i, j)) encontrou = 1;
    }
    return encontrou;
}


/* Devolve 1 se não existe nenhum movimento possível no tabuleiro (derrota). */
int verificarDerrota(EstadoJogo *e) {
    int derrota = 1;
    FOR(i, NUM_COLUNAS) {
        if (existeJogadaDeColuna(e, i)) derrota = 0;
    }
    return derrota;
}


/* Procura o primeiro destino válido para a coluna i e, se ainda não houver
   sugestão activa, regista o par origem/destino no estado. */
static void encontrarJogadaDeColuna(EstadoJogo *e, int i) {
    FOR(j, NUM_COLUNAS) {
        if (j != i && podeMovarPara(e, i, j) && e->destaque_origem < 0) {
            e->destaque_origem  = i;
            e->destaque_destino = j;
        }
    }
}


/* Assinala no estado a primeira jogada válida encontrada (dica ao jogador). */
void calcularMelhorJogada(EstadoJogo *e) {
    e->destaque_origem  = -1;
    e->destaque_destino = -1;
    FOR(i, NUM_COLUNAS) {
        encontrarJogadaDeColuna(e, i);
    }
}

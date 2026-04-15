#ifndef CARTAS_H
#define CARTAS_H

#define FOR(i, n)       for (int i = 0; i < (n); i++)
#define NUM_COLUNAS     10
#define MAX_COLUNA      53   /* margem de segurança: máximo de cartas numa coluna */
#define MAX_HISTORICO   100  /* número máximo de jogadas que se podem desfazer   */

/* ── Tipos base ── */

/* Representa uma carta do baralho. */
typedef struct {
    int valor; /* 1=A, 2-10, 11=J, 12=Q, 13=K */
    int naipe; /* 0=Espadas, 1=Copas, 2=Ouros, 3=Paus */
} Cartas;

/* Pilha genérica de cartas (coluna ou baralho temporário de distribuição). */
typedef struct {
    Cartas cartas[MAX_COLUNA];
    int    topo; /* -1 = pilha vazia */
} Pilha;

/* ── Histórico (suporte de undo) ── */

/* Fotografia do tabuleiro num dado momento.
 * Guarda apenas o que muda com as jogadas: colunas e contador de sequências. */
typedef struct {
    Pilha colunas[NUM_COLUNAS];
    int   sequencias_removidas;
} EstadoSnapshot;

/* ── Estado completo do jogo ── */

/* Passado por ponteiro a todas as funções. */
typedef struct {
    Pilha          colunas[NUM_COLUNAS];
    Pilha          baralho;              /* usado apenas durante a inicialização  */
    int            destaque_origem;      /* coluna sugerida de origem (-1=nenhuma) */
    int            destaque_destino;     /* coluna sugerida de destino (-1=nenhuma)*/
    int            sequencias_removidas; /* sequências K→A completas retiradas (0-4)*/
    EstadoSnapshot historico[MAX_HISTORICO]; /* pilha de estados anteriores       */
    int            historico_topo;       /* índice do topo do histórico (-1=vazio) */
} EstadoJogo;

/* ── Constantes de texto exportadas para a interface ── */
extern const char *SIMBOLOS_NAIPES[];
extern const char *LETRAS_VALORES[];

/* ── Protótipos da lógica do jogo ── */
void prepararBaralho(EstadoJogo *e);
void misturarBaralho(EstadoJogo *e);
void prepararMesa(EstadoJogo *e);
int  movimentoValido(Cartas origem, Cartas destino);
int  tamanhoSequenciaMovivel(EstadoJogo *e, int coluna);
int  moverSequencia(EstadoJogo *e, int origem, int destino);
int  verificarSequenciaCompleta(EstadoJogo *e, int coluna);
void removerSequencias(EstadoJogo *e);
int  verificarVitoria(EstadoJogo *e);
int  verificarDerrota(EstadoJogo *e);
void calcularMelhorJogada(EstadoJogo *e);

/* ── Protótipos do undo ── */
void guardarEstado(EstadoJogo *e);
void cancelarGuardado(EstadoJogo *e);
int  desfazerJogada(EstadoJogo *e);

#endif
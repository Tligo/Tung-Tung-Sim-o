#ifndef INTERFACE_H
#define INTERFACE_H

#include "cartas.h"

/* Códigos de cor ANSI. */
#define COR_VERMELHA "\033[31m"
#define COR_ROXA     "\033[35m"
#define COR_VERDE    "\033[32m"
#define COR_RESET    "\033[0m"

/* ── Protótipos da interface ── */
void mostrarMesa(EstadoJogo *e);
void limparDestaques(EstadoJogo *e);

#endif
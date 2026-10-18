/*
 * function_finder.h — 68K function boundary detection interface.
 */
#pragma once
#include <stdint.h>
#include "rom_parser.h"
#include "game_config.h"

typedef struct {
    uint32_t addr;
} FunctionEntry;

typedef struct {
    FunctionEntry *entries;
    int            count;
    int            capacity;
} FunctionList;

void function_finder_run(const GenesisRom *rom, FunctionList *list, const GameConfig *cfg);
/* Relocatable modules have declared entries, not a reset/vector table. Uses
 * cfg extra functions as seeds and otherwise shares normal CFG discovery. */
void function_finder_run_module(const GenesisRom *rom, FunctionList *list, const GameConfig *cfg);
void function_list_free(FunctionList *list);

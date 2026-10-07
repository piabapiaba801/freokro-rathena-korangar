# Advanced Pet System — Fase 5: Raio Moderno de Coleta

O Modern Pet Loot do FreokRO entrega cada pickup diretamente ao inventario/party. Por isso, o argumento `petloot N` passa a ser interpretado como **raio de deteccao em celulas**, e nao como capacidade de armazenamento.

## Distribuicao
- Poring 5 -> Mastering 7 -> Angeling 9
- Drops 5 -> Eggring 7 / Sweets Drops 7
- Yoyo 5 -> Choco 7
- Poporing 6
- Marin 6

## Regras preservadas
O pet ainda precisa caminhar fisicamente ate o drop. O pathfinding precisa encontrar caminho valido. Prioridade de loot, party loot, dono morto/cant.pickup, peso, slots, stack, Random Options e anuncios especiais continuam sendo validados. Falha de inventario deixa o item no chao.

O limite de quantidade e a bag do jogador; nao existe mais limite artificial de 10/15/20/25 itens no pet. O range permanece limitado tecnicamente a 1..30 celulas no builtin para evitar valores abusivos.

# Party Job Synergy - implementação custom

Implementação independente inspirada no conceito público do tópico rAthena Party Job Synergy Bonus.

## Correção do problema relatado no tópico
O release original recebeu relato de bônus permanecer após sair da party até relog/portal. Esta implementação não depende de recálculo incidental: força `status_calc_pc` dos membros ativos em entrada, saída e mudança de mapa/estado da party, e recalcula também o membro que saiu após limpar `party_id`.

## Segurança de cálculo
Durante `status_calc_pc`, a composição é lida diretamente de `party_data` e limitada a membros ativos no mesmo mapa. Não usa `party_foreachsamemap` para disparar recálculos dentro do próprio cálculo de status, evitando cascatas/recursão desnecessária.

## Bônus padrão por membro da linhagem no mesmo mapa
- Swordman: +1 VIT, +5% recuperação natural de HP
- Mage: -2% taxa de cast variável
- Archer: +1 DEX, +2% HIT
- Thief: +1 AGI, +2% FLEE
- Merchant: +100 capacidade de peso exibida e +2% velocidade de movimento por membro (cap de 20%)
- Acolyte: +5% recuperação natural de SP

As contribuições acumulam por membro elegível. Autotrade e membros fora do mapa não contam.

## Build
`status.cpp` e `party.cpp` compilados individualmente com sucesso. O full build foi iniciado repetidamente e avançou sem erro de código, mas excedeu a janela de execução enquanto recompilava a árvore completa; portanto não é marcado como full-build validated.

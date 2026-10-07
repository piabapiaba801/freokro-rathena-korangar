# Advanced Pet System — Fase 7 — EXP e Drop pessoal

## Objetivo
Criar duas novas especializacoes para pets `[uti]`: aumento de EXP e aumento de Drop Rate exclusivamente como bonus do dono do pet.

## EXP pessoal
- Lunatic: +5%
- Leaf Lunatic: +8%
- Rocker: +5%
- Metaller: +8%
- Peco Peco: +5%
- Grand Peco: +8%

Implementacao: `bonus2 bExpAddRace,RC_All,X;` no `SupportScript` do pet. O core aplica `expaddrace` em `pc_calcexp`, descrito pelo proprio codigo como bonus pessoais que nao sao compartilhados com party. A contribuicao de guilda e calculada antes de `pc_calcexp`, portanto o incremento do pet nao aumenta a parcela da guilda.

## Drop pessoal
- Smokie: +5%
- Spore: +5%
- Christmas Goblin: +5%
- Miyabi Ningyo: +5%
- Civil Servant: +5%
- Leaf Cat: +5%

Implementacao: `bonus2 bDropAddRace,RC_All,5;`. O calculo de drop consulta `sd->indexed_bonus.dropaddrace` do jogador que originou o drop. O bonus de taxa nao e aplicado aos outros membros do grupo ou guilda.

Observacao: depois que um item efetivamente cai, as regras normais de propriedade/distribuicao de loot da party continuam valendo. O que e pessoal e o modificador de taxa.

## Regra de familia
As familias escolhidas para EXP mantem a especializacao na evolucao e sobem de +5% para +8%. Nenhuma evolucao muda de categoria.

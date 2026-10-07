# FreokRO V18 — Advanced Pet System — Fase 3

## Pet Loot sem requisito de Lealdade
O Modern Pet Loot da Fase 2 permanece independente de `pet_support_min_friendly` (900). A coleta funciona em qualquer intimidade positiva; intimidade zero continua sendo o limite nativo em que o pet é perdido/removido.

## AutoFeed universal
- `AllowAutoFeed` passa a ser TRUE por padrão para qualquer pet que não declare explicitamente o campo no pet_db.
- A base atual não possui entradas ativas `AllowAutoFeed: false`; portanto os 107 pets Renewal ficam aptos ao AutoFeed.
- `feature.petautofeed: on` e `pet_autofeed_always: yes` permanecem habilitados.
- O alimento continua sendo consumido do inventário e a rotina nativa de fome/intimidade continua sendo usada.

## Comando @autofeed
Disponível ao grupo Player e herdado pelos grupos superiores.
- `@autofeed` ou `@autofeed on`: ativa e salva o AutoFeed do pet atual.
- `@autofeed off`: desativa e salva.
- `@autofeed now` (também `agora`/`feed`): força uma alimentação imediata usando a comida normal do pet e salva o estado resultante.

O comando não cria comida, não ignora inventário e não alimenta sem o item correto.

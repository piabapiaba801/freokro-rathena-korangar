# FreokRO V18 — Advanced Pet System — Fase 2

## Pet Loot moderno

O `petloot` continua sendo uma habilidade física do pet: o pet procura um drop elegível, caminha até ele e precisa alcançar o item.

Alteração FreokRO: ao alcançar o drop, o item não é mais armazenado no buffer temporário `pet_loot`. Ele entra imediatamente pelo pipeline normal de inventário/party do jogador.

Regras preservadas/aplicadas:
- prioridade temporal do drop (primeiro/segundo/terceiro looter);
- prioridade de party quando aplicável;
- configuração de distribuição de loot da party;
- bloqueio se o dono estiver morto;
- bloqueio quando o estado do personagem proíbe pickup;
- Random Options de drop normal continuam sendo aplicadas antes da entrega;
- anúncio de item especial continua preservado;
- peso, slots, limite de stack e demais validações continuam sendo feitas por `pc_additem`/`party_share_loot`;
- se a entrega falhar, o drop NÃO é apagado: permanece no chão;
- o item passa a usar a persistência normal do inventário do rAthena/char-server, sem depender de buffer volátil do pet.

O buffer nativo de `petloot` permanece na estrutura para compatibilidade com o core e scripts, mas o caminho FreokRO de coleta não o utiliza para novos pickups.

## Segurança / Autoloot administrativo

Os comandos `@autoloot`, `@alootid` e `@autoloottype` foram removidos do grupo Super Player. Jogadores comuns/VIP não recebem esses comandos por esta configuração. O grupo Admin (99) mantém acesso por `all_commands`.

## Intenção de design

Pet Utility Loot != @autoloot.

O pet precisa existir, estar ativo, cumprir as condições do seu SupportScript, localizar o drop, caminhar até ele e respeitar as regras de propriedade. O comando administrativo de autoloot continua sendo uma ferramenta separada do core.

## Phase 3 follow-up
- Modern Pet Loot has no Loyal/900-intimacy gate. Loot is available at any positive intimacy while the pet exists; zero intimacy remains the native abandonment boundary.

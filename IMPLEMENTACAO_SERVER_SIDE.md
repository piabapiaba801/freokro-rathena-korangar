# Implementação server-side custom — rAthena

Ordem executada: Skill 4 → Skill 1 → Skill 5 → Skill 2 → Skill 6 → Boss → Skill 7 → Skill 3 → Skill 8.

## Skills
- **Skill 4 — Social Basic Package (9003):** passiva Lv.1 que libera trade, emoções, sentar, criação de chat e criação de party. O NV_BASIC original (9 níveis) foi preservado como compatibilidade; qualquer uma das duas rotas satisfaz as permissões.
- **Skill 1 — Area Loot (9000):** coleta manual em área usando a lógica segura do Greed/pc_takeitem, portanto respeita as regras de propriedade e coleta do core.
- **Skill 5 — Return to Save Point (9004):** cast fixo de 3 s e cooldown de 30 min; retorna ao save point e respeita mapas NORETURN/NOTELEPORT.
- **Skill 2 — Monster Reading (9001):** alvo monstro; mostra nome, ID, nível, HP, ATK, DEF, MDEF, raça e elemento.
- **Skill 6 — Leaf Wings (9005):** instantânea, cooldown de 30 min. Solo ou não-líder: random warp próprio. Líder: comportamento de Giant Fly Wing, movendo membros online da mesma party/mapa para o novo ponto do líder.
- **Skill 7 — Tame Creature (9006):** usa o pipeline universal de captura de pets do rAthena. Persistência, egg/evolution e comportamentos continuam centralizados no pet_db, permitindo famílias looter/support/defensive/offensive/drop conforme os pets configurados.
- **Skill 3 — Universal Craft (9002):** abre a lista de produção sem exigir a profissão/skill original da receita; materiais, peso, quantidade e lógica de produção continuam validados pelo core.
- **Skill 8 — Black Market (9007):** abre remotamente o mercado server-side. Estoque e preços são SQL-authoritative, possui flag VIP, proteção contra venda simultânea da última unidade e log de compras.

## Boss regional persistente
`npc/custom/regional_boss_controller.txt` implementa contador por região, threshold, pool aleatório, exclusão do último boss, persistência SQL e restauração do MESMO boss selecionado após restart/crash. Regiões ficam desativadas por padrão porque mapas, raça-alvo e pools definitivos não foram especificados; basta preencher o bloco OnInit e definir `.RegionCount`.

## Banco SQL
Importar `sql-files/custom_project.sql` antes de ativar Boss/Black Market.

## Verificação
O `map-server` foi compilado com sucesso no ambiente Linux após as alterações. Uma segunda compilação incremental também terminou em 100% sem erros. Os YAML modificados foram carregados por parser YAML sem erro sintático.

## Boss regional - teste Orc Village
- Spawn natural via `boss_monster` foi desativado nos arquivos de mobs de campo/dungeon de `npc/re/mobs/` e `npc/pre-re/mobs/`. Spawns deliberados de quests, instâncias, arenas e scripts customizados não foram removidos.
- Primeira região ativa: Orc Village, mapa `gef_fild10`.
- A cada 1.000 abates regionais, o sistema seleciona Orc Hero (1087) ou Orc Lord (1190).
- O último boss não pode se repetir; com o pool de dois bosses, os ciclos alternam obrigatoriamente.
- Contador, boss selecionado/ativo e último boss continuam persistidos em `custom_regional_boss_state`.


## Atualizacao - Champion Monster + Skill Progression
- CUSTOM_RETURN_SAVE: MaxLevel 2; cooldown 30 min / 15 min.
- CUSTOM_LEAF_WINGS: MaxLevel 2; cooldown 30 min / 15 min.
- CUSTOM_TAME_CREATURE: MaxLevel 3. Pet DB aceita `CustomCategory: 1|2|3`; incubadora bloqueia ativacao quando o personagem nao possui o nivel necessario. Inventario/storage permanecem livres.
- Champion Monster: contador independente por mapa, threshold 30; anuncio somente no mapa com tipo/nome/coordenadas.
- Base Champion: HP x20, ATK x3, MATK x1.5, DEF x2, MDEF x2.
- Furious: NPC_POWERUP de MVP em <=51% HP, uma vez.
- Precious: drop chance x5 na rotina central `mob_getdroprate`.
- Fleeting: attack/move interval reduzido a 2/3 (50% mais acoes por tempo).
- Arcane: +50% magic sobre MATK Champion => x2.25 efetivo.
- Experienced: Base EXP x50 antes da distribuicao normal de EXP/party.
- Auras: Hat Effects existentes enviados server-side para o GID do mob.

### Market Clone — port C++ aplicado
- Port do `Offline Vending System.diff` aplicado à árvore custom atual.
- Comandos: `@autotrade2` cria vendedor clone persistente; `@closeclone` encerra o clone e devolve estoque remanescente após validações de inventário/peso/stack.
- Novo NPC subtype `NPCTYPE_VENDING_CLONE`, integração de packets de placa/lista/compra e restauração SQL no `do_init_vending`.
- Aparência do personagem, título, posição, moeda e estoque são persistidos; itens colocados no clone saem do carrinho real.
- SQL necessário: tabelas `vending_clones` e `vending_clone_items` em `sql-files/custom_project.sql`.

## Voice Chat — integração server-side (2025-07-16)
- Adicionado `src/map/voice_bridge.cpp/.hpp`, compatível com o protocolo UDP do projeto Sitecraft `rathena-voice-chat`.
- O map-server envia `auth_advisory` com AID/CID/LOGIN_ID1, posição/mapa periódica, join e revoke/leave.
- Bridge lê `conf/voice_athena.conf`; API padrão 127.0.0.1:7001 e voz TCP padrão 7000.
- Atualização de posição usa keepalive de 1,5 s; auth advisory é renovado em 5 s.
- Configuração base adicionada em `conf/voice_athena.conf` e override em `conf/import/voice_conf.txt`.
- Hooks adicionados ao login do personagem, logout e init/final do map-server.
- `voice_bridge.cpp`, `pc.cpp` e `map.cpp` foram compilados incrementalmente sem erro neste ambiente.
- O executável standalone `voice-server` do projeto Sitecraft NÃO foi reconstruído dentro desta árvore porque o snapshot completo de `src/voice/` não está anexado localmente. O bridge está pronto para falar com esse servidor na porta API 7001. Para tornar o pacote 100% autônomo, incorporar o diretório `src/voice/` upstream e suas regras de build quando o snapshot/ZIP oficial estiver disponível.

### ServerGuard — baseline anti-cheat server-side
- Adicionado `src/map/server_guard.cpp/.hpp` com rate limiting autoritativo para pacotes de skill e caminhada.
- Target skills e ground skills passam pelo mesmo contador de skill; caminhada possui janela independente.
- Configuração em `conf/import/battle_conf.txt`: enable/log/drop_excess/skill_pps/walk_pps.
- Padrões conservadores iniciais: 30 requests de skill/s e 40 requests de walk/s; excedentes são descartados e registrados.
- Não há autoban. O rollout é deliberadamente seguro contra falsos positivos; punições só devem ser consideradas depois de telemetria real.
- Mantidas as validações nativas do rAthena (`canact_tick`, skill delay, skill level/target validation etc.).

### MVP Tomb Extended próprio — server-side
- Tomb nativo ampliado sem copiar o código proprietário da edição paga.
- Janela 420x620 para clientes modernos; alvo atual PACKETVER 20250716.
- Snapshot no momento da morte: HP máximo, horário, mapa/coordenadas e duração da luta.
- Ranking de dano (Top 5 + percentual), Tank Ranking (Top 3), Top Guild e Top Party.
- Lista de MVP drops + drops normais com chance e `mesitemlink` clicável para abrir a informação nativa do item no cliente.
- Histórico persistente em `custom_mvp_tomb_history` e participantes em `custom_mvp_tomb_participants`.
- Cronômetro de combate inicia no primeiro evento atribuído a jogador e é zerado após a morte/respawn.
- Build Linux completo do map-server validado após a implementação (RC=0).

## Regional Boss tomb lifecycle (V2)
- Regional MVP tomb remains visible after the boss is defeated.
- Immediately before the next threshold-generated regional boss is spawned, tombs belonging to that region's configured boss pool are removed.
- For Orc Village (`gef_fild10`), the Orc Hero/Orc Lord tomb therefore disappears only when the next 1,000-kill boss actually spawns.
- Added script command `removemvptomb <map>,<mob_id>` backed by server-side tomb cleanup; `mob_id=0` can remove all MVP tombs on a map, but the regional controller deliberately removes only its configured boss IDs.

## Random Options V2 — monstros normais
- Geração natural de Random Options no chão continua desabilitada.
- Para drops de monstros normais, o sorteio ocorre somente na fronteira de aquisição pelo inventário (pickup/autoloot).
- Boss-class e Champions customizados são excluídos.
- Equipamentos elegíveis: Weapon, Armor (inclui headgear/acessório/escudo conforme tipo do item) e Shadow Gear.
- Distribuição global: 0=25%, 1=25%, 2=20%, 3=15%, 4=10%, 5=5% (100%).
- A quantidade sorteada é exata, até MAX_ITEM_RDM_OPT=5.
- Pools de atributos usam Group_1..Group_5 já existentes no rAthena; balanceamento dos atributos/valores pode ser customizado depois sem alterar a distribuição global.
- Transferências não rerrolam opções.
- Build Linux map-server PACKETVER 20250716: concluído com sucesso (LD map-server).

## Random Options V3 - Champions
- A mesma distribuicao global de quantidade (0=25%, 1=25%, 2=20%, 3=15%, 4=10%, 5=5%) agora tambem se aplica aos Custom Champion Monsters.
- MVPs/bosses continuam excluidos: elegibilidade exige CLASS_NORMAL.
- O sorteio continua ocorrendo somente na aquisicao/entrada no inventario, nao no item no chao.

## Preserve Toggle ON/OFF
- ST_PRESERVE foi sobrescrita via SkillFactoryCustom, mantendo os arquivos de implementacao principal intactos.
- Primeiro uso ativa Preserve normalmente.
- Reutilizar Preserve enquanto SC_PRESERVE esta ativo encerra o status (toggle OFF).
- Implementacao usa StatusSkillImpl(ST_PRESERVE, true), mecanismo nativo de toggle do skill framework atual.
- Nenhuma alteracao client-side e necessaria para a logica server-side.

## GearProtect V4
- Protecao contra venda acidental em NPC shops adicionada no fluxo autoritativo `npc_selllist`.
- `disable_card_sell: 1`: bloqueia venda direta de Cards.
- `disable_refine_sell: 1`: bloqueia equipamentos refinados (+1 ou superior).
- `disable_carded_sell: 1`: bloqueia equipamentos com Cards reais inseridos.
- Shop com unique name `sellcardnpc` pode receber Cards diretamente (Card Recycler/Collector); esse bypass nao libera equipamento refinado/cardeado.
- As tres protecoes sao configuraveis em `conf/battle/items.conf`.
- `npc.cpp` e `battle.cpp` compilados individualmente com sucesso (RC=0). O full build foi iniciado, mas excedeu a janela de execucao durante a recompilacao completa; nao foi marcado como full-build validado nesta revisao.

## Card Drop Announcement & Audit Log V6
- Global announcement + dedicated SQL audit for cards dropped by NORMAL, CHAMPION and BOSS/MVP monsters.
- Champion category uses the live `custom_champion_type` instance flag.
- SQL context: AID, CID, Mob ID, Item ID, category, Champion subtype, map, X/Y and timestamp.
- No client-side dependency.

## Global Player Skill Criticals (V7)
- Todas as skills de dano calculadas pelo pipeline BF_WEAPON/BF_MAGIC/BF_MISC de jogadores passam a ser elegiveis a critico.
- Chance base da skill = 50% da chance critica efetiva calculada para o personagem/alvo (CRI interno / 2).
- Exemplo: 100% CRIT efetivo -> 50% de chance critica na skill.
- Aplica a skills fisicas, magicas, misc, AoE e multi-hit; a rolagem ocorre por alvo.
- Fontes nao-player preservam a regra original do rAthena.
- Dano critico Renewal usa a regra ja existente: 140% + CRATE, respeitando crit_def_rate do alvo.
- O tipo DMG_CRITICAL/DMG_MULTI_HIT_CRITICAL e propagado no caminho generico de exibicao de skill.

## V14 — Monster Catalog / regra global sem multi-level
Ver `FREAOKRO_V14_MONSTER_CATALOG.md`. O multi-level foi desativado de forma definitiva no core, e o Olhar do Caçador passou a usar descoberta account-wide com recompensa apenas ao personagem ativo. Basic Skill #9 Catálogo de Monstros adicionada server-side.

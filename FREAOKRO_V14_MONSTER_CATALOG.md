# FreaokRO V14 — Olhar do Caçador + Catálogo de Monstros

## Regra global de progressão
- **Multi-level removido globalmente do FreaokRO.** Uma única concessão de EXP pode subir no máximo um Base Level e um Job Level; o excedente é limitado pela barra seguinte conforme a regra existente do servidor.
- As chaves legadas `multi_level_up*` permanecem no arquivo de configuração apenas por compatibilidade, mas não reativam o recurso.

## Descoberta
- Conhecimento é por **conta + Mob ID**.
- A EXP da primeira descoberta é paga **somente ao personagem ativo que identificou**.
- Normal: 100.000 Base + 100.000 Job EXP.
- Mini-Boss: 500.000 Base + 500.000 Job EXP.
- MVP: 500.000 Base + 500.000 Job EXP.
- Boosts pessoais normais de EXP são permitidos no momento da descoberta.
- Champions e Regional Bosses usam a espécie/Mob ID real; não criam uma segunda espécie.
- Slaves/summons, guardians e Emperium são recusados.

## Proteções críticas
1. `PRIMARY KEY(account_id,mob_id)` + `INSERT IGNORE` torna a primeira descoberta atômica contra spam/race.
2. Cache `monster_catalog_known` é carregado uma vez por sessão; renderização de mobs não consulta SQL.
3. Nome/nível/elemento são montados por destinatário no `clif_name`; desconhecidos recebem `????????` e conhecidos recebem dados reais.
4. O popup Sense usado pelo Olhar do Caçador é enviado somente ao próprio jogador, nunca à Party.
5. Journal `reward_state`: 0=pending, 1=claimed/in-flight, 2=delivered. O estado 1 é deixado para auditoria em crash ambíguo, evitando pagamento automático duplicado.

## Basic Skill #9
`CUSTOM_MONSTER_CATALOG` (ID 9008) abre remotamente `FreaokMonsterCatalog::OnOpen`. Não existe NPC físico no mapa. A interface NPC é provisória e pode ser substituída por HUD cliente sem mudar o backend.

## Pendente para Windows/cliente
- Registrar a skill 9008 no Lua/skillinfo/ícone do Ragexe 2025-07-16.
- Refinar a apresentação visual para ícones de elemento e labels definitivos; o servidor já entrega a decisão conhecido/desconhecido individualmente.
- Teste runtime com MySQL + cliente real, inclusive refresh visual, popup Sense e comportamento em mapas cheios.

## V14 follow-up — Basic Skill #9 and EXP kill-switch

- **Olhar do Caçador** remains the identification skill. It targets a monster, discovers the species for the account, reveals its information and opens the private Sense/Monster Info result.
- **Catálogo de Monstros** remains Basic Skill #9 and is exclusively the Monster Catalog consultation/access skill. It does **not** open Auction.
- The Auction entry belongs to the commerce/basic skill (`CUSTOM_BLACK_MARKET`, ID 9007), alongside its remote market/shop-search access.
- The public FreaokRO website/changelog must describe the commerce skill as the access point for **market/shop search + Auction**, while Basic Skill #9 remains **Monster Catalog only**.
- Added `monster_catalog_exp_enable` (default `1`) to `conf/import/battle_conf.txt`.
- Added GM command `@catalogxp on|off|status` (group level 99+). It changes the reward switch immediately without restarting the map-server.
- With Catalog EXP OFF, first-time discoveries, account knowledge, visual reveal, Sense result and Catalog registration continue normally; only Base/Job EXP payment is skipped. Rows are marked `reward_state=3` so turning rewards back ON later cannot retroactively pay discoveries made while rewards were disabled.
- **Client note:** rAthena's native Auction window is disabled for modern clients (PACKETVER >= 2014-11-12). The server-side commerce-skill route is wired through `openauction`, but the 2025-07-16 client will need the later Windows/client-side auction UI/replacement before this entry point is player-usable as a native auction window.

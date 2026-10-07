# FreaokRO V14.3 - Monster Identification / Catalog

## Final server-side rules
- Olhar do Caçador (`CUSTOM_MONSTER_READ`, 9001) identifies a target species.
- Knowledge is account-wide (`account_id + mob_id`). EXP is paid only to the active character that makes the first discovery.
- Normal: 100,000 Base + 100,000 Job base reward. Mini-Boss/MVP: 500,000 + 500,000.
- Normal character EXP bonuses are intentionally allowed.
- `@catalogxp on|off|status` is the emergency runtime kill-switch. `monster_catalog_exp_enable` is the startup setting.
- When EXP is OFF, discovery/visual reveal/Sense/catalog continue and no retroactive reward is created.
- Champions and regional bosses resolve to their underlying Mob ID; no extra species/reward.
- Slaves/summons, guardians and Emperium are not catalogable.
- Sense result is SELF only, never party-broadcast.
- Discovery cache is loaded once per login and used for per-viewer name masking; no SQL query per rendered mob.
- Basic Skill #9 (`CUSTOM_MONSTER_CATALOG`, 9008) opens only the Monster Catalog remote NPC UI.
- Auction belongs to the commerce/market skill, not the Monster Catalog.

## Global progression rule
FreaokRO globally disables multi-level. A single EXP grant can advance at most one Base Level and one Job Level. This is a core progression rule, not Catalog-only behavior.

## Client-side standby
The 2025-07-16 Ragexe client still needs Lua/skillinfo/icon work for custom skills and final element icon/HUD presentation. Server authority, discovery persistence, reward logic, private Sense and temporary Catalog UI are server-side.

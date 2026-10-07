# Card Drop Announcement & Audit Log V6

Server-side implementation inspired by the public behavior of Baad's 2021 MvP/Mini-Boss Card Ad & Log release, extended independently for this project.

## Categories
- NORMAL: normal monster instance.
- CHAMPION: any instance with custom_champion_type > 0.
- BOSS_MVP: CLASS_BOSS instances.

## Behavior
When a fresh monster DB drop is a Card, the server globally announces it and inserts an audit row into `custom_card_drop_log` with AID/CID, Mob ID, Item ID, category, Champion type, map, coordinates and timestamp.

Champion classification is based on the actual monster instance, not merely Mob ID, so the same species is logged differently when spawned as a project Champion.

## SQL
Apply `sql-files/custom_project.sql` to create `custom_card_drop_log`.

## Validation
`src/map/mob.cpp` was compiled directly with the project PACKETVER=20250716 flags: RC=0.
A complete `make map` was attempted but the environment time window expired while rebuilding third-party dependencies, before map-server link; therefore this package is not marked full-build validated.

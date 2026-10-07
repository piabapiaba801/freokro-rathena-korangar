# FreaokRO V14.5 - FUNK Runtime Apply

- @Funk switches are runtime map-server controls.
- Full Login/Char/Map restart is not required for feature toggles.
- Voice toggle now starts/stops its bridge and timer safely.
- AutoAttack OFF/MASTER OFF immediately stops active timers.
- Party Synergy toggle forces status recalculation for online characters.
- Durability toggle forces status recalculation and OFF restores full structural contribution without changing stored durability.
- GearProtect now honors its FUNK gate.
- Extended Vending currency command now honors its FUNK gate.
- @Funk remains an emergency runtime control; battle_conf values remain the startup defaults.

## Operational note
A process restart is still appropriate after binary/source updates, schema migrations that require it, or infrastructure failures. It is not part of ordinary FUNK ON/OFF switching.

## V14.6 pending binary deployment

New runtime keys: `arealoot`, `monsterread`, `craft`, `returnsave`,
`tame`, `ecommerce` and `leafwings`.

All start enabled in `conf/import/battle_conf.txt`. Their source integration is
complete, but the installed map-server gains these keys only after an explicitly
authorized compilation and restart.

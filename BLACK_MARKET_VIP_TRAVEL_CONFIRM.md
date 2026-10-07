# FreokRO - Black Market VIP / Travel confirmation

## Current behavior
- Group level 1+ (VIP): opens player/offline/Market Clone shops remotely.
- Group level 0 (free): clicking a search result does not open the shop remotely.
- Free users receive a confirmation dialog showing map, coordinates and travel cost.
- Initial travel cost: 10,000 Zeny.
- Selecting No cancels without charging or moving.
- Selecting Yes checks the configured cost, charges it and warps to the shop coordinates.
- NOWARP/NOWARPTO and job-entry restrictions are validated before the confirmation is opened.

## Configuration
`conf/import/battle_conf.txt`

- `black_market_teleport_fee_type: 1`
- `black_market_teleport_zeny: 10000`
- `black_market_teleport_item_id: 0`
- `black_market_teleport_item_amount: 1`

Fee type 0 = free, 1 = Zeny, 2 = item.

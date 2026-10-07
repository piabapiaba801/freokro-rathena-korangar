# FreokiRO AutoAttack V10

Server-side development implementation.

## Access tiers
- Free (tier 0): Basic + Skill, 1 skill slot; Hybrid blocked.
- Premium 1 (tier 1): Basic + Skill + Hybrid, 3 slots.
- Premium 2 (tier 2): Basic + Skill + Hybrid, 6 slots.
- Premium Full (tier 3): Basic + Skill + Hybrid, 9 slots.

The runtime tier switch (`@autoattack tier 0..3`) is intentionally restricted to GM level 99 and exists only for development/testing. It must be replaced by the contribution entitlement backend before production.

## Commands (development)
- `@autoattack on|off`
- `@autoattack mode basic|skill|hybrid`
- `@autoattack skill <slot> <skill_id> <level>`
- `@autoattack skill <slot> clear`
- `@autoattack skills`
- `@autoattack +<mob_id>` / `-<mob_id>`
- `@autoattack list` / `clear`

## Engine
A one-shot 250ms decision timer is rescheduled after each pass. It reacquires the player by runtime ID rather than retaining a player pointer. It stops on death/logout and pauses during NPC/chat/trade/vending/buying-store/storage interaction.

Target acquisition scans 9 cells for the nearest valid mob. Empty Mob ID filter means all mobs.

Skills use normal rAthena `unit_skilluse_id` / `unit_skilluse_pos`; therefore normal cast, SP/HP requirements, cooldown, after-cast, damage and FreokiRO Global Skill Critical remain authoritative. Skills must be learned at the configured level.

Self/support skills are supported server-side and have a generic 30-second anti-spam gate in addition to normal skill cooldowns. Ground/offensive skills target the selected monster. Party-target support automation is intentionally not part of V10 yet.

Hybrid falls back to basic attack when no configured skill acts. Skill mode never falls back to basic attack.

## Future frontend
Direct commands are temporary development controls. The intended production frontend is an item that opens a minimal HUD; the item/HUD should modify the same server-side AutoAttack state.

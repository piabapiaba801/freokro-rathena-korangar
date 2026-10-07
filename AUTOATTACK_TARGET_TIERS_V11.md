# FreokiRO AutoAttack - Target Filter Tiers (V11)

- Free (tier 0): no Mob-ID filter. AutoAttack always considers every valid monster in its search area.
- Premium 1 (tier 1): may select up to 2 Mob IDs.
- Premium 2 (tier 2): may select up to 5 Mob IDs.
- Premium Full (tier 3): target-filter cap is intentionally left uncapped until the production entitlement is defined.

Commands remain a development frontend. Future item/HUD will call the same server-authoritative state.

Runtime enforcement is applied both when editing the target list and when selecting/reusing a target. Downgrading to Free clears the filter; downgrading to a tier whose cap is below the current list also clears it rather than retaining an ambiguous subset.

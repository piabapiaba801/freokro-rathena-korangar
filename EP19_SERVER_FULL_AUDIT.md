# FreokRO Episode 19 server-side completion pass

Target client packet date: 2025-07-16. Client assets/testing intentionally excluded.

Corrections in this pass:
- Airship Destruction: corrected Aquila RES/MRES tiers to 0/200/400; corrected First Class rewards to 20 petals + 5 ore; reputation-tier petal bonuses to 2/3/7; removed unresolved reward placeholders; added explicit difficulty-scaled EXP/JEXP while preserving the existing Business baseline.
- Episode 19 main-story final reward: restored 130,609,489 Base EXP + 10,000,000 Job EXP.
- Simulation Battle: completed post-boss reward path with Snow Flower Ore, 4 petals, reputation bonus petals, reputation points, Base EXP and Job EXP.
- Confused Snake's Nest: filled the missing dialogue transition and made the previously undocumented touch trigger explicitly inert so it cannot double-advance the encounter.
- Existing EP19 mob definitions and mob-skill AI from the previous server-side repair are retained.

Evidence hierarchy used: official iRO/WarpPortal Episode 19 update pages where they expose exact rewards/requirements; cross-checks against current Episode 19 community documentation for encounter stats/flow. Values for which no authoritative exact number was exposed were not falsely labelled as kRO-exact.

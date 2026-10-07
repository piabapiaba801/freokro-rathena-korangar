# Episode 19 server-side fixes

Target client packet version: 20250716 (confirmed by project configure state and user Ragexe).

Changes applied:
- Completed active Episode 19 monster definitions in `db/import/mob_db.yml` for Isgard field/instance mobs used by the current scripts.
- Added EP19 combat AI skills to `db/import/mob_skill_db.txt`, including Aquila, Juncea, Simulation Juncea, Rgans, Limacina variants, Heart Hunter AT, Hallucigenia, Dollocaris and cave mobs.
- Added Airship Destruction box mob definitions (21853/21854) so the instance no longer references undefined mob IDs.
- Kept all changes server-side; no client/data/GRF edits were made.
- PACKETVER 20250716 was already present in the configured build and was not changed.

Validation:
- `db/import/mob_db.yml` parses successfully as YAML MOB_DB v5.
- EP19 spawn/instance mob references are covered by active definitions after the patch.
- Map-server compilation was started and progressed through common/map C++ compilation without a compiler error before the execution time limit stopped the build; this environment did not complete the final link in the allotted run.

Notes:
- Episode 19 upstream/community data contains some historically incomplete/approximate instance mechanics. This patch focuses on making the server-side mob database and combat behavior operational without client-side changes.

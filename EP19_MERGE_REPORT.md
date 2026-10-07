# FreaokRO Episode 19 intelligent merge

Source: uploaded rAthena `script/episode19` snapshot `60201913486d0aa8e093b929ae00ae54d593448e` / PR #8527.
Base: frozen Renew / Pre-EP19 copied to a separate working tree. Frozen backup was not modified.

Merged:
- 13 Episode 19 script/spawn/warp/merchant/barter files
- 4 instances
- Episode 19 main quest line
- 41 Episode 19 mob DB entries
- 208 quest DB entries
- DE_BERSERKAIZER skill DB + no-damage visual handling
- D_GW_EXTRACTOR item group + enum/constant
- Ice_F_Stone_Box2 and D_Gw_Extractor Laphine synthesis
- REPUTATION_EP19 = 4
- F_queststatus_between helper
- unitisforcewalk helper used by Airship Destruction
- loader registrations for scripts, monsters, warps and barters

Not blindly overwritten:
- FreaokRO custom C++ files (mob.cpp, pc.cpp, status.cpp, etc.)
- newer base data unrelated to Episode 19

Known upstream gap:
- PR #8527 still has an open report that mob skills are missing. The uploaded branch snapshot does not contain an Episode 19 mob_skill_db payload, so no fabricated mob-skill data was added.
- Upstream PR remains open/code-review; TODOs in the PR should be treated as upstream limitations, not silently invented here.

Validation performed:
- All modified/added YAML files parse successfully after merge.
- `skill.cpp`, `script.cpp`, and `itemdb.cpp` changed objects compiled successfully with the existing Linux toolchain.
- A full `make map` was started; it progressed through common and map compilation but exceeded the execution window before full link. No compile error was observed before timeout.
- The uploaded upstream Episode 19 quest DB contained unquoted `Title:` values with embedded colons that fail a strict YAML parser. Only the imported Episode 19 blocks were normalized by quoting those titles; the FreaokRO base DB was preserved.

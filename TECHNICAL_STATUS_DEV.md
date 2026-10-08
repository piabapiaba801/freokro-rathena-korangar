# FreokRO rAthena — development checkpoint — October 7, 2026

This file records the **initial FreokRO `dev` snapshot before any integration**. The user confirmed that this project has no earlier official Git base. At preservation preparation, the FreokRO remote had no branches, including `main`; a real rebase therefore had no base. Earlier Portuguese technical records remain in the physical recovery copy, while this English checkpoint is the document intended for Git. This workflow does not commit, push, or merge into `main`.

## Architecture and implemented work

- **IMPLEMENTED:** FreokRO's rAthena fork communicates with the separate Korangar client using `PACKETVER 20220406` without packet obfuscation. See `src/custom/defines_pre.hpp` and `src/config/packets.hpp`. The original Ragexe/EXE `20250716` installation is a different execution line.
- **IMPLEMENTED:** Episode 19 content and caveats are documented in `EP19_MERGE_REPORT.md` in the local source history. That report describes an earlier compilation of changed objects, but this checkpoint does not claim end-to-end EP 19 validation with the current Korangar client.
- **IMPLEMENTED:** custom skills and systems include Black Market, autoattack, advanced pets, party job synergy, and the custom auction phases. Relevant code is under `src/map/skills/custom/`; phase-specific notes remain in the local historical records.
- **IMPLEMENTED:** Black Market skill 7007 signals the separate Auction HUD. `map-server` owns session validation, inventory and Zeny checks, escrow, listings, bids, and fees. `src/map/skills/custom/skill_factory_custom.cpp` inserts `listing_fee` into `custom_auction` before charging the listing fee and reserving the item.
- **IMPLEMENTED:** the README maps the three repositories, explains their coupling, and provides official Windows dependency downloads. The HUD repository is private and optional for basic gameplay.

## Build and functional status

- **BUILDS:** previously built local `login-server.exe`, `char-server.exe`, and `map-server.exe` binaries exist. This checkpoint did not rebuild the full server from the current source; do not infer that every earlier change is present in those binaries.
- **WORKING:** on October 7, 2026, `sql-files/upgrades/upgrade_custom_auction_phase5.sql` was applied to the local `ragnarok` database. `custom_auction.listing_fee` and `custom_auction_history.sale_tax` / `seller_net` were confirmed.
- **WORKING:** an `INSERT` equivalent to the escrow path succeeded in a test transaction; `ROLLBACK` left the listing count unchanged. The user subsequently reported that escrow passed in the client after the migration. The agent did not independently inspect the final item and listing states from that client test.
- **WORKING:** login, char, and map servers started and listened on their expected local ports during earlier validation. Process state can change independently; this is a dated observation, not a runtime guarantee.
- **WORKING:** October 7 game screenshots showed the Auction HUD open inside a Korangar session and the item-description pop-up for Apple (ID 512). The user confirmed that the auction and client description windows can both be dragged. The HUD description remains inside the auction window by the user's latest preference.
- **PARTIAL:** escrow spans in-memory inventory and SQL. Check inventory and listing state before retrying an operation that reports an error.
- **NOT VALIDATED:** bid, buy, cancellation, tax settlement, item return, failure recovery, and all EP 19 interactions with the current client.
- **HAS ISSUES:** databases that have not applied the phase 5 auction migration fail with `Unknown column 'listing_fee' in 'INSERT INTO'` when listing an item.
- **PENDING:** rebuild the full server from this tree, validate the auction transaction lifecycle, and compare client/server behavior for custom skills and EP 19.

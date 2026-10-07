# Custom Auction - Phase 8 Hardening

- Bid SQL side-effects are transactional: auction CAS update, bid journal and displaced bidder wallet refund commit together.
- Buy Now uses the same transactional rule.
- On any SQL-side failure before COMMIT, the transaction is rolled back and the locally reserved Zeny is restored immediately.
- Existing account-level self-bid/self-buy protection remains enforced in the authoritative UPDATE predicates.
- `@auctionaudit` now reports stale pending settlements, historical self-bid invariant violations and invalid price rows.
- ESCROW_PENDING recovery remains deliberately manual: inventory state and SQL cannot be made a single ACID transaction by script alone, so automatic guessing would risk duplication.

## Remaining boundary
The character's in-memory Zeny and SQL are separate transactional domains in rAthena scripting. This phase minimizes failure windows but a process crash at the exact boundary between local Zeny mutation and SQL COMMIT still requires integration testing/recovery policy. A fully atomic cross-domain guarantee would require moving bidding/escrow into server-side C++/char-server persistence rather than script SQL.

# FreokRO Custom Auction House - Phase 1

Implemented foundation:
- Black Market Auction menu no longer opens the legacy client auction directly.
- Persistent InnoDB schema for auctions, bids and sale-price history.
- Item schema reserves full cards, refine, enchant grade, five Random Options, unique ID and FreokRO Durability.
- Menus: browse, my bids, my auctions, create, price history, server special auctions.
- FUNK runtime switch: `auction`.
- Player item creation is intentionally locked in Phase 1 until the atomic item escrow path is implemented; this prevents duplication/loss on crash/restart.

Next implementation phase:
1. atomic player item escrow;
2. bid Zeny escrow + automatic refund of displaced bid;
3. buy-now;
4. anti-sniping extension;
5. expiry worker + RodEx delivery;
6. sale tax/listing fee;
7. admin/server special-auction creation;
8. transaction and restart/crash recovery tests.

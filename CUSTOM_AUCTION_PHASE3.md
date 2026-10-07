# FreokRO Custom Auction - Phase 3

Implemented:
- Persistent Zeny escrow for bids.
- Minimum bid increment: max(1,000 Zeny, 5% of current price).
- Optimistic compare-and-swap on the auction row to prevent two simultaneous bids from both winning.
- Previous highest bidder is released to `custom_auction_wallet` persistently.
- Anti-sniping: a valid bid in the final 120 seconds extends the auction by 120 seconds.
- Buy Now atomically changes ACTIVE -> SETTLEMENT_PENDING and cannot remain biddable.
- Buy Now also releases the former highest bid to the persistent wallet.

Safety boundary:
- Phase 3 deliberately does not deliver the escrowed item or seller proceeds yet.
- SETTLEMENT_PENDING is a durable quarantine state for Phase 4 settlement/RodEx delivery.
- `custom_auction_wallet` is server-side escrow credit; withdrawal into character Zeny will be implemented together with crash-safe settlement so a restart cannot duplicate/refund twice.

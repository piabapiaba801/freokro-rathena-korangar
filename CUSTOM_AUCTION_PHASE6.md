# FreokRO Custom Auction - Phase 6: Server Special Auctions

- Adds administrator-created special auctions with `@serverauction` (group 99 only).
- Syntax: `@serverauction <item_id> <amount> <start_price> <buy_now|0> <hours> [refine]`.
- Duration is limited to 1..168 hours.
- Special auctions use the same bidding, buy-now, anti-sniping, persistent Zeny escrow and settlement pipeline as player auctions.
- The winning Zeny is a 100% server sink: there is no seller payout.
- Reward delivery remains persistent through RodEx.
- Server auctions with no bids now expire cleanly instead of waiting forever for a nonexistent seller return.
- Creator account/character and creation details are persisted in `custom_auction_admin_log` for audit.
- `Leiloes Especiais` shows official server auctions separately; they also remain available in the normal auction browser for bidding.
- Requires `upgrade_custom_auction_phase6.sql` on an existing database.

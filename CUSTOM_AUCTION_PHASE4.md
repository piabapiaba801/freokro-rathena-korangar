# FreokRO Custom Auction - Phase 4

Implemented persistent settlement and RodEx delivery.

- Expired auctions are processed in batches every 10 seconds.
- Winning bidder receives the exact escrow item snapshot through RodEx.
- Refine, cards, Random Options, unique_id, enchant grade and FreokRO durability are preserved in mail_attachments.
- Seller receives the final Zeny through RodEx.
- Auctions with no bids return the escrow item through RodEx.
- Outbid refunds are journaled in custom_auction_wallet and converted to RodEx Zeny by the settlement worker.
- Settlement uses InnoDB transactions and SELECT ... FOR UPDATE. Mail/attachment/history/status commit together.
- Restart/crash before COMMIT rolls back the settlement; after COMMIT it is already durable, preventing duplicate settlement on restart.
- Same-account rebids and Buy Now now reserve only the additional Zeny delta instead of charging the full amount twice.

Database upgrade: sql-files/upgrades/upgrade_black_market_custom_auction.sql

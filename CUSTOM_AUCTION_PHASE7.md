# Custom Auction Phase 7 - Administration & Recovery

Adds group-99 maintenance commands: `@auctioninspect`, `@auctioncancel`, `@auctionrecover`, and `@auctionaudit`.

Administrative cancellation is two-stage (`ACTIVE -> CANCEL_PENDING -> CANCELLED`). The persistent settlement worker atomically refunds the current leader into the auction wallet and returns a player-owned escrow item by RodEx. Server auctions have no item return. All admin cancellation/recovery actions are logged.

`@auctionrecover` only retries idempotent settlement/refund workers. It intentionally does not guess how to resolve old `ESCROW_PENDING` rows because a crash may have happened on either side of inventory removal; those rows are surfaced by `@auctionaudit` for manual inspection instead of risking duplication.

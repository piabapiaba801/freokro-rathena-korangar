-- FreaokRO Custom Auction Phase 7 - maintenance state.
ALTER TABLE `custom_auction`
  MODIFY `status` enum('ESCROW_PENDING','ACTIVE','SETTLEMENT_PENDING','CANCEL_PENDING','SOLD','EXPIRED','CANCELLED') NOT NULL DEFAULT 'ACTIVE';

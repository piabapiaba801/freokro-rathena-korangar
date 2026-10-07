# FreaokRO V13 - Market Clone 2.0 / Extended Vending

Implemented on the post-V12 working copy. V12 Backup Pre-Renewal remains untouched.

## Currency support
- Zeny
- Cash Points
- Kafra Points
- Custom item/token currency by Item ID

## Persistence
- Standard vending persists `extended_vending_item` in `vendings`.
- Autotrade restores the selected currency after server restart.
- Market Clone persists the same currency in `vending_clones.currency`.
- Item-token clone sales pay the seller through RodEx.
- Cash/Kafra clone sales queue point payouts in `vending_clone_point_payouts` and deliver them on login.

## Selection
Temporary server-side selector: `@vendcurrency zeny|cash|kafra|<ItemID>`.
This is a configuration entry point only; purchase/payment/persistence are native server-side flows. A later client UI can replace the selector without changing the economy backend.

## Durability compatibility
Existing 100% durability requirement for vending remains intact.

## Build verification
- map-server: linked successfully on Linux.
- char-server: linked successfully on Linux.
- Live MySQL/client transaction test still requires runtime environment.

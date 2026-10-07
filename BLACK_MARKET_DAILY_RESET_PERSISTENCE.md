# FreokRO Black Market - Daily Reset / Persistent Quota

- Free accounts: 10 successful shop travels per daily reset.
- VIP/group level >= 1: 50 remote shop accesses per daily reset.
- Reset: 00:00 according to the SQL/database server calendar (`CURDATE()`).
- Authoritative state: SQL table `custom_black_market_quota`, keyed by `account_id`.
- The counter is persisted immediately before granting travel/remote access.
- Map-server crash/restart does not reset quota.
- SQL errors fail closed instead of granting an untracked use.
- Free travel refunds the configured fee if quota persistence fails.
- Old rolling-window account variables are no longer authoritative.

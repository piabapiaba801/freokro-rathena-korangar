# FreokRO AutoAttack - Weekly Availability

Status: implemented server-side.

- Available Monday through Friday.
- Friday remains available through 23:59:59 server-local time.
- Saturday and Sunday are blocked.
- Monday 00:00:00 is automatically available again.
- `@autoattack on` refuses activation on weekends.
- `autoattack_timer` checks the rule while AutoAttack is running.
- A Friday session crossing into Saturday is stopped automatically on the first timer cycle after midnight.
- Target is cleared and automatic attack/walking are stopped.
- FUNK master/AutoAttack switches remain authoritative and must also be enabled.
- Time source is the map-server host local clock.
- No server restart is required for the weekly transition.

Validation pending: Windows x64 Release build and runtime boundary tests.

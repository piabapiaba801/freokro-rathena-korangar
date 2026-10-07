# FreokRO x86-64 Release build

Fast native Linux build profile for the server.

- x86-64 (`-m64`)
- Release optimization (`-O2 -DNDEBUG`)
- Debug disabled
- LTO disabled to reduce compile/link time and peak RAM
- PACKETVER fixed at `20250716`
- Incremental Make builds preserved
- Default parallelism: 2 jobs (override with `JOBS=N`)

Clean configure/build:

    make clean || true
    rm -f config.status config.log
    JOBS=2 ./build-release-x64.sh map

Incremental rebuild:

    JOBS=2 ./build-release-x64.sh map

For a machine with more RAM/cores, increase JOBS conservatively.

## Black Market player-shop search update
- `CUSTOM_BLACK_MARKET` now exposes a real player-shop finder from `Mercado / Busca de Lojas`.
- The finder opens native Search Store in remote mode across all maps (`searchstores 255,1,"all"`).
- Standard online vending and rAthena autotrade/offline vending are returned through `vending_db`.
- FreokRO persistent Market Clones (`@autotrade2`) are additionally indexed by the Search Store core.
- Clicking a Market Clone result opens the clone shop remotely.
- Existing SQL Black Market stock and Auction paths are preserved.

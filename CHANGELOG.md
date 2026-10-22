# Changelog

## [0.4.3] - 2026-10-22

- Bind claim-name SQL lookups and use exact operator/member identity.
- Commit rename, child detachment and resize atomically; publish prepared claim/index snapshots after commit.
- Check economy results, record durable payment intent/outcomes and refuse automatic replay of uncertain payments. Delete refunds follow successful deletion.
- Normalize block coordinates with floor, use inclusive wide area arithmetic, bound coordinate/span/index work.
- Protect piston boundary neighborhoods conservatively on SDKs without moved-block lists.
- Isolate flight/session ownership, preserve pre-existing grants, reconcile dimensions and clean quit/reload/disable state.
- Add C++ SQLite/policy/index/failure regression tests and exact-commit Linux/Windows release provenance.

### Upgrade notes

Area is now inclusive of both endpoints and affects pricing/refunds. Flight additionally requires explicit `fly_exclusive_authority: true` and must not overlap another flight provider. Piston restrictions can conservatively block devices within 13 blocks of protected boundaries. Read [SAFETY_CONTRACTS](docs/SAFETY_CONTRACTS.md) and back up the database before upgrading. Real BDS integration and cross-provider crash atomicity are not claimed.

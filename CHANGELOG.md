# Changelog

## [0.4.4] - 2026-10-23

- Restore the pre-0.4.3 piston-body edge policy; remove the 13-block three-dimensional exclusion halo that disabled machines in low or narrow claims.
- Preserve dimension isolation, child-claim priority and edge-piston permissions without guessing moved blocks.
- Restore legacy coordinate truncation and non-inclusive area pricing while retaining checked wide arithmetic and transaction safety.
- Keep stored claim data intact; no automatic recircling, coordinate migration or reversal of previous payments.
- Add compatibility regressions for low-height claims, boundary pistons, child permissions and fractional coordinates.

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

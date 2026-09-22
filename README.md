# RHESSys Water-Balance Audit Copy

This directory contains my working copy of RHESSysEastCoast for checking and correcting water-balance closure. I kept the original source and production executable unchanged; all experimental diagnostics and fixes were developed and tested in this copied source tree.

## Work completed so far

I traced the main water inputs, outputs, storage changes, and lateral transfers from patch to basin scale, then added an independent basin water ledger and focused patch/canopy diagnostics. This showed that the original built-in patch balance used stale or uninitialized beginning-of-day storage fields, so I replaced those checks with consecutive state snapshots and explicit flux counters.

The audit localized and tested the following water-conservation fixes:

- Groundwater diversion during lateral routing is now applied once to the donor volume before the remaining water is distributed among neighboring patches. Previously, it was applied repeatedly inside the neighbor loop.
- Pending groundwater routed from a patch is transferred to hillslope groundwater even when the patch has no detention storage on the following day. Previously, this water could be discarded on dry days.
- Daily and hourly canopy interception functions are evaluated once before applying the `max` bound. The original macro expression could call a state-changing interception function twice.
- Evaporated interception is removed before new canopy storage is calculated, and existing canopy water is recorded as evaporation before its storage is set to zero.

I also added diagnostic-only fields and outputs for canopy storage, precipitation, evapotranspiration, patch storage, groundwater exchange, lateral routing, and stream export. These additions are intended to expose each term in

```text
water-balance residual = input - external output - change in storage
```

while keeping internal patch-to-patch transfers separate from true basin exports.

## Verification

The fixes were tested with short Slurm smoke tests and a four-year simulation using 1999 as warm-up and 2000--2003 as the analysis period.

| Result | Before the complete fixes | After the complete fixes |
|---|---:|---:|
| Four-year cumulative residual | +557.660661 mm | -0.004060 mm |
| Residual relative to precipitation | +7.3976% | -0.0000539% |
| Maximum absolute daily residual | 3.096977 mm | 0.0000234 mm |

The remaining residual is at numerical/ledger scale rather than the original structural imbalance. Individual corrected canopy calls closed within `1.36e-17 m` in the focused test.

## Important limits

This is still an audit and development copy, not the production model. The lateral-routing water correction changes the partitioning between return flow and groundwater/baseflow, so it should receive scientific review before production use. Related solute routing still needs a separate carbon and nitrogen conservation audit. The supplied test watershed also has zero qualifying riparian receiving area, so the groundwater-to-riparian branch has not yet been exercised.

Detailed methods, code comparisons, and results are documented in:

- `../../../notes/process_map.md`
- `../../../notes/water_accounting_dictionary.csv`
- `../../../notes/critical_bugs.md`
- `../../../notes/original_vs_fixed_code.md`
- `../../../notes/four_year_fix_test.md`
- `../../../diagnostics/processed/canopy_complete_fix_4y_summary.csv`

This copy is based on the RHESSysEastCoast branch of [RHESSys](https://github.com/RHESSys/RHESSys).

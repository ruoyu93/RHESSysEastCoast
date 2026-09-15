/*--------------------------------------------------------------*/
/*								*/
/*		compute_basin_water_balance			*/
/*								*/
/*	NAME							*/
/*	compute_basin_storage - area-weighted mean patch storage	*/
/*	compute_basin_gw_storage - area-weighted mean hillslope gw storage */
/*	compute_basin_wbal_fluxes - area-weighted mean patch input/output fluxes */
/*								*/
/*	SYNOPSIS						*/
/*	double compute_basin_storage(struct basin_object *)	*/
/*	double compute_basin_gw_storage(struct basin_object *)	*/
/*	void compute_basin_wbal_fluxes(struct basin_object *,	*/
/*			double *input, double *output)		*/
/*								*/
/*	DESCRIPTION						*/
/*	Helpers for the full-day, whole-basin water balance check	*/
/*	(basin[0].water_balance, computed in basin_daily_F()).	*/
/*	compute_basin_storage sums, over every patch in the basin,	*/
/*	the same storage terms that patch_daily_F()'s own		*/
/*	patch[0].water_balance uses (rz_storage, unsat_storage,	*/
/*	detention_store, snowpack, canopy rain/snow storage, minus	*/
/*	sat_deficit), area-weighted and normalized by basin area, so	*/
/*	it can be called once before the day's physics runs and	*/
/*	once after, to get a true day-start/day-end snapshot.		*/
/*	compute_basin_gw_storage does the same for the hillslope-	*/
/*	level deep groundwater store (hillslope[0].gw.storage),	*/
/*	which is a real water pool fed by patch[0].gw_drainage and	*/
/*	drained by hillslope base flow -- it must be included as a	*/
/*	storage term, not ignored, or the balance will show a	*/
/*	spurious residual.						*/
/*	compute_basin_wbal_fluxes sums patch[0].wbal_input and	*/
/*	patch[0].wbal_output (set once per day in patch_daily_F(),	*/
/*	before routing runs) over the whole basin.			*/
/*--------------------------------------------------------------*/
#include <stdio.h>
#include "rhessys.h"

double compute_basin_storage(struct basin_object *basin)
{
	int h, z, p;
	struct hillslope_object *hillslope;
	struct zone_object *zone;
	struct patch_object *patch;
	double storage;

	storage = 0.0;
	for (h = 0; h < basin[0].num_hillslopes; h++) {
		hillslope = basin[0].hillslopes[h];
		for (z = 0; z < hillslope[0].num_zones; z++) {
			zone = hillslope[0].zones[z];
			for (p = 0; p < zone[0].num_patches; p++) {
				patch = zone[0].patches[p];
				storage += (patch[0].rz_storage + patch[0].unsat_storage
						+ patch[0].detention_store - patch[0].sat_deficit
						+ patch[0].snowpack.water_depth
						+ patch[0].snowpack.water_equivalent_depth
						+ patch[0].rain_stored + patch[0].snow_stored)
						* patch[0].area;
			}
		}
	}
	return (storage / basin[0].area);
}

double compute_basin_gw_storage(struct basin_object *basin)
{
	int h;
	struct hillslope_object *hillslope;
	double gw_storage;

	gw_storage = 0.0;
	for (h = 0; h < basin[0].num_hillslopes; h++) {
		hillslope = basin[0].hillslopes[h];
		gw_storage += hillslope[0].gw.storage * hillslope[0].area;
	}
	return (gw_storage / basin[0].area);
}

void compute_basin_wbal_fluxes(struct basin_object *basin, double *input, double *output)
{
	int h, z, p;
	struct hillslope_object *hillslope;
	struct zone_object *zone;
	struct patch_object *patch;

	*input = 0.0;
	*output = 0.0;
	for (h = 0; h < basin[0].num_hillslopes; h++) {
		hillslope = basin[0].hillslopes[h];
		for (z = 0; z < hillslope[0].num_zones; z++) {
			zone = hillslope[0].zones[z];
			for (p = 0; p < zone[0].num_patches; p++) {
				patch = zone[0].patches[p];
				*input += patch[0].wbal_input * patch[0].area;
				*output += patch[0].wbal_output * patch[0].area;
			}
		}
	}
	*input /= basin[0].area;
	*output /= basin[0].area;
	return;
} /*end compute_basin_water_balance.c*/

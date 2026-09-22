/*--------------------------------------------------------------*/
/* 																*/
/*					output_patch						*/
/*																*/
/*	output_patch - creates output files objects.		*/
/*																*/
/*	NAME														*/
/*	output_patch - outputs current contents of a patch.			*/
/*																*/
/*	SYNOPSIS													*/
/*	void	output_patch(										*/
/*					struct	patch_object	*patch,				*/
/*					struct	date	date,  						*/
/*					FILE 	*outfile)							*/
/*																*/
/*	OPTIONS														*/
/*																*/
/*	DESCRIPTION													*/
/*																*/
/*	outputs spatial structure according to commandline			*/
/*	specifications to specific files							*/
/*																*/
/*	PROGRAMMER NOTES											*/
/*																*/
/*	We only permit one fileset per spatial modelling level.     */
/*	Each fileset has one file for each timestep.  				*/
/*																*/
/*--------------------------------------------------------------*/
#include <stdio.h>
#include "rhessys.h"

void	output_patch(
					 int basinID, int hillID, int zoneID,
					 struct	patch_object	*patch,
					 struct	zone_object	*zone,
					 struct	date	current_date,
					 FILE *outfile)
{
	/*------------------------------------------------------*/
	/*	Local Function Declarations.						*/
	/*------------------------------------------------------*/
	
	/*------------------------------------------------------*/
	/*	Local Variable Definition. 							*/
	/*------------------------------------------------------*/
	int check;
	double total_et;
    double storage_change, water_inputs, water_outputs;

	/* One aggregate ET output avoids duplicating its six components. */
	total_et = patch[0].evaporation + patch[0].evaporation_surf
		+ patch[0].transpiration_sat_zone + patch[0].transpiration_unsat_zone
		+ patch[0].exfiltration_sat_zone + patch[0].exfiltration_unsat_zone;

    /* scripts/check_patch_satdef_balance.py accounting, using daily initial
     * snapshots instead of the previous output record. All terms are meters
     * of water over patch area. Saturated storage is soil_water_cap minus
     * sat_deficit; the constant soil_water_cap cancels in its change.
     * Use the dedicated daily deficit: routing overwrites preday_sat_deficit.
     * At daily output this includes routing; hourly calls are partial-day.
     */
    storage_change = (patch[0].rain_stored - patch[0].preday_rain_stored)
        + (patch[0].snow_stored - patch[0].preday_snow_stored)
        + (patch[0].snowpack.water_depth
            + patch[0].snowpack.water_equivalent_depth - patch[0].preday_snowpack)
        + (patch[0].detention_store - patch[0].preday_detention_store)
        + (patch[0].rz_storage - patch[0].preday_rz_storage)
        + (patch[0].unsat_storage - patch[0].preday_unsat_storage)
        + (patch[0].water_dl_day_start_sat_deficit - patch[0].sat_deficit);
    water_inputs = zone[0].rain + patch[0].water_dl_snow_input
        + zone[0].rain_hourly_total + patch[0].water_dl_dated_irrigation
        + patch[0].grassIrrigation_m + patch[0].septicReleaseQ_m
        + patch[0].water_dl_gw_to_riparian
        + patch[0].Qin_total + patch[0].surface_Qin_total;
    water_outputs = patch[0].water_dl_vertical_gw
        + patch[0].surface_Qout_total + patch[0].streamflow
        + patch[0].stormdrained + patch[0].sewerdrained
        + patch[0].pipedrainYield + total_et
        + patch[0].water_dl_stream_subsurface_out_m3 / patch[0].area;
    /* Streamflow includes the stream-converted fraction of stream-patch
     * subsurface Qout; water continuing to neighbour soil is recorded by
     * water_dl_stream_subsurface_out_m3 above. routing_to_gw_subset is
     * already included in routed exports.
     */
    if (patch[0].drainage_type != 1)
        water_outputs += patch[0].Qout_total;
    patch[0].water_balance = water_inputs - water_outputs - storage_change;

	check = fprintf(outfile,
		"%d %d %d %d %lf %d"
		" %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf"
		" %lf %lf %lf %lf %lf %lf %lf %lf %lf"
		" %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %.6g\n",
		current_date.year,
		current_date.month,
		current_date.day,
		patch[0].ID,
		patch[0].area,
		patch[0].drainage_type,
		patch[0].soil_defaults[0][0].soil_water_cap * 1000.0,
		patch[0].rootzone.potential_sat * 1000.0,
		patch[0].sat_deficit * 1000.0,
		patch[0].sat_deficit_z * 1000.0,
		patch[0].rain_stored * 1000.0,
		patch[0].snow_stored * 1000.0,
		patch[0].snowpack.water_depth * 1000.0,
		patch[0].snowpack.water_equivalent_depth * 1000.0,
		patch[0].detention_store * 1000.0,
		patch[0].rz_storage * 1000.0,
		patch[0].unsat_storage * 1000.0,
		zone[0].rain * 1000.0,
		patch[0].water_dl_snow_input * 1000.0,
		zone[0].rain_hourly_total * 1000.0,
		patch[0].water_dl_dated_irrigation * 1000.0,
		patch[0].grassIrrigation_m * 1000.0,
		patch[0].septicReleaseQ_m * 1000.0,
		patch[0].water_dl_gw_to_riparian * 1000.0,
		patch[0].water_dl_vertical_gw * 1000.0,
		patch[0].Qin_total * 1000.0,
		patch[0].Qout_total * 1000.0,
		patch[0].water_dl_stream_subsurface_out_m3 / patch[0].area * 1000.0,
		patch[0].water_dl_from_stream_subsurface_m3 / patch[0].area * 1000.0,
		patch[0].surface_Qin_total * 1000.0,
		patch[0].surface_Qout_total * 1000.0,
		patch[0].streamflow * 1000.0,
		patch[0].stormdrained * 1000.0,
		patch[0].sewerdrained * 1000.0,
		patch[0].pipedrainYield * 1000.0,
		total_et * 1000.0,
		patch[0].water_dl_routing_gw_m3 / patch[0].area * 1000.0,
        patch[0].water_balance * 1000.0); /* meters to millimeters */

	if (check <= 0) {
		fprintf(stdout, "\nWARNING: output error has occured in output_patch, file");
	}
	return;
} /*end output_patch*/

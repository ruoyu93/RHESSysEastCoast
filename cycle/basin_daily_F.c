/*--------------------------------------------------------------*/
/* 																*/
/*						basin_daily_F							*/
/*																*/
/*	NAME														*/
/*	basin_daily_F												*/
/*			 - performs cycling and output of a basin			*/
/*				for beginning of the  day 						*/
/*				with hillslopes simulated in parallel			*/
/*																*/
/*																*/
/*	SYNOPSIS													*/
/*	void basin_daily_F( 										*/
/*						 long	,								*/
/*						 struct world_object *,					*/
/*						 struct basin_object *,					*/
/*						 struct command_line_object *,			*/
/*						 struct tec_entry *,					*/
/*						 struct date)							*/
/*																*/
/*	OPTIONS														*/
/*																*/
/*	DESCRIPTION													*/
/*																*/
/*	This routine performs simulation cycles on an identified	*/
/*	hillslopes in the basin. The routine also prints out results*/
/*	where specified by current tec events files.				*/
/*																*/
/*	Computes day length at the latitude of the basin.			*/
/*	Executes hillslope daily simulation for all hillslopes. 	*/
/*																*/
/*	PROGRAMMER NOTES											*/
/*																*/
/*																*/
/*	The execution of simulate_hillslope_daily could be done in 	*/
/*	parallel with one processor per hillslope.  				*/
/*																*/
/*	If message passing is required during the simulate_hillslope*/
/*	_daily processes a message structure could be created in 	*/
/*	the basin_daily array.										*/
/*																*/
/*--------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rhessys.h"

static void write_basin_water_ledger(
	struct basin_object *basin,
	struct command_line_object *command_line,
	struct date current_date)
{
	static FILE *ledger = NULL;
	static int disabled = 0;
	const char *filename;
	int h, z, p;
	struct hillslope_object *hillslope;
	struct zone_object *zone;
	struct patch_object *patch;
	double area = 0.0;
	double rain = 0.0, snow = 0.0, hourly_rain = 0.0;
	double grass_irrigation = 0.0, septic_release = 0.0;
	double et = 0.0, stream = 0.0, storm = 0.0, sewer = 0.0, pipe = 0.0;
	double gw_qout = 0.0, gw_storage = 0.0, pending_gw = 0.0;
	double canopy_litter = 0.0, canopy_snow = 0.0;
	double canopy_rain = 0.0, litter_rain = 0.0, rain_throughfall = 0.0;
	double canopy_call_input = 0.0, canopy_call_output = 0.0;
	double canopy_call_old_storage = 0.0, canopy_call_new_storage = 0.0;
	double canopy_call_evaporation = 0.0, canopy_call_residual = 0.0;
	double snow_liquid = 0.0, snow_ice = 0.0, detention = 0.0;
	double rz = 0.0, unsat = 0.0, saturated = 0.0, total_storage;
	int hourly_routing = 0;

	if (disabled)
		return;
	if (ledger == NULL) {
		filename = getenv("RHESSYS_BASIN_WATER_DIAG");
		if (filename == NULL || filename[0] == '\0') {
			disabled = 1;
			return;
		}
		ledger = fopen(filename, "w");
		if (ledger == NULL) {
			fprintf(stderr, "Unable to open basin water ledger %s.\n", filename);
			exit(EXIT_FAILURE);
		}
		fprintf(ledger,
			"year,month,day,basin_id,area_m2,hourly_routing_flag,"
			"rain_m3,snow_m3,hourly_rain_m3,grass_irrigation_m3,"
			"septic_release_m3,et_m3,stream_export_m3,gw_export_m3,"
			"storm_export_m3,sewer_export_m3,pipe_export_m3,"
			"canopy_litter_storage_m3,canopy_rain_storage_m3,"
			"litter_rain_storage_m3,rain_throughfall_m3,"
			"canopy_call_input_m3,canopy_call_output_m3,"
			"canopy_call_old_storage_m3,canopy_call_new_storage_m3,"
			"canopy_call_evaporation_m3,canopy_call_residual_m3,"
			"canopy_snow_storage_m3,"
			"snow_liquid_storage_m3,snow_ice_storage_m3,"
			"detention_storage_m3,rz_storage_m3,unsat_storage_m3,"
			"saturated_storage_m3,hillslope_gw_storage_m3,"
			"pending_routing_gw_storage_m3,total_storage_m3\n");
	}

	for (h = 0; h < basin[0].num_hillslopes; h++) {
		hillslope = basin[0].hillslopes[h];
		gw_storage += hillslope[0].gw.storage * hillslope[0].area;
		gw_qout += hillslope[0].gw.Qout * hillslope[0].area;
		for (z = 0; z < hillslope[0].num_zones; z++) {
			zone = hillslope[0].zones[z];
			for (p = 0; p < zone[0].num_patches; p++) {
				patch = zone[0].patches[p];
				if (zone[0].hourly_rain_flag == 1)
					hourly_routing = 1;
				area += patch[0].area;
				rain += zone[0].rain * patch[0].area;
				snow += zone[0].snow
					* (command_line[0].snow_scale_flag == 1
						? patch[0].snow_redist_scale : 1.0)
					* patch[0].area;
				hourly_rain += zone[0].rain_hourly_total * patch[0].area;
				grass_irrigation += patch[0].grassIrrigation_m * patch[0].area;
				septic_release += patch[0].septicReleaseQ_m * patch[0].area;
				et += (patch[0].evaporation + patch[0].evaporation_surf
					+ patch[0].transpiration_sat_zone
					+ patch[0].transpiration_unsat_zone
					+ patch[0].exfiltration_sat_zone
					+ patch[0].exfiltration_unsat_zone) * patch[0].area;
				stream += patch[0].streamflow * patch[0].area;
				storm += patch[0].stormdrained * patch[0].area;
				sewer += patch[0].sewerdrained * patch[0].area;
				pipe += patch[0].pipedrainYield * patch[0].area;
				canopy_litter += patch[0].rain_stored * patch[0].area;
				litter_rain += patch[0].litter.rain_stored * patch[0].area;
				canopy_rain +=
					(patch[0].rain_stored - patch[0].litter.rain_stored)
					* patch[0].area;
				rain_throughfall +=
					patch[0].rain_throughfall * patch[0].area;
				canopy_call_input +=
					patch[0].water_diag_canopy_input * patch[0].area;
				canopy_call_output +=
					patch[0].water_diag_canopy_output * patch[0].area;
				canopy_call_old_storage +=
					patch[0].water_diag_canopy_old_storage * patch[0].area;
				canopy_call_new_storage +=
					patch[0].water_diag_canopy_new_storage * patch[0].area;
				canopy_call_evaporation +=
					patch[0].water_diag_canopy_evaporation * patch[0].area;
				canopy_call_residual +=
					patch[0].water_diag_canopy_residual * patch[0].area;
				canopy_snow += patch[0].snow_stored * patch[0].area;
				snow_liquid += patch[0].snowpack.water_depth * patch[0].area;
				snow_ice += patch[0].snowpack.water_equivalent_depth
					* patch[0].area;
				detention += patch[0].detention_store * patch[0].area;
				rz += patch[0].rz_storage * patch[0].area;
				unsat += patch[0].unsat_storage * patch[0].area;
				saturated +=
					(patch[0].soil_defaults[0][0].soil_water_cap
						- patch[0].sat_deficit) * patch[0].area;
				/*
				 * After routing this field is a pending volume that is
				 * credited to hillslope groundwater on the following day.
				 */
				pending_gw += patch[0].gw_drainage;
			}
		}
	}

	total_storage = canopy_litter + canopy_snow + snow_liquid + snow_ice
		+ detention + rz + unsat + saturated + gw_storage + pending_gw;
	if (hourly_routing)
		stream = basin[0].water_diag_hourly_stream_m3;
	fprintf(ledger,
		"%ld,%ld,%ld,%d,%.17g,%d,"
		"%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,"
		"%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,"
		"%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,"
		"%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,"
		"%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,"
		"%.17g\n",
		current_date.year, current_date.month, current_date.day, basin[0].ID,
		area, hourly_routing, rain, snow, hourly_rain, grass_irrigation,
		septic_release,
		et, stream, gw_qout, storm, sewer, pipe,
		canopy_litter, canopy_rain, litter_rain, rain_throughfall,
		canopy_call_input, canopy_call_output,
		canopy_call_old_storage, canopy_call_new_storage,
		canopy_call_evaporation, canopy_call_residual,
		canopy_snow, snow_liquid, snow_ice, detention,
		rz, unsat, saturated, gw_storage, pending_gw, total_storage);
	fflush(ledger);
}

void	basin_daily_F(
					  long	day,
					  struct	world_object	*world,
					  struct	basin_object 	*basin,
					  struct	command_line_object *command_line,
					  struct	tec_entry		*event,
					  struct	date			current_date)
{
	/*--------------------------------------------------------------*/
	/*  Local Function Declarations.                                */
	/*--------------------------------------------------------------*/
	void hillslope_daily_F(
		long,
		struct	world_object *,
		struct 	basin_object *,
		struct	hillslope_object *,
		struct	command_line_object *, 
		struct	tec_entry *,
		struct	date );
	/*--------------------------------------------------------------*/
	/* this part has been moved to basin_hourly			*/
	/*--------------------------------------------------------------*/
	void	compute_subsurface_routing(
		struct command_line_object *,
		struct basin_object *,
		int, struct	date);
	
	double	compute_stream_routing(
		struct command_line_object *,
		struct stream_network_object *,
		int, struct	date);

	void	update_basin_patch_accumulator(
		struct command_line_object *command_line,
		struct basin_object *basin,
		struct date current_date);

	void	update_hillslope_accumulator(
		struct command_line_object *command_line,
		struct basin_object *basin);
	/*--------------------------------------------------------------*/
	/*  Local variable definition.                                  */
	/*--------------------------------------------------------------*/
	int	h, z, p,inx;
	double	scale;
	struct	hillslope_object *hillslope;
	struct	zone_object *zone;
	struct	patch_object *patch; 
	struct	dated_sequence	clim_event;

	/*--------------------------------------------------------------*/
	/* 	track basin scale snow for snow assimilation 			*/
	/*--------------------------------------------------------------*/
	basin[0].area_withsnow=0;
	basin[0].snowpack.surface_age = 0.0;
	basin[0].snowpack.energy_deficit = 0.0;
	basin[0].snowpack.T = 0.0;
	/*--------------------------------------------------------------*/
	/*	Simulate the hillslopes in this basin for the whole day		*/
	/*--------------------------------------------------------------*/

	for ( h = 0 ; h < basin[0].num_hillslopes; h ++ ){
		hillslope_daily_F(	day,
			world,
			basin,
			basin[0].hillslopes[h],
			command_line, 
			event,
			current_date );
	}

        hillslope = basin[0].hillslopes[0];
	zone = hillslope[0].zones[0];
	basin[0].snowpack.surface_age /=  basin[0].area_withsnow;
	basin[0].snowpack.T /=  basin[0].area_withsnow;
	basin[0].snowpack.energy_deficit /=  basin[0].area_withsnow;


	/*--------------------------------------------------------------*/
	/*  For routing option - route water between patches within     */
	/*      the basin:  this part has been moved to basin_hourly    */
	/*--------------------------------------------------------------*/
   	if ( command_line[0].routing_flag == 1 && zone[0].hourly_rain_flag == 0) {
		compute_subsurface_routing(command_line,
			basin,
			basin[0].defaults[0][0].n_routing_timesteps,
			current_date);
	}
	
	/*--------------------------------------------------------------*/
	/*  For stream routing option - route water between patches within     */
	/*      the basin                                               */
	/*--------------------------------------------------------------*/
	if ( command_line[0].stream_routing_flag == 1) {
		 basin[0].stream_list.streamflow=compute_stream_routing(command_line,
			basin[0].stream_list.stream_network,
			basin[0].stream_list.num_reaches,
                        current_date);
	}

	write_basin_water_ledger(basin, command_line, current_date);

	/*--------------------------------------------------------------*/
	/* update basin patch accumulator				*/
	/*--------------------------------------------------------------*/
    if(command_line[0].output_flags.yearly == 1 || command_line[0].output_flags.monthly == 1) update_basin_patch_accumulator(command_line,
                    basin,
                    current_date);

	/*--------------------------------------------------------------*/
	/* update hillslope accumulator					*/
	/*--------------------------------------------------------------*/
//    update_hillslope_accumulator(command_line,
//                    basin);

	return;
} /*end basin_daily_F*/

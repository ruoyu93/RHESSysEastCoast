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
#include <math.h>
#include "rhessys.h"

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

	double	compute_basin_storage(struct basin_object *);
	double	compute_basin_gw_storage(struct basin_object *);
	void	compute_basin_wbal_fluxes(struct basin_object *, double *, double *);
	/*--------------------------------------------------------------*/
	/*  Local variable definition.                                  */
	/*--------------------------------------------------------------*/
	int	h, z, p,inx;
	double	scale;
	double	wbal_start_storage, wbal_start_gw;
	double	wbal_end_storage, wbal_end_gw, wbal_input, wbal_output;
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
	/*	basin[0].water_balance (full-day check) only makes sense for	*/
	/*	the explicit daily-routing configuration: it relies on a	*/
	/*	single start-of-day/end-of-day snapshot bounding "one day",	*/
	/*	which isn't true under hourly rain (physics already ran	*/
	/*	across today's hours via world_hourly()/basin_hourly() before	*/
	/*	this function is even called), and basin_outflow is never	*/
	/*	populated at all under routing_flag==0 (TOPMODEL).		*/
	/*--------------------------------------------------------------*/
	hillslope = basin[0].hillslopes[0];
	zone = hillslope[0].zones[0];

	if (command_line[0].routing_flag == 1 && zone[0].hourly_rain_flag == 0) {
		wbal_start_storage = compute_basin_storage(basin);
		wbal_start_gw = compute_basin_gw_storage(basin);
	}

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

	/*--------------------------------------------------------------*/
	/*	full-day, whole-basin water balance check: true day-start	*/
	/*	storage vs. true day-end storage, reconciled against the	*/
	/*	day's rain/irrigation/septic inputs, ET/gw_drainage outputs,	*/
	/*	the hillslope groundwater store, and basin outflow.		*/
	/*--------------------------------------------------------------*/
	if (command_line[0].routing_flag == 1 && zone[0].hourly_rain_flag == 0) {
		wbal_end_storage = compute_basin_storage(basin);
		wbal_end_gw = compute_basin_gw_storage(basin);
		compute_basin_wbal_fluxes(basin, &wbal_input, &wbal_output);
		basin[0].water_balance = wbal_input - wbal_output
				- (wbal_end_storage - wbal_start_storage)
				- (wbal_end_gw - wbal_start_gw)
				- basin[0].basin_outflow;
	} else {
		basin[0].water_balance = 0.0; /* not computed in this routing configuration */
	}

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

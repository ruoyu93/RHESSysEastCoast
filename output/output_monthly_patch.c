/*--------------------------------------------------------------*/
/* 																*/
/*					output_monthly_patch						*/
/*																*/
/*	output_monthly_patch - creates output files objects.		*/
/*																*/
/*	NAME														*/
/*	output_monthly_patch - outputs current contents of a patch.			*/
/*																*/
/*	SYNOPSIS													*/
/*	void	output_monthly_patch(										*/
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

void	output_monthly_patch(
							 int basinID, int hillID, int zoneID,
							 struct	patch_object	*patch,
							 struct	date	current_date,
							 FILE *outfile)
{
    int check;
    check = fprintf(outfile,"%d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
    
    current_date.year,
    current_date.month,
    patch[0].ID,//3
    patch[0].acc_month.subQnet*1000.0,
    patch[0].acc_month.surfQnet*1000.0,//5
    patch[0].acc_month.subQvnet*1000.0,
    patch[0].acc_month.precip*1000.0,
    patch[0].acc_month.PET*1000.0,
    patch[0].acc_month.ET*1000.0,
    patch[0].acc_month.sat_deficit_z*1000.0 / patch[0].acc_month.days,//10
    patch[0].acc_month.peakLAI,
    patch[0].acc_month.meanLAI/ patch[0].acc_month.days,
    patch[0].acc_month.psn*1000.0,
    patch[0].acc_month.denitrif*1000.0,
    patch[0].acc_month.nitrif*1000.0, //15
    // RZ: (May 3, 2024) added NO3 variables
    patch[0].acc_month.surf_NO3*1000.0/ patch[0].acc_month.days,   //, surfNO3 
    patch[0].acc_month.surf_NO3_out*1000.0, // surfNO3_out
    patch[0].acc_month.surf_NO3_in*1000.0,  // surfNO3_in

    patch[0].acc_month.soil_NO3*1000/ patch[0].acc_month.days, // soilNO3  g/m2

    patch[0].acc_month.sat_NO3*1000/ patch[0].acc_month.days, //20, satNO3
    patch[0].acc_month.sat_NO3_Qin*1000, 


    patch[0].acc_month.mineralization*1000.0,
    patch[0].acc_month.uptake*1000.0,
    patch[0].acc_month.subNO3net*1000.0,
    patch[0].acc_month.subNO3vnet*1000.0, //25
    patch[0].acc_month.subDOCnet*1000.0,
    patch[0].acc_month.no3drain2gw*1000.0,
    patch[0].acc_month.satChance/ patch[0].acc_month.days,
    patch[0].acc_month.plantlimitN/ patch[0].acc_month.days,
    patch[0].acc_month.plantlimitQ/ patch[0].acc_month.days, //30
    patch[0].acc_month.top30cmSat/ patch[0].acc_month.days, //30
    patch[0].acc_month.top60cmSat/ patch[0].acc_month.days, //30
    patch[0].acc_month.top100cmSat/ patch[0].acc_month.days, //30
    patch[0].acc_month.rtzS/ patch[0].acc_month.days,
    patch[0].acc_month.activeS/ patch[0].acc_month.days
    );
    
  
	return;
} /*end output_monthly_patch*/

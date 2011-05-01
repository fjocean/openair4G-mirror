#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "SIMULATION/TOOLS/defs.h"
#include "SIMULATION/RF/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/extern.h"

#ifdef OPENAIR2
#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/extern.h"
#include "UTIL/LOG/log_if.h"
#include "RRC/LITE/extern.h"
#include "PHY_INTERFACE/extern.h"
#include "UTIL/OCG/OCG.h"
#include "UTIL/OPT/opt.h" // to test OPT
#endif

#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"

#ifdef IFFT_FPGA
#include "PHY/LTE_REFSIG/mod_table.h"
#endif

#include "SCHED/defs.h"
#include "SCHED/extern.h"

#ifdef XFORMS
#include "forms.h"
#include "phy_procedures_sim_form.h"
#endif


#define RF

void init_lte_vars(LTE_DL_FRAME_PARMS **frame_parms, u8 extended_prefix_flag, u8 cooperation_flag,u8 transmission_mode,u8 abstraction_flag) {

  u8 eNB_id,UE_id;
  int i,j;

  PHY_vars_eNB_g = malloc(NB_eNB_INST*sizeof(PHY_VARS_eNB*));
  for (eNB_id=0; eNB_id<NB_eNB_INST;eNB_id++){ 
    PHY_vars_eNB_g[eNB_id] = malloc(sizeof(PHY_VARS_eNB));
    PHY_vars_eNB_g[eNB_id]->Mod_id=eNB_id;
    PHY_vars_eNB_g[eNB_id]->cooperation_flag=cooperation_flag;

  }
  //  PHY_VARS_UE *PHY_vars_UE; 
  PHY_vars_UE_g = malloc(NB_UE_INST*sizeof(PHY_VARS_UE*));
  for (UE_id=0; UE_id<NB_UE_INST;UE_id++){ // begin navid
    PHY_vars_UE_g[UE_id] = malloc(sizeof(PHY_VARS_UE));
    PHY_vars_UE_g[UE_id]->Mod_id=UE_id; 
  }// end navid

  //PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));
 
  *frame_parms = malloc(sizeof(LTE_DL_FRAME_PARMS));
  (*frame_parms)->frame_type         = 1;
  (*frame_parms)->tdd_config         = 3;
  (*frame_parms)->N_RB_DL            = 25;
  (*frame_parms)->N_RB_UL            = 25;
  (*frame_parms)->phich_config_common.phich_resource = oneSixth;
  (*frame_parms)->Ncp                = extended_prefix_flag;
  (*frame_parms)->Nid_cell           = 0;
  (*frame_parms)->nushift            = 0;
  (*frame_parms)->nb_antennas_tx     = (transmission_mode == 1) ? 1 : 2;
  (*frame_parms)->nb_antennas_rx     = 2;
  (*frame_parms)->mode1_flag = (transmission_mode == 1) ? 1 : 0;
  (*frame_parms)->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift = 0;//n_DMRS1 set to 0

  init_frame_parms(*frame_parms,1);
  //copy_lte_parms_to_phy_framing(frame_parms, &(PHY_config->PHY_framing));
  phy_init_top(*frame_parms);

  (*frame_parms)->twiddle_fft      = twiddle_fft;
  (*frame_parms)->twiddle_ifft     = twiddle_ifft;
  (*frame_parms)->rev              = rev;
  // navid 
  (*frame_parms)->tdd_config = 3;

  phy_init_lte_top((*frame_parms));

  // init all eNB vars

  for (eNB_id=0;eNB_id<NB_eNB_INST;eNB_id++) {
    memcpy(&(PHY_vars_eNB_g[eNB_id]->lte_frame_parms), (*frame_parms), sizeof(LTE_DL_FRAME_PARMS));
    PHY_vars_eNB_g[eNB_id]->lte_frame_parms.Nid_cell = eNB_id;
    PHY_vars_eNB_g[eNB_id]->lte_frame_parms.nushift = eNB_id%6;
    phy_init_lte_eNB(&PHY_vars_eNB_g[eNB_id]->lte_frame_parms,
		     &PHY_vars_eNB_g[eNB_id]->lte_eNB_common_vars,
		     PHY_vars_eNB_g[eNB_id]->lte_eNB_ulsch_vars,
		     0,
		     PHY_vars_eNB_g[eNB_id],
		     cooperation_flag,
		     abstraction_flag);
    
    /*
      PHY_vars_eNB_g[eNB_id]->dlsch_eNB[0] = (LTE_eNB_DLSCH_t**) malloc16(NUMBER_OF_UE_MAX*sizeof(LTE_eNB_DLSCH_t*));
      PHY_vars_eNB_g[eNB_id]->dlsch_eNB[1] = (LTE_eNB_DLSCH_t**) malloc16(NUMBER_OF_UE_MAX*sizeof(LTE_eNB_DLSCH_t*));
      PHY_vars_eNB_g[eNB_id]->ulsch_eNB = (LTE_eNB_ULSCH_t**) malloc16((1+NUMBER_OF_UE_MAX)*sizeof(LTE_eNB_ULSCH_t*));
    */

    for (i=0;i<NUMBER_OF_UE_MAX;i++) {
      for (j=0;j<2;j++) {
	PHY_vars_eNB_g[eNB_id]->dlsch_eNB[i][j] = new_eNB_dlsch(1,8,abstraction_flag);
	if (!PHY_vars_eNB_g[eNB_id]->dlsch_eNB[i][j]) {
	  msg("Can't get eNB dlsch structures\n");
	  exit(-1);
	}
	else {
	  msg("dlsch_eNB[%d][%d] => %p\n",i,j,PHY_vars_eNB_g[eNB_id]->dlsch_eNB[i][j]);
	  PHY_vars_eNB_g[eNB_id]->dlsch_eNB[i][j]->rnti=0;
	}
      }
      PHY_vars_eNB_g[eNB_id]->ulsch_eNB[1+i] = new_eNB_ulsch(3,abstraction_flag);
      if (!PHY_vars_eNB_g[eNB_id]->ulsch_eNB[1+i]) {
	msg("Can't get eNB ulsch structures\n");
	exit(-1);
      }

    }

    // ULSCH for RA
    PHY_vars_eNB_g[eNB_id]->ulsch_eNB[0] = new_eNB_ulsch(3,abstraction_flag);
    if (!PHY_vars_eNB_g[eNB_id]->ulsch_eNB[0]) {
      msg("Can't get eNB ulsch structures\n");
      exit(-1);
    }

    PHY_vars_eNB_g[eNB_id]->dlsch_eNB_SI  = new_eNB_dlsch(1,1,abstraction_flag);
    printf("eNB %d : SI %p\n",eNB_id,PHY_vars_eNB_g[eNB_id]->dlsch_eNB_SI);
    PHY_vars_eNB_g[eNB_id]->dlsch_eNB_ra  = new_eNB_dlsch(1,1,abstraction_flag);
    printf("eNB %d : RA %p\n",eNB_id,PHY_vars_eNB_g[eNB_id]->dlsch_eNB_ra);

    PHY_vars_eNB_g[eNB_id]->rx_total_gain_eNB_dB=150;
  }

  // init all UE vars

  for (UE_id=0; UE_id<NB_UE_INST;UE_id++){ 
    memcpy(&(PHY_vars_UE_g[UE_id]->lte_frame_parms), *frame_parms, sizeof(LTE_DL_FRAME_PARMS));
    // Do this until SSS detection is finished
    PHY_vars_UE_g[UE_id]->lte_frame_parms.Nid_cell = UE_id%3;
    PHY_vars_UE_g[UE_id]->lte_frame_parms.nushift = UE_id%3;

    phy_init_lte_ue(&PHY_vars_UE_g[UE_id]->lte_frame_parms,
		    &PHY_vars_UE_g[UE_id]->lte_ue_common_vars,
		    PHY_vars_UE_g[UE_id]->lte_ue_dlsch_vars,
		    PHY_vars_UE_g[UE_id]->lte_ue_dlsch_vars_SI,
		    PHY_vars_UE_g[UE_id]->lte_ue_dlsch_vars_ra,
		    PHY_vars_UE_g[UE_id]->lte_ue_pbch_vars,
		    PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars,
		    PHY_vars_UE_g[UE_id],
		    abstraction_flag);

    /*
      PHY_vars_UE_g[UE_id]->dlsch_ue[0] = (LTE_UE_DLSCH_t**) malloc16(NUMBER_OF_eNB_MAX*sizeof(LTE_UE_DLSCH_t*));
      PHY_vars_UE_g[UE_id]->dlsch_ue[1] = (LTE_UE_DLSCH_t**) malloc16(NUMBER_OF_eNB_MAX*sizeof(LTE_UE_DLSCH_t*));
    
      PHY_vars_UE_g[UE_id]->ulsch_ue = (LTE_UE_ULSCH_t**) malloc16(NUMBER_OF_eNB_MAX*sizeof(LTE_UE_ULSCH_t*));
    
      PHY_vars_UE_g[UE_id]->dlsch_ue_SI = (LTE_UE_DLSCH_t**) malloc16(NUMBER_OF_eNB_MAX*sizeof(LTE_UE_DLSCH_t*));
      PHY_vars_UE_g[UE_id]->dlsch_ue_ra = (LTE_UE_DLSCH_t**) malloc16(NUMBER_OF_eNB_MAX*sizeof(LTE_UE_DLSCH_t*));
    */

    for (i=0;i<NUMBER_OF_eNB_MAX;i++) {
      for (j=0;j<2;j++) {
	PHY_vars_UE_g[UE_id]->dlsch_ue[i][j]  = new_ue_dlsch(1,8,abstraction_flag);
	if (!PHY_vars_UE_g[UE_id]->dlsch_ue[i][j]) {
	  msg("Can't get ue dlsch structures\n");
	  exit(-1);
	}
	else
	  msg("dlsch_ue[%d][%d] => %p\n",UE_id,i,PHY_vars_UE_g[UE_id]->dlsch_ue[i][j]);//navid
      }
      
      
      PHY_vars_UE_g[UE_id]->ulsch_ue[i]  = new_ue_ulsch(3,abstraction_flag);
      if (!PHY_vars_UE_g[UE_id]->ulsch_ue[i]) {
	msg("Can't get ue ulsch structures\n");
	exit(-1);
      }
      
      PHY_vars_UE_g[UE_id]->dlsch_ue_SI[i]  = new_ue_dlsch(1,1,abstraction_flag);
      PHY_vars_UE_g[UE_id]->dlsch_ue_ra[i]  = new_ue_dlsch(1,1,abstraction_flag);
    }

  }
}

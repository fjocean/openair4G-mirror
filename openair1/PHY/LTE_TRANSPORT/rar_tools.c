/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file PHY/LTE_TRANSPORT/rar_tools.c
* \brief Routine for filling the PUSCH/ULSCH data structures based on a random-access response (RAR) SDU from MAC.  Note this is both for UE and eNB. V8.6 2009-03
* \author R. Knopp
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr
* \note
* \warning
*/
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "SCHED/extern.h"
#ifdef OPENAIR2
#include "LAYER2/MAC/defs.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SCHED/defs.h"
#endif

extern uint16_t RIV2nb_rb_LUT6[32];
extern uint16_t RIV2first_rb_LUT6[32];
extern uint16_t RIV2nb_rb_LUT25[512];
extern uint16_t RIV2first_rb_LUT25[512];
extern uint16_t RIV2nb_rb_LUT50[1600];
extern uint16_t RIV2first_rb_LUT50[1600];
extern uint16_t RIV2nb_rb_LUT100[6000];
extern uint16_t RIV2first_rb_LUT100[600];

extern uint16_t RIV_max6,RIV_max25,RIV_max50,RIV_max100;

//#define DEBUG_RAR

#ifdef OPENAIR2
int generate_eNB_ulsch_params_from_rar(unsigned char *rar_pdu,
				       u32 frame,
				       unsigned char subframe,
				       LTE_eNB_ULSCH_t *ulsch,
				       LTE_DL_FRAME_PARMS *frame_parms) {

  

  //  RA_HEADER_RAPID *rarh = (RA_HEADER_RAPID *)rar_pdu;
  //  RAR_PDU *rar = (RAR_PDU *)(rar_pdu+1);
  uint8_t *rar = (uint8_t *)(rar_pdu+1);
  u8 harq_pid = get_Msg3_harq_pid(frame_parms,frame,subframe);
  uint16_t rballoc;
  uint8_t cqireq;
  uint16_t *RIV2nb_rb_LUT, *RIV2first_rb_LUT;
  uint16_t RIV_max;

  LOG_D(PHY,"[eNB][RAPROC] generate_eNB_ulsch_params_from_rar: subframe %d (harq_pid %d)\n",subframe,harq_pid);
  switch (frame_parms->N_RB_DL) {
    case 6:
      RIV2nb_rb_LUT     = &RIV2nb_rb_LUT6[0];
      RIV2first_rb_LUT  = &RIV2first_rb_LUT6[0];
      RIV_max           = RIV_max6;
      break;
    case 25:
      RIV2nb_rb_LUT     = &RIV2nb_rb_LUT25[0];
      RIV2first_rb_LUT  = &RIV2first_rb_LUT25[0];
      RIV_max           = RIV_max25;
      break;
    case 50:
      RIV2nb_rb_LUT     = &RIV2nb_rb_LUT50[0];
      RIV2first_rb_LUT  = &RIV2first_rb_LUT50[0];
      RIV_max           = RIV_max50;
      break;
    case 100:
      RIV2nb_rb_LUT     = &RIV2nb_rb_LUT100[0];
      RIV2first_rb_LUT  = &RIV2first_rb_LUT100[0];
      RIV_max           = RIV_max100;
      break;
  }

  ulsch->harq_processes[harq_pid]->TPC                = (rar[3]>>2)&7;//rar->TPC;
  rballoc = (((uint16_t)(rar[1]&7))<<7)|(rar[2]>>1);
  if (rballoc>RIV_max) {
    LOG_E(PHY,"[eNB]dci_tools.c: ERROR: rb_alloc (%x)> RIV_max\n",rballoc);
    return(-1);
  }
  ulsch->harq_processes[harq_pid]->rar_alloc          = 1;
  ulsch->harq_processes[harq_pid]->first_rb           = RIV2first_rb_LUT[rballoc];
  ulsch->harq_processes[harq_pid]->nb_rb              = RIV2nb_rb_LUT[rballoc];
  ulsch->harq_processes[harq_pid]->Ndi                = 1;

  cqireq = rar[3]&1;
  if (cqireq==1){
    ulsch->Or2                                   = sizeof_wideband_cqi_rank2_2A_5MHz;
    ulsch->Or1                                   = sizeof_wideband_cqi_rank1_2A_5MHz;
    ulsch->O_RI                                  = 1;
  }
  else{
    ulsch->O_RI                                  = 0;//1;
    ulsch->Or2                                   = 0;
    ulsch->Or1                                   = 0;
 
  }
  ulsch->harq_processes[harq_pid]->O_ACK                                 = 0;//2;
  ulsch->beta_offset_cqi_times8                = 18;
  ulsch->beta_offset_ri_times8                 = 10;
  ulsch->beta_offset_harqack_times8            = 16;

  
  ulsch->Nsymb_pusch                           = 12-(frame_parms->Ncp<<1);
  ulsch->rnti = (((uint16_t)rar[4])<<8)+rar[5];  
  if (ulsch->harq_processes[harq_pid]->Ndi == 1) {
    ulsch->harq_processes[harq_pid]->status = ACTIVE;
    ulsch->harq_processes[harq_pid]->rvidx = 0;
    ulsch->harq_processes[harq_pid]->mcs         = ((rar[2]&1)<<3)|(rar[3]>>5);
    //ulsch->harq_processes[harq_pid]->TBS         = dlsch_tbs25[ulsch->harq_processes[harq_pid]->mcs][ulsch->harq_processes[harq_pid]->nb_rb-1];
    ulsch->harq_processes[harq_pid]->TBS         = TBStable[get_I_TBS(ulsch->harq_processes[harq_pid]->mcs)][ulsch->harq_processes[harq_pid]->nb_rb-1];
    ulsch->harq_processes[harq_pid]->Msc_initial   = 12*ulsch->harq_processes[harq_pid]->nb_rb;
    ulsch->harq_processes[harq_pid]->Nsymb_initial = 9;
    ulsch->harq_processes[harq_pid]->round = 0;
  }
  else {
    ulsch->harq_processes[harq_pid]->rvidx = 0;
    ulsch->harq_processes[harq_pid]->round++;
  }
  //#ifdef DEBUG_RAR
  msg("ulsch ra (eNB): harq_pid %d\n",harq_pid);
  msg("ulsch ra (eNB): NBRB     %d\n",ulsch->harq_processes[harq_pid]->nb_rb);
  msg("ulsch ra (eNB): rballoc  %x\n",ulsch->harq_processes[harq_pid]->first_rb);
  msg("ulsch ra (eNB): harq_pid %d\n",harq_pid);
  msg("ulsch ra (eNB): Ndi      %d\n",ulsch->harq_processes[harq_pid]->Ndi);  
  msg("ulsch ra (eNB): TBS      %d\n",ulsch->harq_processes[harq_pid]->TBS);
  msg("ulsch ra (eNB): mcs      %d\n",ulsch->harq_processes[harq_pid]->mcs);
  msg("ulsch ra (eNB): Or1      %d\n",ulsch->Or1);
  msg("ulsch ra (eNB): ORI      %d\n",ulsch->O_RI);
  //#endif
  return(0);
}

s8 delta_PUSCH_msg2[8] = {-6,-4,-2,0,2,4,6,8};

int generate_ue_ulsch_params_from_rar(PHY_VARS_UE *phy_vars_ue,
				      unsigned char eNB_id ){
  
  //  RA_HEADER_RAPID *rarh = (RA_HEADER_RAPID *)rar_pdu;
  u8 transmission_mode = phy_vars_ue->transmission_mode[eNB_id];
  u32 frame              = phy_vars_ue->ulsch_ue_Msg3_frame[eNB_id];
  unsigned char *rar_pdu = phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->b;
  unsigned char subframe = phy_vars_ue->ulsch_ue_Msg3_subframe[eNB_id];
  LTE_UE_ULSCH_t *ulsch  = phy_vars_ue->ulsch_ue[eNB_id];
  PHY_MEASUREMENTS *meas = &phy_vars_ue->PHY_measurements;
  LTE_DL_FRAME_PARMS *frame_parms =  &phy_vars_ue->lte_frame_parms;
  //  int current_dlsch_cqi = phy_vars_ue->current_dlsch_cqi[eNB_id];  

  uint8_t *rar = (uint8_t *)(rar_pdu+1);
  u8 harq_pid = subframe2harq_pid(frame_parms,phy_vars_ue->frame,subframe);
  uint16_t rballoc;
  uint8_t cqireq;
  double sinr_eff;
  uint16_t *RIV2nb_rb_LUT, *RIV2first_rb_LUT;
  uint16_t RIV_max;

  LOG_D(PHY,"[eNB][RAPROC] generate_ue_ulsch_params_from_rar: subframe %d (harq_pid %d)\n",subframe,harq_pid);

  switch (frame_parms->N_RB_DL) {
    case 6:
      RIV2nb_rb_LUT     = &RIV2nb_rb_LUT6[0];
      RIV2first_rb_LUT  = &RIV2first_rb_LUT6[0];
      RIV_max           = RIV_max6;
      break;
    case 25:
      RIV2nb_rb_LUT     = &RIV2nb_rb_LUT25[0];
      RIV2first_rb_LUT  = &RIV2first_rb_LUT25[0];
      RIV_max           = RIV_max25;
      break;
    case 50:
      RIV2nb_rb_LUT     = &RIV2nb_rb_LUT50[0];
      RIV2first_rb_LUT  = &RIV2first_rb_LUT50[0];
      RIV_max           = RIV_max50;
      break;
    case 100:
      RIV2nb_rb_LUT     = &RIV2nb_rb_LUT100[0];
      RIV2first_rb_LUT  = &RIV2first_rb_LUT100[0];
      RIV_max           = RIV_max100;
      break;
  }


  
  ulsch->harq_processes[harq_pid]->TPC                                   = (rar[3]>>3)&7;//rar->TPC;
  
  rballoc = (((uint16_t)(rar[1]&7))<<7)|(rar[2]>>1);
  cqireq=rar[3]&1;

  if (rballoc>RIV_max) {
    msg("rar_tools.c: ERROR: rb_alloc (%x) > RIV_max\n",rballoc);
    return(-1);
  }

  ulsch->harq_processes[harq_pid]->first_rb                              = RIV2first_rb_LUT[rballoc];
  ulsch->harq_processes[harq_pid]->nb_rb                                 = RIV2nb_rb_LUT[rballoc];
  if (ulsch->harq_processes[harq_pid]->nb_rb ==0)
    return(-1);

  ulsch->power_offset = ue_power_offsets[ulsch->harq_processes[harq_pid]->nb_rb];

  if (ulsch->harq_processes[harq_pid]->nb_rb > 4) {
    msg("rar_tools.c: unlikely rb count for RAR grant : nb_rb > 3\n");
    return(-1);
  }

  ulsch->harq_processes[harq_pid]->Ndi                                   = 1;
  if (ulsch->harq_processes[harq_pid]->Ndi == 1)
    ulsch->harq_processes[harq_pid]->status = ACTIVE;

  if (cqireq==1) {
    ulsch->O_RI                                  = 1;
    if (meas->rank[eNB_id] == 1) {
      ulsch->uci_format                          = wideband_cqi_rank2_2A;
      ulsch->O                                   = sizeof_wideband_cqi_rank2_2A_5MHz;
      ulsch->o_RI[0]                             = 1;
    }
    else {
      ulsch->uci_format                          = wideband_cqi_rank1_2A;
      ulsch->O                                   = sizeof_wideband_cqi_rank1_2A_5MHz;
      ulsch->o_RI[0]                             = 0;
    }
  
    ulsch->uci_format = HLC_subband_cqi_nopmi;
    //int flag_LA = 0;
    //   if(flag_LA==1)
    //  {
    sinr_eff = sinr_eff_cqi_calc(phy_vars_ue, eNB_id);
    
    fill_CQI(ulsch->o,ulsch->uci_format,meas,eNB_id,transmission_mode,sinr_eff);
    //  }
    // else
    // fill_CQI(ulsch->o,ulsch->uci_format,meas,eNB_id,transmission_mode);
    if (((phy_vars_ue->frame % 100) == 0) || (phy_vars_ue->frame < 10)) 
      print_CQI(ulsch->o,ulsch->uci_format,eNB_id);
  }
  else {
    ulsch->O_RI                                = 0;
    ulsch->O                                   = 0;
  }
  ulsch->harq_processes[harq_pid]->O_ACK                                  = 0;//2;
  
  ulsch->beta_offset_cqi_times8                  = 18;
  ulsch->beta_offset_ri_times8                   = 10;
  ulsch->beta_offset_harqack_times8              = 16;
  
  ulsch->Nsymb_pusch                             = 12-(frame_parms->Ncp<<1);
  ulsch->rnti = (((uint16_t)rar[4])<<8)+rar[5];  //rar->t_crnti;
  
  if (ulsch->harq_processes[harq_pid]->Ndi == 1) {
    ulsch->harq_processes[harq_pid]->status = ACTIVE;
    ulsch->harq_processes[harq_pid]->rvidx = 0;
    ulsch->harq_processes[harq_pid]->mcs         = ((rar[2]&1)<<3)|(rar[3]>>5);
    ulsch->harq_processes[harq_pid]->TPC         = (rar[3]>>2)&7;
    //ulsch->harq_processes[harq_pid]->TBS         = dlsch_tbs25[ulsch->harq_processes[harq_pid]->mcs][ulsch->harq_processes[harq_pid]->nb_rb-1];
    ulsch->harq_processes[harq_pid]->TBS         = TBStable[get_I_TBS(ulsch->harq_processes[harq_pid]->mcs)][ulsch->harq_processes[harq_pid]->nb_rb-1];
    ulsch->harq_processes[harq_pid]->Msc_initial   = 12*ulsch->harq_processes[harq_pid]->nb_rb;
    ulsch->harq_processes[harq_pid]->Nsymb_initial = 9;
    ulsch->harq_processes[harq_pid]->round = 0;
    }
    else {
      ulsch->harq_processes[harq_pid]->rvidx = 0;
      ulsch->harq_processes[harq_pid]->round++;
    }

    // initialize power control based on PRACH power
	ulsch->f_pusch = delta_PUSCH_msg2[ulsch->harq_processes[harq_pid]->TPC] +
	mac_xface->get_deltaP_rampup(phy_vars_ue->Mod_id);
	LOG_D(PHY,"[UE %d][PUSCH PC] Initializing f_pusch to %d dB, TPC %d (delta_PUSCH_msg2 %d dB), deltaP_rampup %d dB\n",
	    phy_vars_ue->Mod_id,ulsch->f_pusch,ulsch->harq_processes[harq_pid]->TPC,delta_PUSCH_msg2[ulsch->harq_processes[harq_pid]->TPC],
	    mac_xface->get_deltaP_rampup(phy_vars_ue->Mod_id));
    

	//#ifdef DEBUG_RAR
    msg("ulsch ra (UE): NBRB     %d\n",ulsch->harq_processes[harq_pid]->nb_rb);
    msg("ulsch ra (UE): first_rb %x\n",ulsch->harq_processes[harq_pid]->first_rb);
    msg("ulsch ra (UE): nb_rb    %d\n",ulsch->harq_processes[harq_pid]->nb_rb);
    msg("ulsch ra (UE): Ndi      %d\n",ulsch->harq_processes[harq_pid]->Ndi);  
    msg("ulsch ra (UE): TBS      %d\n",ulsch->harq_processes[harq_pid]->TBS);
    msg("ulsch ra (UE): mcs      %d\n",ulsch->harq_processes[harq_pid]->mcs);
    msg("ulsch ra (UE): TPC      %d\n",ulsch->harq_processes[harq_pid]->TPC);
    msg("ulsch ra (UE): O        %d\n",ulsch->O);
    msg("ulsch ra (UE): ORI      %d\n",ulsch->O_RI);
    //#endif
    return(0);
}
#endif

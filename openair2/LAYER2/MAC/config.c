#include "COMMON/platform_types.h"
#include "COMMON/platform_constants.h"
#include "RadioResourceConfigCommonSIB.h"
#include "RadioResourceConfigDedicated.h"
#include "MeasGapConfig.h"
#include "MeasObjectToAddModList.h"
#include "TDD-Config.h"
#include "defs.h"
#include "extern.h"
#include "UTIL/LOG/log.h"

int rrc_mac_config_req(u8 Mod_id,u8 eNB_flag,u8 UE_id,u8 eNB_index, 
		       RadioResourceConfigCommonSIB_t *radioResourceConfigCommon,
		       PhysicalConfigDedicated_t *physicalConfigDedicated,
		       MeasObjectToAddMod_t **measObj,
		       MAC_MainConfig_t *mac_MainConfig,
		       long logicalChannelIdentity,
		       LogicalChannelConfig_t *logicalChannelConfig,
		       MeasGapConfig_t *measGapConfig,
		       TDD_Config_t *tdd_Config,
		       u8 *SIwindowsize,
		       u16 *SIperiod) {

  int i;

  if (eNB_flag==0) {
    LOG_I(MAC,"[CONFIG][UE %d] Configuring MAC/PHY from eNB %d\n",Mod_id,eNB_index);
    if (tdd_Config != NULL)
      UE_mac_inst[Mod_id].scheduling_info[eNB_index].tdd_Config = tdd_Config;
  }else {
    if (physicalConfigDedicated == NULL){
      LOG_I(MAC,"[CONFIG][eNB %d] Configuring MAC/PHY for UE %d \n",Mod_id, UE_id);
    } else{
      LOG_I(MAC,"[CONFIG][eNB %d] Configuring MAC/PHY for UE %d (%x)\n",Mod_id,UE_id,find_UE_RNTI(Mod_id,UE_id));
    }
  }
  
  if ((tdd_Config!=NULL)||
      (SIwindowsize!=NULL)||
      (SIperiod!=NULL)){

    if (eNB_flag==1)
      mac_xface->phy_config_sib1_eNB(Mod_id,tdd_Config,*SIwindowsize,*SIperiod);
    else
      mac_xface->phy_config_sib1_ue(Mod_id,eNB_index,tdd_Config,*SIwindowsize,*SIperiod);
  } 

  if (radioResourceConfigCommon) {
    if (eNB_flag==1) {
      LOG_I(MAC,"[CONFIG]SIB2/3 Contents (partial)\n");
      LOG_I(MAC,"[CONFIG]pusch_config_common.n_SB = %ld\n",radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.n_SB);
      LOG_I(MAC,"[CONFIG]pusch_config_common.hoppingMode = %ld\n",radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode);
      LOG_I(MAC,"[CONFIG]pusch_config_common.pusch_HoppingOffset = %ld\n",  radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset);
      LOG_I(MAC,"[CONFIG]pusch_config_common.enable64QAM = %d\n",radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM);
      LOG_I(MAC,"[CONFIG]pusch_config_common.groupHoppingEnabled = %d\n",radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled);
      LOG_I(MAC,"[CONFIG]pusch_config_common.groupAssignmentPUSCH = %ld\n",radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH);
      LOG_I(MAC,"[CONFIG]pusch_config_common.sequenceHoppingEnabled = %d\n",radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled);
      LOG_I(MAC,"[CONFIG]pusch_config_common.cyclicShift  = %ld\n",radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift); 
      mac_xface->phy_config_sib2_eNB(Mod_id,radioResourceConfigCommon);
    }
    else {
      UE_mac_inst[Mod_id].scheduling_info[eNB_index].radioResourceConfigCommon = radioResourceConfigCommon;
      mac_xface->phy_config_sib2_ue(Mod_id,eNB_index,radioResourceConfigCommon);
    }
  }
  
  if (logicalChannelConfig!= NULL) {
    if (eNB_flag==0){
      LOG_I(MAC,"[CONFIG][UE %d] Applying RRC logicalChannelConfig with id %d from eNB %d\n",Mod_id,logicalChannelIdentity, eNB_index);
      UE_mac_inst[Mod_id].scheduling_info[eNB_index].logicalChannelConfig[logicalChannelIdentity]=logicalChannelConfig;
      UE_mac_inst[Mod_id].scheduling_info[eNB_index].Bj[logicalChannelIdentity]=0; // initilize the bucket for this lcid
      if (logicalChannelConfig->ul_SpecificParameters)
	UE_mac_inst[Mod_id].scheduling_info[eNB_index].bucket_size[logicalChannelIdentity]=logicalChannelConfig->ul_SpecificParameters->prioritisedBitRate *
	  logicalChannelConfig->ul_SpecificParameters->bucketSizeDuration; // set the max bucket size
      else {
	LOG_E(MAC,"[CONFIG][UE %d] LCID %d NULL ul_SpecificParameters\n",Mod_id,logicalChannelIdentity);
	mac_xface->macphy_exit("");
      }
    } 
  }

  if (mac_MainConfig != NULL){
    if (eNB_flag==0){
      LOG_I(MAC,"[CONFIG][UE%d] Applying RRC macMainConfig from eNB%d\n",Mod_id,eNB_index);
      UE_mac_inst[Mod_id].scheduling_info[eNB_index].macConfig=mac_MainConfig;
      if (measGapConfig !=NULL) {
	UE_mac_inst[Mod_id].scheduling_info[eNB_index].measGapConfig=measGapConfig;
      } else {
	UE_mac_inst[Mod_id].scheduling_info[eNB_index].measGapConfig=NULL;
      }
      if (mac_MainConfig->ul_SCH_Config) {
	
	if (mac_MainConfig->ul_SCH_Config->periodicBSR_Timer)
	  UE_mac_inst[Mod_id].scheduling_info[eNB_index].periodicBSR_Timer = (u16) *mac_MainConfig->ul_SCH_Config->periodicBSR_Timer;
	else
	  UE_mac_inst[Mod_id].scheduling_info[eNB_index].periodicBSR_Timer = (u16) MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_infinity;

	if (mac_MainConfig->ul_SCH_Config->maxHARQ_Tx)
	  UE_mac_inst[Mod_id].scheduling_info[eNB_index].maxHARQ_Tx     = (u16) *mac_MainConfig->ul_SCH_Config->maxHARQ_Tx;
	else
	  UE_mac_inst[Mod_id].scheduling_info[eNB_index].maxHARQ_Tx     = (u16) MAC_MainConfig__ul_SCH_Config__maxHARQ_Tx_n5;
      	
	if (mac_MainConfig->ul_SCH_Config->retxBSR_Timer)
	  UE_mac_inst[Mod_id].scheduling_info[eNB_index].retxBSR_Timer     = (u16) mac_MainConfig->ul_SCH_Config->retxBSR_Timer;
	else 
	  UE_mac_inst[Mod_id].scheduling_info[eNB_index].retxBSR_Timer     = (u16)MAC_MainConfig__ul_SCH_Config__retxBSR_Timer_sf2560;
      }
#ifdef Rel10   
      if (mac_MainConfig->sr_ProhibitTimer_r9) 
	UE_mac_inst[Mod_id].scheduling_info[eNB_index].sr_ProhibitTimer  = (u16) *mac_MainConfig->sr_ProhibitTimer_r9;
      else
	UE_mac_inst[Mod_id].scheduling_info[eNB_index].sr_ProhibitTimer  = (u16) 0;
#else 
      UE_mac_inst[Mod_id].scheduling_info[eNB_index].sr_ProhibitTimer  = (u16) 0;
#endif
      UE_mac_inst[Mod_id].scheduling_info[eNB_index].periodicBSR_SF  = get_sf_periodicBSRTimer(UE_mac_inst[Mod_id].scheduling_info[eNB_index].periodicBSR_Timer);
      UE_mac_inst[Mod_id].scheduling_info[eNB_index].retxBSR_SF     = get_sf_retxBSRTimer(UE_mac_inst[Mod_id].scheduling_info[eNB_index].retxBSR_Timer);
      
      UE_mac_inst[Mod_id].scheduling_info[eNB_index].drx_config     = mac_MainConfig->drx_Config;
      UE_mac_inst[Mod_id].scheduling_info[eNB_index].phr_config     = mac_MainConfig->phr_Config;
      if (mac_MainConfig->phr_Config){
	UE_mac_inst[Mod_id].PHR_state[eNB_index] = mac_MainConfig->phr_Config->present;
	UE_mac_inst[Mod_id].PHR_reconfigured[eNB_index] = 1;
	UE_mac_inst[Mod_id].scheduling_info[eNB_index].periodicPHR_Timer = mac_MainConfig->phr_Config->choice.setup.periodicPHR_Timer;
	UE_mac_inst[Mod_id].scheduling_info[eNB_index].prohibitPHR_Timer = mac_MainConfig->phr_Config->choice.setup.prohibitPHR_Timer;
	UE_mac_inst[Mod_id].scheduling_info[eNB_index].PathlossChange = mac_MainConfig->phr_Config->choice.setup.dl_PathlossChange;
      } else {
	UE_mac_inst[Mod_id].PHR_reconfigured[eNB_index] = 0;
	UE_mac_inst[Mod_id].PHR_state[eNB_index] = MAC_MainConfig__phr_Config_PR_setup;
	UE_mac_inst[Mod_id].scheduling_info[eNB_index].periodicPHR_Timer = MAC_MainConfig__phr_Config__setup__periodicPHR_Timer_sf20;
	UE_mac_inst[Mod_id].scheduling_info[eNB_index].prohibitPHR_Timer = MAC_MainConfig__phr_Config__setup__prohibitPHR_Timer_sf20;
	UE_mac_inst[Mod_id].scheduling_info[eNB_index].PathlossChange = MAC_MainConfig__phr_Config__setup__dl_PathlossChange_dB1;
      }	
      UE_mac_inst[Mod_id].scheduling_info[eNB_index].periodicPHR_SF =  get_sf_perioidicPHR_Timer(UE_mac_inst[Mod_id].scheduling_info[eNB_index].periodicPHR_Timer);
      UE_mac_inst[Mod_id].scheduling_info[eNB_index].prohibitPHR_SF =  get_sf_prohibitPHR_Timer(UE_mac_inst[Mod_id].scheduling_info[eNB_index].prohibitPHR_Timer);
      UE_mac_inst[Mod_id].scheduling_info[eNB_index].PathlossChange_db =  get_db_dl_PathlossChange(UE_mac_inst[Mod_id].scheduling_info[eNB_index].PathlossChange);
      LOG_D(MAC,"[UE %d] config PHR for eNB %d: periodic %d (SF) prohibit %d (SF)  pathlosschange %d (db) \n",
	    Mod_id,eNB_index, 
	    UE_mac_inst[Mod_id].scheduling_info[eNB_index].periodicPHR_SF,
	    UE_mac_inst[Mod_id].scheduling_info[eNB_index].prohibitPHR_SF,
	    UE_mac_inst[Mod_id].scheduling_info[eNB_index].PathlossChange_db);
    }
  }
  if (physicalConfigDedicated != NULL) {
    if (eNB_flag==1){
      mac_xface->phy_config_dedicated_eNB(Mod_id,find_UE_RNTI(Mod_id,UE_id),physicalConfigDedicated);
    }else{
      mac_xface->phy_config_dedicated_ue(Mod_id,eNB_index,physicalConfigDedicated);
      UE_mac_inst[Mod_id].scheduling_info[eNB_index].physicalConfigDedicated=physicalConfigDedicated; // for SR proc
    }
  }
  if (eNB_flag == 0) {
    if (measObj!= NULL) 
      if (measObj[0]!= NULL){
	UE_mac_inst[Mod_id].n_adj_cells = measObj[0]->measObject.choice.measObjectEUTRA.cellsToAddModList->list.count;
	LOG_I(MAC,"Number of adjacent cells %d\n",UE_mac_inst[Mod_id].n_adj_cells);
	for (i=0;i<UE_mac_inst[Mod_id].n_adj_cells;i++) {
	  UE_mac_inst[Mod_id].adj_cell_id[i] = measObj[0]->measObject.choice.measObjectEUTRA.cellsToAddModList->list.array[i]->physCellId;
	  LOG_I(MAC,"Cell %d : Nid_cell %d\n",i,UE_mac_inst[Mod_id].adj_cell_id[i]);
	}
	mac_xface->phy_config_meas_ue(Mod_id,eNB_index,UE_mac_inst[Mod_id].n_adj_cells,UE_mac_inst[Mod_id].adj_cell_id);
      }
  }
  return(0);
}

int rrc_mac_config_co_req(u8 Mod_id,
			  u8 eNB_flag, 
			  u8 index,
			  u16 co_RNTI,
			  u8 virtualLinkID) {
  
  int ret = 0;
  int nb_corntis=0;
  
  ret = mac_forwarding_add_entry(Mod_id, eNB_flag, index, virtualLinkID, co_RNTI);
  
  
  if (eNB_flag == 0 ) { // this is a UE
    nb_corntis = UE_mac_inst[Mod_id].corntis.count;
    UE_mac_inst[Mod_id].corntis.array[nb_corntis] = co_RNTI;
    mac_xface->phy_config_cornti(Mod_id, eNB_flag, index, co_RNTI,virtualLinkID); 
    UE_mac_inst[Mod_id].corntis.count++;
    LOG_D(MAC,"[UE %d] configuring CORNTI %x with index %d for eNB %d\n", Mod_id,co_RNTI,nb_corntis, index);
    if ((ret = mac_buffer_instantiate (Mod_id, index, co_RNTI, UE_mac_inst[Mod_id].mac_buffer_capacity)) == 1 )
      LOG_D(MAC,"[UE %d] mac buffer instantiated  CORNTI %x with index %d for eNB %d with capacity %d pdus\n", 
	    Mod_id,co_RNTI,nb_corntis, index, UE_mac_inst[Mod_id].mac_buffer_capacity);
    else 
      LOG_E(MAC,"[UE %d] failed to instantiate mac buffer CORNTI %x with index %d for eNB %d\n", Mod_id,co_RNTI,nb_corntis, index);
  } else { // this is an eNB
    nb_corntis = eNB_mac_inst[Mod_id].UE_template[index].corntis.count;
    //LOG_D(MAC,"[eNB %d] before configuring CORNTI %x with index %d for UE %d\n", Mod_id,co_RNTI,nb_corntis, index);
    eNB_mac_inst[Mod_id].UE_template[index].corntis.array[nb_corntis] = co_RNTI;
    mac_xface->phy_config_cornti(Mod_id, eNB_flag, index, co_RNTI, virtualLinkID);  
    nb_corntis = eNB_mac_inst[Mod_id].UE_template[index].corntis.count++;
    LOG_D(MAC,"[eNB %d] configuring CORNTI %x with index %d for UE %d\n", Mod_id,co_RNTI,nb_corntis, index);
  }
      
  return ret;
}


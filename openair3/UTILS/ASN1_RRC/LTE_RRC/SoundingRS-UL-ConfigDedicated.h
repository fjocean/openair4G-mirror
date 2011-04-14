/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "EUTRA-RRC-Definitions"
 * 	found in "../ASN1_files/EUTRA-RRC-Definitions.asn"
 * 	`asn1c -gen-PER`
 */

#ifndef	_SoundingRS_UL_ConfigDedicated_H_
#define	_SoundingRS_UL_ConfigDedicated_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NULL.h>
#include <ENUMERATED.h>
#include <NativeInteger.h>
#include <BOOLEAN.h>
#include <constr_SEQUENCE.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SoundingRS_UL_ConfigDedicated_PR {
	SoundingRS_UL_ConfigDedicated_PR_NOTHING,	/* No components present */
	SoundingRS_UL_ConfigDedicated_PR_release,
	SoundingRS_UL_ConfigDedicated_PR_setup
} SoundingRS_UL_ConfigDedicated_PR;
typedef enum SoundingRS_UL_ConfigDedicated__setup__srs_Bandwidth {
	SoundingRS_UL_ConfigDedicated__setup__srs_Bandwidth_bw0	= 0,
	SoundingRS_UL_ConfigDedicated__setup__srs_Bandwidth_bw1	= 1,
	SoundingRS_UL_ConfigDedicated__setup__srs_Bandwidth_bw2	= 2,
	SoundingRS_UL_ConfigDedicated__setup__srs_Bandwidth_bw3	= 3
} e_SoundingRS_UL_ConfigDedicated__setup__srs_Bandwidth;
typedef enum SoundingRS_UL_ConfigDedicated__setup__srs_HoppingBandwidth {
	SoundingRS_UL_ConfigDedicated__setup__srs_HoppingBandwidth_hbw0	= 0,
	SoundingRS_UL_ConfigDedicated__setup__srs_HoppingBandwidth_hbw1	= 1,
	SoundingRS_UL_ConfigDedicated__setup__srs_HoppingBandwidth_hbw2	= 2,
	SoundingRS_UL_ConfigDedicated__setup__srs_HoppingBandwidth_hbw3	= 3
} e_SoundingRS_UL_ConfigDedicated__setup__srs_HoppingBandwidth;
typedef enum SoundingRS_UL_ConfigDedicated__setup__cyclicShift {
	SoundingRS_UL_ConfigDedicated__setup__cyclicShift_cs0	= 0,
	SoundingRS_UL_ConfigDedicated__setup__cyclicShift_cs1	= 1,
	SoundingRS_UL_ConfigDedicated__setup__cyclicShift_cs2	= 2,
	SoundingRS_UL_ConfigDedicated__setup__cyclicShift_cs3	= 3,
	SoundingRS_UL_ConfigDedicated__setup__cyclicShift_cs4	= 4,
	SoundingRS_UL_ConfigDedicated__setup__cyclicShift_cs5	= 5,
	SoundingRS_UL_ConfigDedicated__setup__cyclicShift_cs6	= 6,
	SoundingRS_UL_ConfigDedicated__setup__cyclicShift_cs7	= 7
} e_SoundingRS_UL_ConfigDedicated__setup__cyclicShift;

/* SoundingRS-UL-ConfigDedicated */
typedef struct SoundingRS_UL_ConfigDedicated {
	SoundingRS_UL_ConfigDedicated_PR present;
	union SoundingRS_UL_ConfigDedicated_u {
		NULL_t	 release;
		struct SoundingRS_UL_ConfigDedicated__setup {
			ENUMERATED_t	 srs_Bandwidth;
			ENUMERATED_t	 srs_HoppingBandwidth;
			long	 freqDomainPosition;
			BOOLEAN_t	 duration;
			long	 srs_ConfigIndex;
			long	 transmissionComb;
			ENUMERATED_t	 cyclicShift;
			
			/* Context for parsing across buffer boundaries */
			asn_struct_ctx_t _asn_ctx;
		} setup;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SoundingRS_UL_ConfigDedicated_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_srs_Bandwidth_4;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_srs_HoppingBandwidth_9;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_cyclicShift_18;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_SoundingRS_UL_ConfigDedicated;

#ifdef __cplusplus
}
#endif

#endif	/* _SoundingRS_UL_ConfigDedicated_H_ */

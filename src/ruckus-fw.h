#ifndef RUCKUS_FW_H_INCLUDED
#define RUCKUS_FW_H_INCLUDED

#include <stdlib.h>
#include <stdint.h> 
#include <stdio.h>
#include <string.h>
#include "md5.h"

static const char HDR_MAGIC[] = "RCKS";

struct bin_hdr
{
	// This struct is stored in network byte order.
	
	uint8_t magic[4];	// 0x0
	uint32_t next_image;	 // 0x4 // offset to next image ( reserved )
	uint8_t invalid;	 // 0x8 // only autoboot from this image if invalid is 0
	uint8_t hdr_len;	 // 0x9 length of this header
	uint8_t compression[2]; // 0xa // compression scheme, default l7, [l7|zl|xx]

	uint32_t entry_point; // 0xc // Execution entry point
	uint32_t binl7_len;   // 0x10 // length of (compressed) binary image

	uint32_t timestamp;	// 0x14
	uint8_t signature[16]; // 0x18 // MD5 checksum

	uint16_t hdr_version; // 0x28 // header version
	uint16_t hdr_cksum;   // 0x2a // header checksum
	uint8_t version[16];  // 0x2c // FW version string

	// the following fields are for validating that we have the correct image
	// The rule is that a value of 0 means don't care
	//
	uint8_t product;	// 0x3c // product class, e.g. AP vs Adapter
	uint8_t architecture; // 0x3d // mips, le/be, etc.
	uint8_t chipset;	// 0x3e // e.g AR531X
	uint8_t board_type;   // 0x3f // V54 board type

	uint8_t customer[32];	 // 0x40 // customer ID, NULL ==> any customer
	uint8_t product_v2[12]; // 0x60 // FWv2 product string
	uint8_t version_v2[19]; // 0x6c // FWv2 version string
	uint8_t board_class;	 // 0x7f // FWv2 board class

	uint32_t load_address; // 0x80
	uint32_t image_type;	// 0x84 // 0=Unsigned Image (UI) / 1=Intermediate Signed Image (ISI) / 2=Fully Signed Image (FSI)
	uint32_t tail_offset;	// 0x88 // Offset to the signatures within the binl7. Only ISI stores signatures within the binl7, so this is 0 for UI & FSI images.
	uint8_t _pad_end[20];	// pad out to 160 bytes
};

void ntoh_hdr(struct bin_hdr *binhdr);
void hton_hdr(struct bin_hdr *binhdr);

uint16_t calc_hdr_cksum(struct bin_hdr *binhdr);
int calc_binl7_digest(FILE *fd, size_t binl7_bytes, md5_byte_t *digest);

#endif
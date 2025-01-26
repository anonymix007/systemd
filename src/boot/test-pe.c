/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <stddef.h>
#include <stdint.h>

#include "chid.h"
#include "devicetree.h"
#include "efifirmware.h"
#include "pe.h"
#include "smbios.h"
#include "tests.h"
#include "uki.h"

extern uint8_t _binary_src_boot_pe_efi_start[];
extern size_t _binary_src_boot_pe_efi_size;

static struct {
        const RawSmbiosInfo smbios_info;
        uint32_t device_type;
        const char *compatible;
        const char *fwid;
} tests[] = {
        {
                .smbios_info =  {
                        .manufacturer           = "First Vendor",
                        .product_name           = "Device 1",
                        .product_sku            = "KD01",
                        .family                 = "Laptop X",
                        .baseboard_product      = "FODM1",
                        .baseboard_manufacturer = "First ODM",
                },
                .device_type = DEVICE_TYPE_DEVICETREE,
                .compatible = "test,device-1",
        },
        {
                .smbios_info = {
                        .manufacturer           = "Second Vendor",
                        .product_name           = "Device 2",
                        .product_sku            = "KD02",
                        .family                 = "Laptop 2",
                        .baseboard_product      = "SODM2",
                        .baseboard_manufacturer = "Second ODM",
                },
                .device_type = DEVICE_TYPE_DEVICETREE,
                .compatible = "test,device-2",
        },
        {
                .smbios_info = {
                        .manufacturer           = "First Vendor",
                        .product_name           = "Device 3",
                        .product_sku            = "KD03",
                        .family                 = "Tablet Y",
                        .baseboard_product      = "FODM2",
                        .baseboard_manufacturer = "First ODM",
                },
                .device_type = DEVICE_TYPE_DEVICETREE,
                .compatible = "test,device-3",
        },
        {
                .smbios_info = {
                        .manufacturer           = "VMware, Inc.",
                        .product_name           = "VMware20,1",
                        .product_sku            = "0000000000000001",
                        .family                 = "VMware",
                        .baseboard_product      = "VBSA",
                        .baseboard_manufacturer = "VMware, Inc.",
                },
                .device_type = DEVICE_TYPE_UEFI_FW,
                .fwid = "fw",
        },
};

static RawSmbiosInfo current_info = {};

/* This is a dummy implementation for testing purposes */
void smbios_raw_info_get_cached(RawSmbiosInfo *ret_info) {
        assert(ret_info);
        *ret_info = current_info;
}

TEST(pe_memory_locate) {
        for (size_t i = 0; i < ELEMENTSOF(tests); i++) {
                current_info = tests[i].smbios_info;
                PeSectionVector sections[ELEMENTSOF(unified_sections)] = {};
                /* Locate and check */
                ASSERT_EQ(pe_memory_locate_sections(_binary_src_boot_pe_efi_start, unified_sections, sections), EFI_SUCCESS);
                ASSERT_TRUE(PE_SECTION_VECTOR_IS_SET(&sections[UNIFIED_SECTION_LINUX]));
                const uint8_t *kernel = _binary_src_boot_pe_efi_start + sections[UNIFIED_SECTION_LINUX].file_offset;
                ASSERT_EQ(memcmp(kernel, "MZ\0\0", 4), 0);
                const char *cmdline = (const char *) _binary_src_boot_pe_efi_start + sections[UNIFIED_SECTION_CMDLINE].file_offset;
                ASSERT_STREQ(cmdline, "test");
                const char *osrel = (const char *) _binary_src_boot_pe_efi_start + sections[UNIFIED_SECTION_OSREL].file_offset;
                ASSERT_STREQ(osrel, "v7");
                if (PE_SECTION_VECTOR_IS_SET(&sections[UNIFIED_SECTION_DTBAUTO])) {
                        const uint8_t *dtb = _binary_src_boot_pe_efi_start + sections[UNIFIED_SECTION_DTBAUTO].file_offset;
                        const size_t dtb_length = sections[UNIFIED_SECTION_DTBAUTO].file_size;
                        ASSERT_EQ(devicetree_match_by_compatible(dtb, dtb_length, tests[i].compatible), EFI_SUCCESS);
                } else if (PE_SECTION_VECTOR_IS_SET(&sections[UNIFIED_SECTION_EFIFW])) {
                        const uint8_t *fw = _binary_src_boot_pe_efi_start + sections[UNIFIED_SECTION_EFIFW].file_offset;
                        const size_t fw_length = sections[UNIFIED_SECTION_EFIFW].file_size;
                        ASSERT_EQ(efifirmware_match_by_fwid(fw, fw_length, tests[i].fwid), EFI_SUCCESS);
                } else {
                        log_error("%s:%i: Assertion failed: expected to find .dtbauto or .efifw section, but found neither", PROJECT_FILE, __LINE__);
                        abort();
                }
        }
}

static int intro(void) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        return EXIT_SUCCESS;
#else
        return log_tests_skipped("cannot run CHID calculation on big-endian machine");
#endif
}

DEFINE_TEST_MAIN_WITH_INTRO(LOG_INFO, intro);

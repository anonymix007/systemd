/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include "edid.h"
#include "log.h"
#include "proto/edid-discovered.h"
#include "util.h"

const char map[] = {
        [0b00001] = 'A',
        [0b00010] = 'B',
        [0b00011] = 'C',
        [0b00100] = 'D',
        [0b00101] = 'E',
        [0b00110] = 'F',
        [0b00111] = 'G',
        [0b01000] = 'H',
        [0b01001] = 'I',
        [0b01010] = 'J',
        [0b01011] = 'K',
        [0b01100] = 'L',
        [0b01101] = 'M',
        [0b01110] = 'N',
        [0b01111] = 'O',
        [0b10000] = 'P',
        [0b10001] = 'Q',
        [0b10010] = 'R',
        [0b10011] = 'S',
        [0b10100] = 'T',
        [0b10101] = 'U',
        [0b10110] = 'V',
        [0b10111] = 'W',
        [0b11000] = 'X',
        [0b11001] = 'Y',
        [0b11010] = 'Z',
};

const uint8_t header[] = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00 };

EFI_STATUS print_edid(void) {
        EFI_STATUS status = EFI_SUCCESS;
        EFI_EDID_DISCOVERED_PROTOCOL edid = {};

        status = BS->LocateProtocol(MAKE_GUID_PTR(EFI_EDID_DISCOVERED_PROTOCOL), NULL, (void **) &edid);
        if (EFI_STATUS_IS_ERROR(status))
                return status;

        log_info("%s: EDID = {.SizeOfEdid = %u, .Edid = %p}", __func__, edid.SizeOfEdid, edid.Edid);

        if (edid.SizeOfEdid == 0)
                return EFI_UNSUPPORTED;
        if (edid.Edid == NULL)
                return EFI_UNSUPPORTED;

        if (edid.SizeOfEdid < 128)
                return EFI_BUFFER_TOO_SMALL;

        log_hexdump(L"EDID[0..19]:", edid.Edid, (size_t) 19);

        if (memcmp(edid.Edid, header, sizeof(header)) != 0)
                return EFI_INCOMPATIBLE_VERSION;

        uint16_t vendor = (edid.Edid[8] << 8) | edid.Edid[9];
        uint16_t product = (edid.Edid[11] << 8) | edid.Edid[10];

        char manufacturer_string[4] = {};
        for (size_t i = 0; i < 3; i++) {
                uint8_t letter = (vendor >> (5 * i)) & 0b11111;
                if (letter <= 0b11010)
                        manufacturer_string[i] = map[letter];
                else
                        log_info("%s: unknown letter: 0x%x", __func__, letter);
        }

        log_info("%s: vendor: 0x%x (%s), product: 0x%x", __func__, vendor, manufacturer_string, product);

        return status;
}

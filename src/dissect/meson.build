# SPDX-License-Identifier: LGPL-2.1-or-later

if conf.get('HAVE_BLKID') != 1
        subdir_done()
endif

executables += [
        executable_template + {
                'name' : 'systemd-dissect',
                'public' : true,
                'sources' : files('dissect.c'),
        },
]

install_symlink('mount.ddi',
                pointing_to : sbin_to_bin + 'systemd-dissect',
                install_dir : sbindir)

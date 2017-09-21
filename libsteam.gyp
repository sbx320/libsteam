{
	'targets': [
	{
		'target_name': 'libsteam',
		'dependencies': [
			"protobufs/protobufs-cpp.gyp:protobuf-steam",
			"zlib",
			"cryptopp",
		],
		'type': 'static_library',
		'include_dirs': [
			'libsteam',
			'vendor',
			'protobufs',
		],
		'direct_dependent_settings': {
          'include_dirs': [
			'libsteam',
			'vendor',
			'protobufs',
		  ],
        },
		'sources': [
			'<!@pymod_do_main(glob-files libsteam/**/*.cpp)',
			'<!@pymod_do_main(glob-files libsteam/**/*.h)',
		]
	},
	{
		'target_name': 'libdota2',
		'dependencies': [
			"libsteam",
			"protobufs/protobufs-cpp.gyp:protobuf-steam",
			"protobufs/protobufs-cpp.gyp:protobuf-dota",
		],
		'type': 'static_library',
		'include_dirs': [
			'libdota2',
		],
		'direct_dependent_settings': {
          'include_dirs': [
			'libsteam',
			'vendor',
			'protobufs',
		  	'libdota2',
		  ],
        },
		'sources': [
			'<!@pymod_do_main(glob-files libdota2/**/*.cpp)',
			'<!@pymod_do_main(glob-files libdota2/**/*.h)',
		]
	},
	{
		'target_name': 'zlib',
		'type': 'static_library',
		'include_dirs': [
			"vendor/zlib",
		],
		'conditions': [
			['OS=="linux"', {
				'!cflags': [
				  '-Weverything',
				],
				'cflags': [
				  '-w',
				]
			}
		]],
		'sources': [
			'<!@pymod_do_main(glob-files vendor/zlib/*.c)',
			'<!@pymod_do_main(glob-files vendor/zlib/*.h)',
		]
	},	
	{
		'target_name': 'cryptopp',
		'type': 'static_library',
		'include_dirs': [
			"vendor/cryptopp",
		],
		'conditions': [
			['OS=="linux"', {
				'cflags_cc!': [
					'-std=c++1z',
				],
				'!cflags': [
				  '-Weverything',
				],
				'cflags': [
				  '-w',
				]
			}
		]],
		'sources': [
			'<!@pymod_do_main(glob-files vendor/cryptopp/*.cpp)',
			'<!@pymod_do_main(glob-files vendor/cryptopp/*.h)',
		],
		'msvs_settings': {
          'VCCLCompilerTool': {
            'AdditionalOptions': ['/std:c++14'],
          },
        },
	},
	]
}

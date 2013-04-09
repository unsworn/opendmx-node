{
    "targets": [
        {
            "target_name": "opendmx",
            "variables": {
                'node_version': '<!(node --version | sed -e "s/^v\([0-9]*\\.[0-9]*\).*$/\\1/")'
            },
            "sources": [         
                "src/DmxMain.cpp",
                "src/StringUtils.cpp",
                "src/DmxBuffer.cpp",
                "src/DmxThread.cpp",
                "src/Mutex.cpp",
                "src/Thread.cpp",
                "src/Serial.cpp"
            ],
            "conditions": [
                ['OS=="mac"',
                    {
                        'include_dirs': [
                            "/opt/local/include"
                        ],
                        'defines': [
                            'DEBUG_FTD2XX=1'
                        ],
                        "link_settings": {
                            'libraries': ["-L/opt/local/lib", "-lftd2xx"]
                        },
                    }
                ],
                ['OS=="linux"',
                    {
                        'link_settings': {
                            'libraries': ["-lftd2xx"]
                        }
                    }
                ]
            ],
            "target_conditions": [
                ['node_version=="0.8"', { 'defines': ['NODE_TARGET_VERSION=8'] } ]
            ]
        }
    ]
}

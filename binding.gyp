{
  "targets": [
    {
      "target_name": "dmx_native",
      "sources": [ "dmx.cc" ],
      'include_dirs': [
        '<!(node -e "require(\'nan\')")',
        '/usr/include/libftdi1'
      ],
      "libraries": [ "-lftdi1", "-lpthread" ]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "dmx_native" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/dmx_native.node" ],
          "destination": "."
        }
      ]
    }
  ]
}

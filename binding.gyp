{
  "targets": [
    {
      "target_name": "dmx_native",
      "sources": [ "dmx.cc" ],
      'include_dirs': [
        '<!(node -e "require(\'nan\')")'
      ],
      "libraries": [ "-lftdi1" ]
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

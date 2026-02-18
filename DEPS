{
  "version": "1.4.5",
  "vars": {
    "PAG_GROUP": "https://github.com/libpag"
  },
  "repos": {
    "common": [
      {
        "url": "${PAG_GROUP}/tgfx.git",
        "commit": "fbb9102d0c65f7901aa309a5a4cb1baa3e1d5c12",
        "dir": "third_party/tgfx"
      }
    ]
  },
  "actions": {
    "mac": [
      {
        "command": "python tgfx/third_party/shaderc/utils/git-sync-deps",
        "dir": "third_party"
      }
    ],
    "common": [
      {
        "command": "depctl --clean",
        "dir": "third_party"
      }
    ]
  }
}
# mod-gather-more

<img src="icon.png" alt="" width="64" align="right">

An [AzerothCore](https://www.azerothcore.org/) (WotLK 3.3.5a) module that lets
players get more out of each gathering node, corpse and fishing pool. Ore veins
and herbs can be gathered several times, fishing pools give more catches,
corpses can be skinned more than once, and cloth and disenchanting yield more.

- **Author:** [dudemcbro](https://github.com/dudemcbro)
- **License:** [GNU GPL v3.0](LICENSE)
- **Works with:** AzerothCore WotLK (3.3.5a). Developed on the
  [mod-playerbots AzerothCore fork](https://github.com/mod-playerbots/azerothcore-wotlk)
  (`Playerbot` branch).

## Features

- **Mining and herb nodes.** 3 gathers per node by default: a fully looted node
  is reset to ready instead of despawning.
- **Fishing pools.** 3x catches per fishing pool (fishing hole) by default.
- **Fishing skill.** Each fishing attempt rolls for a skill-up twice by default.
- **Skinning.** 3 skins per corpse by default. Needs the optional
  [core patch](docs/core-patches.md).
- **Cloth drops (SQL).** Linen through Frostweave drop in stacks of 3 to 10.
  Drop chances are unchanged.
- **Disenchanting (SQL).** Disenchant counts are multiplied by 3, once.
- **Configurable.** Every multiplier is a config option and is re-read on
  `.reload config`.

The details, defaults and caveats are in [How it works](docs/how-it-works.md).

## Quick start

1. Clone into your AzerothCore `modules/` folder (keep the name `mod-gather-more`):
   ```sh
   cd <azerothcore>/modules
   git clone https://github.com/dudemcbro/mod-gather-more.git
   ```
2. Optional, only needed for multi-skinning: apply the core patch from the core root:
   ```sh
   cd <azerothcore>
   git apply modules/mod-gather-more/patches/0001-core-keep-skinned-corpse-until-skinnable-cleared.patch
   ```
   Without it every other feature still works. See
   [Optional core patch](docs/core-patches.md).
3. Re-run CMake, build and install the core.
4. Copy `mod_gather_more.conf.dist` to `mod_gather_more.conf` in your
   `etc/modules` folder and adjust it.
5. Restart `worldserver`. It applies the module's SQL files on start.

The full steps, the SQL files, updating and uninstalling are in
[docs/installation.md](docs/installation.md).

## Documentation

| Page | What's in it |
| --- | --- |
| [Installation](docs/installation.md) | Requirements, install, the SQL files, updating, upgrading from a manual install, uninstalling |
| [Configuration](docs/configuration.md) | Every option in `mod_gather_more.conf` |
| [How it works](docs/how-it-works.md) | What each feature does, its default, and notes on skill-ups and node types |
| [Optional core patch](docs/core-patches.md) | The skinning patch: what it changes, applying and removing it |
| [Known limitations](docs/known-limitations.md) | Gas extraction, fishing pool respawns, full bags, core loot updates |

## Contributing

Issues and pull requests are welcome. Work on a feature branch and open the pull
request against `main`. Commit messages follow
[Conventional Commits](https://www.conventionalcommits.org/) (`fix: ...`,
`feat: ...`, `docs: ...`).

## License

GNU General Public License v3.0. See [LICENSE](LICENSE).

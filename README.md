# mod-gather-more

An [AzerothCore](https://www.azerothcore.org/) (WotLK 3.3.5a) module that lets
players get more out of each gathering node, corpse and fishing pool.

## Features

| Feature | Default | How it works |
| --- | --- | --- |
| Mining and herb nodes | 3 gathers per node | After a node is fully looted it is reset to ready instead of despawning, until it has been gathered `Multiplier` times. |
| Fishing pools | 3x catches | The number of catches a fishing pool (fishing hole) allows is multiplied when the pool spawns. |
| Fishing skill | 2x | Each fishing attempt runs the fishing skill-up roll `SkillMultiplier` times. |
| Skinning | 3 skins per corpse | After the last item of a skinning loot is taken, the corpse is made skinnable again until it has been skinned `Multiplier` times. **Requires the core patch below.** |
| Cloth drops (SQL) | 3 to 10 per stack | Sets the stack size of Linen, Wool, Silk, Mageweave, Runecloth, Netherweave and Frostweave in `creature_loot_template`. Drop chances are unchanged. |
| Disenchanting (SQL) | 3x | Multiplies `MinCount` and `MaxCount` of every `disenchant_loot_template` row by 3, once. |

Notes:

- Repeat gathers of the same node give loot but no extra skill-ups: the core
  allows one skill-up per player per node until it respawns.
- Mining and herb multipliers only apply to nodes that despawn on use (normal
  ore veins and herbs).

## Requirements

- AzerothCore WotLK (3.3.5a). Developed on the
  [mod-playerbots AzerothCore fork](https://github.com/mod-playerbots/azerothcore-wotlk)
  (`Playerbot` branch).
- For multi-skinning: the core patch in `patches/` (see below). Without it the
  corpse still despawns right after the first skin, so only the other features
  work.

## Installation

1. Clone the module into the core's `modules` folder:

   ```sh
   cd <azerothcore>/modules
   git clone https://github.com/dudemcbro/mod-gather-more.git
   ```

2. (Optional, needed for multi-skinning) apply the core patch:

   ```sh
   cd <azerothcore>
   git apply modules/mod-gather-more/patches/0001-core-keep-skinned-corpse-until-skinnable-cleared.patch
   ```

   The patch changes one line in `Creature::AllLootRemovedFromCorpse` so a
   skinned corpse is only scheduled for removal once it is no longer
   skinnable. It has no effect while the module is disabled. To remove it
   later, run the same command with `git apply -R`.

3. Re-run CMake and rebuild the core. No `CMakeLists.txt` is needed: AzerothCore
   compiles everything under `src/` and calls `Addmod_gather_moreScripts()`.

4. Copy the config and adjust it (see below).

5. Apply the SQL files (see [SQL](#sql)) and restart `worldserver`.

## Configuration

The build installs `mod_gather_more.conf.dist` to your `etc/modules` folder.
Copy it to `mod_gather_more.conf` and edit as needed. Values below 1 are
treated as 1 (Blizzlike).

| Option | Default | Description |
| --- | --- | --- |
| `GatherMore.Enable` | `1` | Turn all script features on or off. |
| `GatherMore.Mining.Multiplier` | `3` | Gathers per mining node. |
| `GatherMore.Herbalism.Multiplier` | `3` | Gathers per herb node. |
| `GatherMore.Fishing.Multiplier` | `3` | Multiplier for the number of catches per fishing pool. |
| `GatherMore.Fishing.SkillMultiplier` | `2` | Fishing skill-up rolls per attempt. |
| `GatherMore.Skinning.Multiplier` | `3` | Skins per corpse (needs the core patch). |
| `GatherMore.GasExtraction.Multiplier` | `3` | Loot ticks per gas cloud for Extract Gas (spell 30427). Not active yet, see Known limitations. |

Settings are re-read on `.reload config`.

## SQL

The SQL files live in `data/sql/updates/db_world/`. The worldserver's database
updater only imports a module's `data/sql/world`, `data/sql/characters` and
`data/sql/auth` folders, so it does **not** pick these up. Apply them to your world
database (usually `acore_world`) yourself, in file-name order, from the module
directory:

```sh
cd <azerothcore>/modules/mod-gather-more
for f in data/sql/updates/db_world/*.sql; do mysql -u <user> -p acore_world < "$f"; done
```

| File | What it does | Safe to re-run |
| --- | --- | --- |
| `2026_02_12_00_cloth_x3.sql` | Sets cloth `MinCount`/`MaxCount` to 3/10 in `creature_loot_template`. | Yes |
| `2026_02_12_01_disenchant_x3.sql` | Multiplies disenchant counts by 3. Creates a `mod_gather_more_migrations` table in the world DB so it only ever applies once. | Yes (no-op after the first run) |
| `2026_02_13_00_skinning_multiskin.sql` | Binds `spell_gather_more_skinning` to Skinning (spell 8613). | Yes |

These changes overwrite the original values and there is no rollback script; see
[Uninstall](#uninstall).

## Known limitations

- **Gas extraction is not active.** The `spell_gather_more_extract_gas` aura
  script exists in C++, but no `spell_script_names` row binds it to spell
  30427 and it hooks `SPELL_AURA_PERIODIC_DUMMY`, while the core handles this
  spell as a periodic trigger aura. Extract Gas keeps its default behavior.
- `spell_gather_more_skinning` is an empty spell script; multi-skinning is
  handled by the player loot hook.
- Fishing pools that are not part of a pool re-roll their catch count on
  respawn without the multiplier. Pooled fishing holes (the usual case) are
  multiplied every time they spawn.
- If the last skinning item cannot be looted (full bags), that attempt still
  counts as one skin.

## Updating

```sh
cd <azerothcore>/modules/mod-gather-more
git pull
```

Re-run CMake, rebuild, and apply any SQL files that are new since your last update
(all of them can be run again safely). If `git pull` changed the core patch, reverse
the old one first (`git apply -R` with the old file), then apply the new one.

## Uninstall

1. Restore `creature_loot_template` and `disenchant_loot_template` from a backup (or
   from AzerothCore's `data/sql/base/db_world` files), then run on the world
   database:

   ```sql
   DROP TABLE IF EXISTS mod_gather_more_migrations;
   DELETE FROM spell_script_names WHERE ScriptName = 'spell_gather_more_skinning';
   ```

2. If you applied the core patch, remove it from the core root:

   ```sh
   git apply -R modules/mod-gather-more/patches/0001-core-keep-skinned-corpse-until-skinnable-cleared.patch
   ```

3. Delete `modules/mod-gather-more` and your server's
   `etc/modules/mod_gather_more.conf`, re-run CMake and rebuild.

## Contributing

Issues and pull requests are welcome. Work on a feature branch and open the pull
request against `main`. Commit messages follow
[Conventional Commits](https://www.conventionalcommits.org/) (`fix: ...`,
`feat: ...`, `docs: ...`).

## License

GNU General Public License v3.0, see [LICENSE](LICENSE).

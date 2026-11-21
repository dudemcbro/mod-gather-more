# How it works

What each feature does and its default.

## Features

| Feature | Default | How it works |
| --- | --- | --- |
| Mining and herb nodes | 3 gathers per node | After a node is fully looted it is reset to ready instead of despawning, until it has been gathered `Multiplier` times. |
| Fishing pools | 3x catches | The number of catches a fishing pool (fishing hole) allows is multiplied when the pool spawns. |
| Fishing skill | 2x | Each fishing attempt runs the fishing skill-up roll `SkillMultiplier` times. |
| Skinning | 3 skins per corpse | After the last item of a skinning loot is taken, the corpse is made skinnable again until it has been skinned `Multiplier` times. **Requires the [core patch](core-patches.md).** |
| Cloth drops (SQL) | 3 to 10 per stack | Sets the stack size of Linen, Wool, Silk, Mageweave, Runecloth, Netherweave and Frostweave in `creature_loot_template`. Drop chances are unchanged. |
| Disenchanting (SQL) | 3x | Multiplies `MinCount` and `MaxCount` of every `disenchant_loot_template` row by 3, once. |

The multipliers are set in [Configuration](configuration.md). The SQL features
are described in [Installation: SQL](installation.md#sql).

## Notes

- Repeat gathers of the same node give loot but no extra skill-ups: the core
  allows one skill-up per player per node until it respawns.
- Mining and herb multipliers only apply to nodes that despawn on use (normal
  ore veins and herbs).

See also [Known limitations](known-limitations.md).

# Configuration

Every option in `mod_gather_more.conf`.

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
| `GatherMore.Skinning.Multiplier` | `3` | Skins per corpse (needs the [core patch](core-patches.md)). |
| `GatherMore.GasExtraction.Multiplier` | `3` | Loot ticks per gas cloud for Extract Gas (spell 30427). Not active yet, see [Known limitations](known-limitations.md). |

Settings are re-read on `.reload config`.

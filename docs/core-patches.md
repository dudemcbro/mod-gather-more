# Optional core patch

The patch is only needed for multi-skinning.

## Skinning

Without the patch the corpse still despawns right after the first skin, so only
the other features work.

[`patches/0001-core-keep-skinned-corpse-until-skinnable-cleared.patch`](../patches/0001-core-keep-skinned-corpse-until-skinnable-cleared.patch)
changes one line in `Creature::AllLootRemovedFromCorpse` so a skinned corpse is
only scheduled for removal once it is no longer skinnable. It has no effect
while the module is disabled.

Apply it from the root of your AzerothCore source tree, then re-run CMake and
rebuild:

```sh
cd <azerothcore>
git apply modules/mod-gather-more/patches/0001-core-keep-skinned-corpse-until-skinnable-cleared.patch
```

To remove it later, run the same command with `git apply -R`:

```sh
git apply -R modules/mod-gather-more/patches/0001-core-keep-skinned-corpse-until-skinnable-cleared.patch
```

If `git pull` changed the core patch, reverse the old one first (`git apply -R`
with the old file), then apply the new one. See
[Updating](installation.md#updating).

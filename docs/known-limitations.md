# Known limitations

What does not work yet, and edge cases to know about.

- **Gas extraction is not active.** The `spell_gather_more_extract_gas` aura
  script exists in C++ but is not registered: no `spell_script_names` row binds
  it to spell 30427, and it hooks `SPELL_AURA_PERIODIC_DUMMY`, while the core handles this
  spell as a periodic trigger aura. Extract Gas keeps its default behavior.
- `spell_gather_more_skinning` is an empty spell script; multi-skinning is
  handled by the player loot hook.
- Fishing pools that are not part of a pool re-roll their catch count on
  respawn without the multiplier. Pooled fishing holes (the usual case) are
  multiplied every time they spawn.
- If the last skinning item cannot be looted (full bags), that attempt still
  counts as one skin.
- The cloth and disenchant SQL only changes the loot rows that exist when it
  runs. If a later core update replaces some of those rows, they get the core's
  values back. Running the cloth file again by hand fixes the cloth stacks; the
  disenchant file only multiplies once per world database.

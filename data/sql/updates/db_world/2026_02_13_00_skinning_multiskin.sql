-- mod-gather-more: bind skinning SpellScript to spell 8613 (idempotent)

INSERT IGNORE INTO spell_script_names (spell_id, ScriptName)
VALUES (8613, 'spell_gather_more_skinning');

-- mod-gather-more: multiply disenchant loot amounts by 3 (idempotent)

CREATE TABLE IF NOT EXISTS mod_gather_more_migrations (
  name VARCHAR(100) NOT NULL PRIMARY KEY,
  applied_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

START TRANSACTION;

INSERT IGNORE INTO mod_gather_more_migrations (name) VALUES ('2026_02_12_01_disenchant_x3');
SET @apply := ROW_COUNT();

UPDATE disenchant_loot_template
SET
  MinCount = MinCount * 3,
  MaxCount = MaxCount * 3
WHERE @apply = 1;

COMMIT;

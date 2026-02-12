-- mod-gather-more: multiply tailoring cloth drops by 3 (idempotent)
-- 2589 Linen, 2592 Wool, 4306 Silk, 4338 Mageweave, 14047 Runecloth, 21877 Netherweave, 33470 Frostweave

CREATE TABLE IF NOT EXISTS mod_gather_more_migrations (
  name VARCHAR(100) NOT NULL PRIMARY KEY,
  applied_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

START TRANSACTION;

INSERT IGNORE INTO mod_gather_more_migrations (name) VALUES ('2026_02_12_00_cloth_x3');
SET @apply := ROW_COUNT();

UPDATE creature_loot_template
SET
  MinCount = MinCount * 3,
  MaxCount = MaxCount * 3
WHERE @apply = 1
  AND Item IN (2589, 2592, 4306, 4338, 14047, 21877, 33470);

COMMIT;

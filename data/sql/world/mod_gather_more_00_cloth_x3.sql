-- mod-gather-more: set cloth stack size to 3..10 (idempotent)
-- 2589 Linen, 2592 Wool, 4306 Silk, 4338 Mageweave, 14047 Runecloth, 21877 Netherweave, 33470 Frostweave

UPDATE creature_loot_template
SET MinCount = 3,
    MaxCount = 10
WHERE Item IN (2589, 2592, 4306, 4338, 14047, 21877, 33470)
  AND Reference = 0
  AND (MinCount <> 3 OR MaxCount <> 10);

-- Earlier versions also changed reference rows that list these items, which the
-- core rejects (a reference row needs MinCount = MaxCount). Put them back to the
-- stock 1..1.
UPDATE creature_loot_template
SET MinCount = 1,
    MaxCount = 1
WHERE Item IN (2589, 2592, 4306, 4338, 14047, 21877, 33470)
  AND Reference <> 0
  AND MinCount = 3
  AND MaxCount = 10;

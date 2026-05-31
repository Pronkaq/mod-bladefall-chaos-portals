--
-- BladeFall Chaos Portals
--
-- Re-applicable SQL for the world database.
--

SET @BLADEFALL_CHAOS_SHARD_ITEM := 900001;
SET @BLADEFALL_CHAOS_PORTAL_GO  := 900001;
SET @BLADEFALL_CHAOS_INVADER    := 900001;

--
-- Chaos Shard
--
-- Clone an existing simple stackable item in a later iteration, then update:
--   - entry
--   - name
--   - Quality
--   - SellPrice = 0
--   - stackable
--   - bonding policy
--

--
-- Chaos Portal
--
DELETE FROM `gameobject_template`
WHERE `entry` = @BLADEFALL_CHAOS_PORTAL_GO;

INSERT INTO `gameobject_template`
SELECT
    @BLADEFALL_CHAOS_PORTAL_GO,
    `type`,
    `displayId`,
    'BladeFall Chaos Portal',
    `IconName`,
    `castBarCaption`,
    `unk1`,
    `size`,
    `Data0`,
    `Data1`,
    `Data2`,
    `Data3`,
    `Data4`,
    `Data5`,
    `Data6`,
    `Data7`,
    `Data8`,
    `Data9`,
    `Data10`,
    `Data11`,
    `Data12`,
    `Data13`,
    `Data14`,
    `Data15`,
    `Data16`,
    `Data17`,
    `Data18`,
    `Data19`,
    `Data20`,
    `Data21`,
    `Data22`,
    `Data23`,
    '',
    '',
    NULL
FROM `gameobject_template`
WHERE `entry` = 177193;

--
-- Chaos Invader
--
-- Creature template and models will be added in the next iteration.
--

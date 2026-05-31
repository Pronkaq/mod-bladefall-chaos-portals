--
-- BladeFall Chaos Portals
--
-- Re-applicable SQL for the world database.
--

SET @BLADEFALL_CHAOS_SHARD_ITEM := 900001;
SET @BLADEFALL_CHAOS_PORTAL_GO  := 900001;
SET @BLADEFALL_CHAOS_INVADER    := 900001;
SET @BLADEFALL_CHAOS_WEAKNESS   := 900001;

--
-- Chaos Shard
--
-- Clone Badge of Justice as a temporary technical template.
--

DELETE FROM `item_template`
WHERE `entry` = @BLADEFALL_CHAOS_SHARD_ITEM;

DROP TEMPORARY TABLE IF EXISTS `tmp_bladefall_chaos_shard`;

CREATE TEMPORARY TABLE `tmp_bladefall_chaos_shard`
LIKE `item_template`;

INSERT INTO `tmp_bladefall_chaos_shard`
SELECT *
FROM `item_template`
WHERE `entry` = 29434;

UPDATE `tmp_bladefall_chaos_shard`
SET
    `entry` = @BLADEFALL_CHAOS_SHARD_ITEM,
    `name` = 'Chaos Shard',
    `Quality` = 2,
    `BuyPrice` = 0,
    `SellPrice` = 0,
    `stackable` = 200,
    `description` = 'A fragment of unstable energy recovered from a Chaos Portal.',
    `VerifiedBuild` = NULL;

INSERT INTO `item_template`
SELECT *
FROM `tmp_bladefall_chaos_shard`;

DROP TEMPORARY TABLE IF EXISTS `tmp_bladefall_chaos_shard`;

DELETE FROM `item_template_locale`
WHERE `ID` = @BLADEFALL_CHAOS_SHARD_ITEM;

INSERT INTO `item_template_locale`
    (`ID`, `locale`, `Name`, `Description`, `VerifiedBuild`)
VALUES
    (
        @BLADEFALL_CHAOS_SHARD_ITEM,
        'ruRU',
        'Осколок хаоса',
        'Фрагмент нестабильной энергии, полученный после закрытия портала хаоса.',
        NULL
    );

--
-- Chaos Portal
--
-- Clone the visual configuration of Doom Portal (177193).
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
-- Clone Restless Skeleton (17261), but remove normal loot and use a dedicated
-- C++ AI script. Reward handling belongs to the encounter controller.
--

DELETE FROM `creature_template`
WHERE `entry` = @BLADEFALL_CHAOS_INVADER;

DROP TEMPORARY TABLE IF EXISTS `tmp_bladefall_chaos_invader`;

CREATE TEMPORARY TABLE `tmp_bladefall_chaos_invader`
LIKE `creature_template`;

INSERT INTO `tmp_bladefall_chaos_invader`
SELECT *
FROM `creature_template`
WHERE `entry` = 17261;

UPDATE `tmp_bladefall_chaos_invader`
SET
    `entry` = @BLADEFALL_CHAOS_INVADER,
    `name` = 'Chaos Invader',
    `subname` = 'BladeFall Chaos Portal',
    `minlevel` = 68,
    `maxlevel` = 80,
    `faction` = 14,
    `lootid` = 0,
    `pickpocketloot` = 0,
    `skinloot` = 0,
    `mingold` = 0,
    `maxgold` = 0,
    `AIName` = '',
    `ScriptName` = 'npc_bladefall_chaos_invader',
    `VerifiedBuild` = NULL;

INSERT INTO `creature_template`
SELECT *
FROM `tmp_bladefall_chaos_invader`;

DROP TEMPORARY TABLE IF EXISTS `tmp_bladefall_chaos_invader`;

DELETE FROM `creature_template_model`
WHERE `CreatureID` = @BLADEFALL_CHAOS_INVADER;

INSERT INTO `creature_template_model`
    (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`)
SELECT
    @BLADEFALL_CHAOS_INVADER,
    `Idx`,
    `CreatureDisplayID`,
    `DisplayScale`,
    `Probability`,
    NULL
FROM `creature_template_model`
WHERE `CreatureID` = 17261;

--
-- Chaos Weakness
--
-- Clone a known server spell as a structural template and turn its first
-- effect into a -10% all-stats aura.
--
-- The custom server-side spell ID is not present in an unmodified client
-- Spell.dbc. The mechanical effect works, but the stock client may not render
-- a complete tooltip or icon until a client patch is introduced.
--

DELETE FROM `spell_dbc`
WHERE `ID` = @BLADEFALL_CHAOS_WEAKNESS;

DROP TEMPORARY TABLE IF EXISTS `tmp_bladefall_chaos_weakness`;

CREATE TEMPORARY TABLE `tmp_bladefall_chaos_weakness`
LIKE `spell_dbc`;

INSERT INTO `tmp_bladefall_chaos_weakness`
SELECT *
FROM `spell_dbc`
WHERE `ID` = 24899;

UPDATE `tmp_bladefall_chaos_weakness`
SET
    `ID` = @BLADEFALL_CHAOS_WEAKNESS,
    `DurationIndex` = 0,

    `Effect_1` = 6,
    `EffectAura_1` = 137,
    `EffectBasePoints_1` = -11,
    `EffectMiscValue_1` = -1,

    `Effect_2` = 0,
    `EffectAura_2` = 0,
    `EffectBasePoints_2` = 0,
    `EffectMiscValue_2` = 0,

    `Effect_3` = 0,
    `EffectAura_3` = 0,
    `EffectBasePoints_3` = 0,
    `EffectMiscValue_3` = 0,

    `Name_Lang_enUS` = 'Chaos Weakness',
    `Name_Lang_enGB` = 'Chaos Weakness',
    `Name_Lang_ruRU` = 'Слабость хаоса',
    `Name_Lang_Mask` = 0;

INSERT INTO `spell_dbc`
SELECT *
FROM `tmp_bladefall_chaos_weakness`;

DROP TEMPORARY TABLE IF EXISTS `tmp_bladefall_chaos_weakness`;

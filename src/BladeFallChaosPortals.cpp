/*
 * BladeFall Chaos Portals
 */

#include "AllCreatureScript.h"
#include "AreaDefines.h"
#include "Chat.h"
#include "Config.h"
#include "Creature.h"
#include "CreatureScript.h"
#include "GameObject.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "Random.h"
#include "ScriptedCreature.h"
#include "SpellAuras.h"
#include "TemporarySummon.h"
#include "WorldScript.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
constexpr uint32 ITEM_CHAOS_SHARD = 900001;
constexpr uint32 GO_CHAOS_PORTAL = 900001;
constexpr uint32 NPC_CHAOS_INVADER = 900001;
constexpr uint32 SPELL_CHAOS_WEAKNESS = 900001;

constexpr uint32 DEFAULT_SPAWN_INTERVAL_MS = 10 * MINUTE * IN_MILLISECONDS;
constexpr uint32 DEFAULT_ENCOUNTER_DURATION_MS = MINUTE * IN_MILLISECONDS;
constexpr uint32 DEFAULT_FAILURE_DEBUFF_DURATION_MS = MINUTE * IN_MILLISECONDS;
constexpr float TWO_PI = 6.28318530718f;

struct ChaosPortalConfig
{
    bool Enabled = true;
    uint32 SpawnIntervalMs = DEFAULT_SPAWN_INTERVAL_MS;
    uint32 DurationMs = DEFAULT_ENCOUNTER_DURATION_MS;
    uint32 MinCreatures = 1;
    uint32 MaxCreatures = 5;
    float DamageMultiplier = 0.90f;
    uint32 RewardItem = ITEM_CHAOS_SHARD;
    uint32 ShardsPerCreature = 1;
    uint32 FailureDebuffSpell = SPELL_CHAOS_WEAKNESS;
    uint32 FailureDebuffDurationMs = DEFAULT_FAILURE_DEBUFF_DURATION_MS;
    uint32 PortalGameObject = GO_CHAOS_PORTAL;
    uint32 CreatureEntry = NPC_CHAOS_INVADER;
};

bool IsMajorCity(uint32 zoneId)
{
    switch (zoneId)
    {
        case AREA_UNDERCITY:
        case AREA_STORMWIND_CITY:
        case AREA_IRONFORGE:
        case AREA_ORGRIMMAR:
        case AREA_THUNDER_BLUFF:
        case AREA_DARNASSUS:
        case AREA_SILVERMOON_CITY:
        case AREA_THE_EXODAR:
        case AREA_SHATTRATH_CITY:
        case AREA_DALARAN:
            return true;
        default:
            return false;
    }
}

bool IsEligibleTarget(Player const* player)
{
    if (!player || !player->IsInWorld() || !player->IsAlive())
        return false;

    if (player->IsBeingTeleported() || player->IsFlying() || player->isAFK())
        return false;

    if (player->IsGameMaster() || player->InBattleground())
        return false;

    if (!player->FindMap() || player->GetMap()->Instanceable())
        return false;

    if (IsMajorCity(player->GetZoneId()))
        return false;

    return true;
}

void SendSystemMessage(Player* player, std::string_view message)
{
    if (player && player->GetSession())
        ChatHandler(player->GetSession()).SendSysMessage(message);
}

class ChaosPortalManager
{
public:
    static ChaosPortalManager& Instance()
    {
        static ChaosPortalManager instance;
        return instance;
    }

    void LoadConfig()
    {
        _config.Enabled =
            sConfigMgr->GetOption<bool>("BladeFall.ChaosPortals.Enable", true);

        _config.SpawnIntervalMs =
            std::max<uint32>(
                1000,
                sConfigMgr->GetOption<uint32>(
                    "BladeFall.ChaosPortals.SpawnIntervalMs",
                    DEFAULT_SPAWN_INTERVAL_MS));

        _config.DurationMs =
            std::max<uint32>(
                1000,
                sConfigMgr->GetOption<uint32>(
                    "BladeFall.ChaosPortals.DurationMs",
                    DEFAULT_ENCOUNTER_DURATION_MS));

        _config.MinCreatures =
            std::max<uint32>(
                1,
                sConfigMgr->GetOption<uint32>(
                    "BladeFall.ChaosPortals.MinCreatures",
                    1));

        _config.MaxCreatures =
            std::max<uint32>(
                _config.MinCreatures,
                sConfigMgr->GetOption<uint32>(
                    "BladeFall.ChaosPortals.MaxCreatures",
                    5));

        _config.DamageMultiplier =
            std::max<float>(
                0.01f,
                sConfigMgr->GetOption<float>(
                    "BladeFall.ChaosPortals.DamageMultiplier",
                    0.90f));

        _config.RewardItem =
            sConfigMgr->GetOption<uint32>(
                "BladeFall.ChaosPortals.RewardItem",
                ITEM_CHAOS_SHARD);

        _config.ShardsPerCreature =
            std::max<uint32>(
                1,
                sConfigMgr->GetOption<uint32>(
                    "BladeFall.ChaosPortals.ShardsPerCreature",
                    1));

        _config.FailureDebuffSpell =
            sConfigMgr->GetOption<uint32>(
                "BladeFall.ChaosPortals.FailureDebuffSpell",
                SPELL_CHAOS_WEAKNESS);

        _config.FailureDebuffDurationMs =
            std::max<uint32>(
                1000,
                sConfigMgr->GetOption<uint32>(
                    "BladeFall.ChaosPortals.FailureDebuffDurationMs",
                    DEFAULT_FAILURE_DEBUFF_DURATION_MS));

        _config.PortalGameObject =
            sConfigMgr->GetOption<uint32>(
                "BladeFall.ChaosPortals.PortalGameObject",
                GO_CHAOS_PORTAL);

        _config.CreatureEntry =
            sConfigMgr->GetOption<uint32>(
                "BladeFall.ChaosPortals.CreatureEntry",
                NPC_CHAOS_INVADER);

        if (!_active)
            _nextEncounterTimerMs = _config.SpawnIntervalMs;
    }

    void Update(uint32 diff)
    {
        if (!_config.Enabled)
            return;

        if (_active)
        {
            if (_finishSuccessPending)
            {
                FinishSuccess();
                return;
            }

            Player* target = ObjectAccessor::FindConnectedPlayer(_targetGuid);
            if (!target || !target->IsInWorld())
            {
                ClearEncounter(target);
                return;
            }

            if (_encounterTimerMs <= diff)
            {
                FinishFailure(target);
                return;
            }

            _encounterTimerMs -= diff;
            return;
        }

        if (_nextEncounterTimerMs > diff)
        {
            _nextEncounterTimerMs -= diff;
            return;
        }

        _nextEncounterTimerMs = _config.SpawnIntervalMs;
        StartEncounter();
    }

    void OnBeforeCreatureSelectLevel(CreatureTemplate const* creatureTemplate, uint8& level) const
    {
        if (creatureTemplate && creatureTemplate->Entry == _config.CreatureEntry && _spawnLevelOverride)
            level = _spawnLevelOverride;
    }

    void OnInvaderDied(ObjectGuid guid)
    {
        if (!_active)
            return;

        auto const itr = std::find(_invaderGuids.begin(), _invaderGuids.end(), guid);
        if (itr == _invaderGuids.end())
            return;

        _invaderGuids.erase(itr);

        if (_invaderGuids.empty())
            _finishSuccessPending = true;
    }

private:
    ChaosPortalManager() : _nextEncounterTimerMs(DEFAULT_SPAWN_INTERVAL_MS) { }

    Player* SelectRandomTarget() const
    {
        std::vector<Player*> candidates;

        for (auto const& [guid, player] : ObjectAccessor::GetPlayers())
        {
            (void)guid;

            if (IsEligibleTarget(player))
                candidates.push_back(player);
        }

        if (candidates.empty())
            return nullptr;

        return candidates[urand(0, uint32(candidates.size() - 1))];
    }

    void StartEncounter()
    {
        Player* target = SelectRandomTarget();
        if (!target)
            return;

        float const portalAngle = frand(0.0f, TWO_PI);
        float const portalDistance = frand(8.0f, 12.0f);
        float const portalX = target->GetPositionX() + std::cos(portalAngle) * portalDistance;
        float const portalY = target->GetPositionY() + std::sin(portalAngle) * portalDistance;
        float const portalZ = target->GetPositionZ();

        float const halfAngle = portalAngle * 0.5f;

        GameObject* portal = target->SummonGameObject(
            _config.PortalGameObject,
            portalX,
            portalY,
            portalZ,
            portalAngle,
            0.0f,
            0.0f,
            std::sin(halfAngle),
            std::cos(halfAngle),
            std::max<uint32>(1, _config.DurationMs / IN_MILLISECONDS));

        if (!portal)
            return;

        _targetGuid = target->GetGUID();
        _portalGuid = portal->GetGUID();
        _initialCreatureCount = urand(_config.MinCreatures, _config.MaxCreatures);
        _encounterTimerMs = _config.DurationMs;
        _finishSuccessPending = false;
        _active = true;

        for (uint32 index = 0; index < _initialCreatureCount; ++index)
        {
            float const creatureAngle = frand(0.0f, TWO_PI);
            float const creatureDistance = frand(2.0f, 4.0f);
            float const creatureX = portalX + std::cos(creatureAngle) * creatureDistance;
            float const creatureY = portalY + std::sin(creatureAngle) * creatureDistance;

            _spawnLevelOverride = target->GetLevel();

            TempSummon* invader = target->SummonCreature(
                _config.CreatureEntry,
                creatureX,
                creatureY,
                portalZ,
                creatureAngle,
                TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,
                _config.DurationMs);

            _spawnLevelOverride = 0;

            if (!invader)
                continue;

            ScaleInvaderDamage(invader);
            _invaderGuids.push_back(invader->GetGUID());

            if (invader->AI())
                invader->AI()->AttackStart(target);
        }

        if (_invaderGuids.empty())
        {
            ClearEncounter(target);
            return;
        }

        _initialCreatureCount = uint32(_invaderGuids.size());

        SendSystemMessage(
            target,
            "A Chaos Portal has opened nearby! Defeat its invaders within one minute.");
    }

    void ScaleInvaderDamage(Creature* invader) const
    {
        for (WeaponAttackType attackType : { BASE_ATTACK, OFF_ATTACK, RANGED_ATTACK })
        {
            float const minimum =
                invader->GetWeaponDamageRange(attackType, MINDAMAGE) *
                _config.DamageMultiplier;

            float const maximum =
                invader->GetWeaponDamageRange(attackType, MAXDAMAGE) *
                _config.DamageMultiplier;

            invader->SetBaseWeaponDamage(attackType, MINDAMAGE, minimum);
            invader->SetBaseWeaponDamage(attackType, MAXDAMAGE, maximum);
            invader->UpdateDamagePhysical(attackType);
        }
    }

    void FinishSuccess()
    {
        Player* target = ObjectAccessor::FindConnectedPlayer(_targetGuid);

        if (target && target->IsInWorld())
        {
            uint32 const rewardCount = _initialCreatureCount * _config.ShardsPerCreature;

            if (target->AddItem(_config.RewardItem, rewardCount))
            {
                ChatHandler(target->GetSession()).PSendSysMessage(
                    "The Chaos Portal collapses. You receive {} Chaos Shard(s).",
                    rewardCount);
            }
            else
            {
                SendSystemMessage(
                    target,
                    "The Chaos Portal collapses, but your inventory is full. The reward could not be delivered.");
            }
        }

        ClearEncounter(target);
    }

    void FinishFailure(Player* target)
    {
        if (target && target->IsInWorld())
        {
            if (Aura* aura = target->AddAura(_config.FailureDebuffSpell, target))
            {
                aura->SetMaxDuration(_config.FailureDebuffDurationMs);
                aura->SetDuration(_config.FailureDebuffDurationMs);
            }

            SendSystemMessage(
                target,
                "The Chaos Portal closes before you defeat its invaders. Chaos Weakness afflicts you.");
        }

        ClearEncounter(target);
    }

    void ClearEncounter(Player* target)
    {
        if (target && target->IsInWorld())
        {
            if (GameObject* portal = ObjectAccessor::GetGameObject(*target, _portalGuid))
                portal->DespawnOrUnsummon();

            for (ObjectGuid const& invaderGuid : _invaderGuids)
            {
                if (Creature* invader = ObjectAccessor::GetCreature(*target, invaderGuid))
                    invader->DespawnOrUnsummon();
            }
        }

        _targetGuid.Clear();
        _portalGuid.Clear();
        _invaderGuids.clear();
        _initialCreatureCount = 0;
        _encounterTimerMs = 0;
        _spawnLevelOverride = 0;
        _finishSuccessPending = false;
        _active = false;
        _nextEncounterTimerMs = _config.SpawnIntervalMs;
    }

    ChaosPortalConfig _config;
    ObjectGuid _targetGuid;
    ObjectGuid _portalGuid;
    std::vector<ObjectGuid> _invaderGuids;
    uint32 _nextEncounterTimerMs;
    uint32 _encounterTimerMs = 0;
    uint32 _initialCreatureCount = 0;
    uint8 _spawnLevelOverride = 0;
    bool _finishSuccessPending = false;
    bool _active = false;
};

class BladeFallChaosPortalWorldScript : public WorldScript
{
public:
    BladeFallChaosPortalWorldScript()
        : WorldScript(
            "BladeFallChaosPortalWorldScript",
            {
                WORLDHOOK_ON_AFTER_CONFIG_LOAD,
                WORLDHOOK_ON_UPDATE
            })
    {
    }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        ChaosPortalManager::Instance().LoadConfig();
    }

    void OnUpdate(uint32 diff) override
    {
        ChaosPortalManager::Instance().Update(diff);
    }
};

class BladeFallChaosPortalAllCreatureScript : public AllCreatureScript
{
public:
    BladeFallChaosPortalAllCreatureScript()
        : AllCreatureScript("BladeFallChaosPortalAllCreatureScript")
    {
    }

    void OnBeforeCreatureSelectLevel(
        CreatureTemplate const* creatureTemplate,
        Creature* /*creature*/,
        uint8& level) override
    {
        ChaosPortalManager::Instance().OnBeforeCreatureSelectLevel(creatureTemplate, level);
    }
};

struct npc_bladefall_chaos_invader : public ScriptedAI
{
    explicit npc_bladefall_chaos_invader(Creature* creature)
        : ScriptedAI(creature)
    {
    }

    void JustDied(Unit* /*killer*/) override
    {
        ChaosPortalManager::Instance().OnInvaderDied(me->GetGUID());
    }

    void UpdateAI(uint32 /*diff*/) override
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};
}

void AddBladeFallChaosPortalScripts()
{
    new BladeFallChaosPortalWorldScript();
    new BladeFallChaosPortalAllCreatureScript();
    RegisterCreatureAI(npc_bladefall_chaos_invader);
}

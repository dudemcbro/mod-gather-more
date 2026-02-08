#include "AllGameObjectScript.h"
#include "AllCreatureScript.h"
#include "Config.h"
#include "Creature.h"
#include "DBCStores.h"
#include "GameObject.h"
#include "GlobalScript.h"
#include "Log.h"
#include "LootMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "SpellScript.h"

#include <algorithm>
#include <limits>
#include <unordered_map>

namespace
{
bool sEnabled = true;
uint32 sMiningMultiplier = 5;
uint32 sHerbalismMultiplier = 5;
uint32 sFishingMultiplier = 5;
uint32 sSkinningMultiplier = 5;
uint32 sGasExtractionMultiplier = 5;

static std::unordered_map<ObjectGuid, uint32> sFishingHoleBaseMaxOpens;
static std::unordered_map<ObjectGuid, uint32> sGasCloudExtractionCount;

enum class GatheringType : uint8
{
    None,
    Mining,
    Herbalism
};

GatheringType GetGatheringType(GameObject const* go)
{
    if (!go)
        return GatheringType::None;

    GameObjectTemplate const* info = go->GetGOInfo();
    if (!info || !info->IsDespawnAtAction())
        return GatheringType::None;

    uint32 lockId = info->GetLockId();
    if (!lockId)
        return GatheringType::None;

    LockEntry const* lock = sLockStore.LookupEntry(lockId);
    if (!lock)
        return GatheringType::None;

    for (uint8 i = 0; i < MAX_LOCK_CASE; ++i)
    {
        if (!lock->Type[i])
            continue;

        if (lock->Type[i] != LOCK_KEY_SKILL)
            continue;

        if (lock->Index[i] == LOCKTYPE_MINING)
            return GatheringType::Mining;

        if (lock->Index[i] == LOCKTYPE_HERBALISM)
            return GatheringType::Herbalism;
    }

    return GatheringType::None;
}

uint32 GetConfiguredMultiplier(GatheringType type)
{
    switch (type)
    {
        case GatheringType::Mining:
            return std::max<uint32>(1u, sMiningMultiplier);
        case GatheringType::Herbalism:
            return std::max<uint32>(1u, sHerbalismMultiplier);
        default:
            return 1u;
    }
}
} // namespace

class MoreNodes_WorldScript final : public WorldScript
{
public:
    MoreNodes_WorldScript() : WorldScript("MoreNodes_WorldScript") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        sEnabled = sConfigMgr->GetOption<bool>("MoreNodes.Enable", true);
        sMiningMultiplier = sConfigMgr->GetOption<uint32>("MoreNodes.Mining.Multiplier", 5);
        sHerbalismMultiplier = sConfigMgr->GetOption<uint32>("MoreNodes.Herbalism.Multiplier", 5);
        sFishingMultiplier = sConfigMgr->GetOption<uint32>("MoreNodes.Fishing.Multiplier", 5);
        sSkinningMultiplier = sConfigMgr->GetOption<uint32>("MoreNodes.Skinning.Multiplier", 5);
        sGasExtractionMultiplier = sConfigMgr->GetOption<uint32>("MoreNodes.GasExtraction.Multiplier", 5);

        LOG_INFO("module", "mod-more-nodes: Enable={}, Mining.Multiplier={}, Herbalism.Multiplier={}, Fishing.Multiplier={}, Skinning.Multiplier={}, GasExtraction.Multiplier={}",
            sEnabled ? 1 : 0, sMiningMultiplier, sHerbalismMultiplier, sFishingMultiplier, sSkinningMultiplier, sGasExtractionMultiplier);
    }
};

class MoreNodes_AllGameObjectScript final : public AllGameObjectScript
{
public:
    MoreNodes_AllGameObjectScript() : AllGameObjectScript("MoreNodes_AllGameObjectScript") { }

    void OnGameObjectAddWorld(GameObject* go) override
    {
        if (!sEnabled || !go)
            return;

        if (go->GetGoType() != GAMEOBJECT_TYPE_FISHINGHOLE)
            return;

        uint32 multiplier = std::max<uint32>(1u, sFishingMultiplier);
        if (multiplier <= 1)
            return;

        GameObjectTemplate const* info = go->GetGOInfo();
        if (!info)
            return;

        auto* goValue = const_cast<GameObjectValue*>(go->GetGOValue());
        if (!goValue)
            return;

        uint32 base = 0;
        if (go->GetUseCount() == 0)
        {
            base = goValue->FishingHole.MaxOpens;
            if (!base)
                base = urand(info->fishinghole.minSuccessOpens, info->fishinghole.maxSuccessOpens);

            sFishingHoleBaseMaxOpens[go->GetGUID()] = base;
        }
        else
        {
            auto itr = sFishingHoleBaseMaxOpens.find(go->GetGUID());
            if (itr != sFishingHoleBaseMaxOpens.end())
                base = itr->second;
            else
                base = goValue->FishingHole.MaxOpens;
        }

        base = std::max<uint32>(1u, base);
        uint64 scaled64 = uint64(base) * uint64(multiplier);
        uint32 scaled = scaled64 > std::numeric_limits<uint32>::max() ? std::numeric_limits<uint32>::max() : uint32(scaled64);
        goValue->FishingHole.MaxOpens = std::max<uint32>(1u, scaled);
    }

    void OnGameObjectLootStateChanged(GameObject* go, uint32 state, Unit* /*unit*/) override
    {
        if (!sEnabled || !go)
            return;

        if (state != GO_JUST_DEACTIVATED)
            return;

        if (!go->loot.isLooted())
            return;

        GatheringType type = GetGatheringType(go);
        uint32 multiplier = GetConfiguredMultiplier(type);
        if (multiplier <= 1)
            return;

        go->AddUse();

        if (go->GetUseCount() < multiplier)
            go->SetLootState(GO_READY);
    }
};

class MoreNodes_AllCreatureScript final : public AllCreatureScript
{
public:
    MoreNodes_AllCreatureScript() : AllCreatureScript("MoreNodes_AllCreatureScript") { }

    void OnCreatureRemoveWorld(Creature* creature) override
    {
        if (!creature)
            return;

        sGasCloudExtractionCount.erase(creature->GetGUID());
    }
};

class MoreNodes_GlobalScript final : public GlobalScript
{
public:
    MoreNodes_GlobalScript() : GlobalScript("MoreNodes_GlobalScript", { GLOBALHOOK_ON_AFTER_REF_COUNT }) { }

    void OnAfterRefCount(Player const* /*player*/, LootStoreItem* /*lootStoreItem*/, Loot& loot, bool /*canRate*/, uint16 /*lootMode*/, uint32& maxcount, LootStore const& /*store*/) override
    {
        if (!sEnabled)
            return;

        if (loot.loot_type != LOOT_SKINNING)
            return;

        uint32 multiplier = std::max<uint32>(1u, sSkinningMultiplier);
        if (multiplier <= 1)
            return;

        if (!maxcount)
            return;

        uint64 scaled64 = uint64(maxcount) * uint64(multiplier);
        uint32 scaled = scaled64 > std::numeric_limits<uint32>::max() ? std::numeric_limits<uint32>::max() : uint32(scaled64);

        // keep within sane bounds for client/inventory
        maxcount = std::min<uint32>(scaled, 1000u);
    }
};

// Spell 30427 - Extract Gas (Zapthrottle Mote Extractor)
class spell_more_nodes_extract_gas final : public SpellScriptLoader
{
public:
    spell_more_nodes_extract_gas() : SpellScriptLoader("spell_more_nodes_extract_gas") { }

    class spell_more_nodes_extract_gas_AuraScript final : public AuraScript
    {
        PrepareAuraScript(spell_more_nodes_extract_gas_AuraScript);

        void HandlePeriodic(AuraEffect const* /*aurEff*/)
        {
            if (!sEnabled)
                return;

            PreventDefaultAction();

            Unit* caster = GetCaster();
            Unit* target = GetTarget();
            if (!caster || !target)
                return;

            Player* player = caster->ToPlayer();
            Creature* creature = target->ToCreature();
            if (!player || !creature)
                return;

            if (creature->GetCreatureTemplate()->type != CREATURE_TYPE_GAS_CLOUD)
                return;

            uint32 multiplier = std::max<uint32>(1u, sGasExtractionMultiplier);
            if (multiplier <= 1)
            {
                creature->DespawnOrUnsummon();
                return;
            }

            uint32 lootId = creature->GetCreatureTemplate()->SkinLootId;
            if (!lootId)
                return;

            uint32& extracted = sGasCloudExtractionCount[creature->GetGUID()];
            if (extracted >= multiplier)
                return;

            player->AutoStoreLoot(lootId, LootTemplates_Skinning, true);
            ++extracted;

            if (extracted >= multiplier)
            {
                sGasCloudExtractionCount.erase(creature->GetGUID());
                creature->DespawnOrUnsummon();
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_more_nodes_extract_gas_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_more_nodes_extract_gas_AuraScript();
    }
};

void Addmod_more_nodesScripts()
{
    new MoreNodes_WorldScript();
    new MoreNodes_AllGameObjectScript();
    new MoreNodes_AllCreatureScript();
    new MoreNodes_GlobalScript();
    new spell_more_nodes_extract_gas();
}

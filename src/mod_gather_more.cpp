#include "AllGameObjectScript.h"
#include "AllCreatureScript.h"
#include "Config.h"
#include "Creature.h"
#include "DBCStores.h"
#include "GameObject.h"
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
uint32 sMiningMultiplier = 3;
uint32 sHerbalismMultiplier = 3;
uint32 sFishingMultiplier = 3;
uint32 sSkinningMultiplier = 3;
uint32 sGasExtractionMultiplier = 3;

static std::unordered_map<ObjectGuid, uint32> sFishingHoleBaseMaxOpens;
static std::unordered_map<ObjectGuid, uint32> sGasCloudExtractionCount;
static std::unordered_map<ObjectGuid, uint32> sSkinningCount;

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

class GatherMore_WorldScript final : public WorldScript
{
public:
    GatherMore_WorldScript() : WorldScript("GatherMore_WorldScript") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        sEnabled = sConfigMgr->GetOption<bool>("GatherMore.Enable", true);
        sMiningMultiplier = sConfigMgr->GetOption<uint32>("GatherMore.Mining.Multiplier", 3);
        sHerbalismMultiplier = sConfigMgr->GetOption<uint32>("GatherMore.Herbalism.Multiplier", 3);
        sFishingMultiplier = sConfigMgr->GetOption<uint32>("GatherMore.Fishing.Multiplier", 3);
        sSkinningMultiplier = sConfigMgr->GetOption<uint32>("GatherMore.Skinning.Multiplier", 3);
        sGasExtractionMultiplier = sConfigMgr->GetOption<uint32>("GatherMore.GasExtraction.Multiplier", 3);

        LOG_INFO("module", "mod-gather-more: Enable={}, Mining.Multiplier={}, Herbalism.Multiplier={}, Fishing.Multiplier={}, Skinning.Multiplier={}, GasExtraction.Multiplier={}",
            sEnabled ? 1 : 0, sMiningMultiplier, sHerbalismMultiplier, sFishingMultiplier, sSkinningMultiplier, sGasExtractionMultiplier);
    }
};

class GatherMore_AllGameObjectScript final : public AllGameObjectScript
{
public:
    GatherMore_AllGameObjectScript() : AllGameObjectScript("GatherMore_AllGameObjectScript") { }

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
        {
            go->ClearSkillupList();
            go->SetLootState(GO_READY);
        }
    }
};

class GatherMore_AllCreatureScript final : public AllCreatureScript
{
public:
    GatherMore_AllCreatureScript() : AllCreatureScript("GatherMore_AllCreatureScript") { }

    void OnCreatureRemoveWorld(Creature* creature) override
    {
        if (!creature)
            return;

        sGasCloudExtractionCount.erase(creature->GetGUID());
        sSkinningCount.erase(creature->GetGUID());
    }
};


// Spell 30427 - Extract Gas (Zapthrottle Mote Extractor)
class spell_gather_more_extract_gas final : public SpellScriptLoader
{
public:
    spell_gather_more_extract_gas() : SpellScriptLoader("spell_gather_more_extract_gas") { }

    class spell_gather_more_extract_gas_AuraScript final : public AuraScript
    {
        PrepareAuraScript(spell_gather_more_extract_gas_AuraScript);

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
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_gather_more_extract_gas_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_gather_more_extract_gas_AuraScript();
    }
};


// Spell 8613 - Skinning (multi-use per corpse)
class spell_gather_more_skinning final : public SpellScriptLoader
{
public:
    spell_gather_more_skinning() : SpellScriptLoader("spell_gather_more_skinning") { }

    class spell_gather_more_skinning_SpellScript final : public SpellScript
    {
        PrepareSpellScript(spell_gather_more_skinning_SpellScript);

        void HandleHit(SpellEffIndex /*effIndex*/)
        {
            if (!sEnabled)
                return;

            Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
            Creature* creature = GetHitCreature();
            if (!player || !creature)
                return;

            uint32 multiplier = std::max<uint32>(1u, sSkinningMultiplier);
            if (multiplier <= 1)
                return;

            uint32 lootId = creature->GetCreatureTemplate()->SkinLootId;
            if (!lootId)
                return;

            uint32& skinned = sSkinningCount[creature->GetGUID()];
            if (skinned >= multiplier)
                return;

            PreventHitDefaultEffect();

            if (!creature->HasUnitFlag(UNIT_FLAG_SKINNABLE))
                return;

            player->AutoStoreLoot(lootId, LootTemplates_Skinning, true);

            uint32 skill = creature->GetCreatureTemplate()->GetRequiredLootSkill();
            int32 targetLevel = creature->GetLevel();
            int32 reqValue = targetLevel < 10 ? 0 : targetLevel < 20 ? (targetLevel - 10) * 10 : targetLevel * 5;
            int32 skillValue = player->GetPureSkillValue(skill);
            player->UpdateGatherSkill(skill, skillValue, reqValue, creature->isElite() ? 2 : 1);

            ++skinned;

            if (skinned >= multiplier)
                creature->RemoveUnitFlag(UNIT_FLAG_SKINNABLE);
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_gather_more_skinning_SpellScript::HandleHit, EFFECT_0, SPELL_EFFECT_SKINNING);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_gather_more_skinning_SpellScript();
    }
};
void Addmod_gather_moreScripts()
{
    new GatherMore_WorldScript();
    new GatherMore_AllGameObjectScript();
    new GatherMore_AllCreatureScript();
    new spell_gather_more_extract_gas();
    new spell_gather_more_skinning();
}


#include "include\Settings.h"

#include "include\PerkEntryPointExtenderAPI.h"

class Serialization {
public:
    Serialization() = default;
    Serialization(const Serialization&) = delete;
    Serialization(Serialization&&) = delete;

    ~Serialization() = default;

    Serialization& operator=(const Serialization&) = delete;
    Serialization& operator=(Serialization&&) = delete;

    mutable std::mutex lock;

    std::unordered_map<int, RE::ActorValue> collection;
    float movementangle = 90.0f;
    // std::vector<RE::ActiveEffect*> LastCastEffect;
    
    static auto GetSingleton() -> Serialization* {
        static Serialization singleton;
        return std::addressof(singleton);
    }
    static auto GetGlobalValue(int a_value) -> RE::ActorValue {
        return Serialization::GetSingleton()->collection[a_value];
    }

    static void SetGlobalValue(int a_value, RE::ActorValue a_amount) {
        Serialization::GetSingleton()->collection[a_value] = (a_amount);
    }

        static auto GetMovementAngle() -> float {
        return Serialization::GetSingleton()->movementangle;
    }
        static auto SetMovementAngle(float a_value)  { Serialization::GetSingleton()->movementangle = a_value;
        }


    // static void MultGlobalValue(int a_value, float a_amount) {
    //     Serialization::GetSingleton()->collection[a_value] =
    //         Serialization::GetSingleton()->collection[a_value] * (a_amount);
    // }

    // static void DivideGlobalValue(int a_value, float a_amount) {
    //     Serialization::GetSingleton()->collection[a_value] =
    //         Serialization::GetSingleton()->collection[a_value] / (a_amount);
    // }

    // static void AddGlobalValue(int a_value, float a_amount) {
    //     Serialization::GetSingleton()->collection[a_value] =
    //         (Serialization::GetSingleton()->collection[a_value] + (a_amount));
};

[[nodiscard]] inline RE::BShkbAnimationGraph* GetActorFromHkbCharacter(RE::hkbCharacter* a_hkbCharacter) {
    if (a_hkbCharacter) {
         RE::BShkbAnimationGraph* animGraph =
            SKSE::stl::adjust_pointer<RE::BShkbAnimationGraph>(a_hkbCharacter, -0xC0);
        return animGraph;
    }

    return nullptr;
}
[[nodiscard]] inline RE::Actor* GetActorFromMagicTarget(RE::MagicTarget* a_MT) {
    if (a_MT) {
        RE::Actor* a_actor = SKSE::stl::adjust_pointer<RE::Actor>(a_MT, -0x0A0);
        return a_actor;
    }

    return nullptr;
}


static float* g_deltaTime = (float*)RELOCATION_ID(523660, 410199).address();          // 2F6B948, 30064C8
static float* g_deltaTimeRealTime = (float*)RELOCATION_ID(523661, 410200).address();  // 2F6B94C, 30064CC

static RE::ActorValue LookupActorValueByName(const char* av_name) {
    // SE: 0x3E1450, AE: 0x3FC5A0, VR: ---
    using func_t = decltype(&LookupActorValueByName);
    REL::Relocation<func_t> func{REL::RelocationID(26570, 27203)};
    return func(av_name);
}

void CastSpellsPerk(std::vector<RE::SpellItem*> a_MIVector, RE::Actor* caster, RE::Actor* Target) {
    for (auto* a_MI : a_MIVector) {
        if (a_MI->IsPermanent()) {
            RE::SpellItem* Spell = a_MI->As<RE::SpellItem>();
            if (Spell) {
                Target->AddSpell(Spell);
            }

        } else {
            caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
                ->CastSpellImmediate(a_MI, false, Target, 1.0f, false, 0.0f, nullptr);
        }
    }
}


void CastSpellsTarget(int num, RE::Actor* caster, RE::Actor* Target) {

    std::vector<RE::SpellItem*> vecSpells;
    RE::HandleEntryPoint(RE::PerkEntryPoint::kApplyBashingSpell, caster, &vecSpells, "CastSpell", num, {Target});
    CastSpellsPerk(vecSpells, caster, Target);

}

void CastSpellsWeapon(int num, RE::Actor* caster, RE::TESObjectWEAP* weapon, RE::Actor* Target) {
    std::vector<RE::SpellItem*> vecSpells;
    RE::HandleEntryPoint(RE::PerkEntryPoint::kApplyCombatHitSpell, caster, &vecSpells, "CastSpell", num, {weapon, Target});
    CastSpellsPerk(vecSpells, caster, Target);
}



bool IsCurrentAnimSubString(std::string anim_name, std::string str_check) {
    std::reverse(anim_name.begin(), anim_name.end());
    auto result = anim_name.find("\\");
    auto flippedname = anim_name.substr(4, result - 4);
    std::reverse(flippedname.begin(), flippedname.end());

    if (strstr(flippedname.c_str(), str_check.c_str())) {
        return true;
    } else
        return false;
}

bool IsCurrentAnimExactString(std::string anim_name, std::string str_check) {
    std::reverse(anim_name.begin(), anim_name.end());
    auto result = anim_name.find("\\");
    auto flippedname = anim_name.substr(4, result - 4);
    std::reverse(flippedname.begin(), flippedname.end());

    if (flippedname.c_str() == str_check.c_str()) {
        return true;
    } else
        return false;
}


bool IsAnimPlaying(RE::TESObjectREFR* a_thisObj, std::string test) {

    auto ToClipGenerator = [](RE::hkbNode* a_node) -> RE::hkbClipGenerator* {
        if (a_node && a_node->GetClassType()) {
            if (_strcmpi(a_node->GetClassType()->name, "hkbClipGenerator") == 0)
                return skyrim_cast<RE::hkbClipGenerator*>(a_node);

            //if (_strcmpi(a_node->GetClassType()->name, "BSSynchronizedClipGenerator") == 0) {
            //    auto syncClip = skyrim_cast<RE::BSSynchronizedClipGenerator*>(a_node);
            //    if (syncClip) return syncClip->clipGenerator;
            //}
        }

        return nullptr;
    };

    if (a_thisObj) {
        RE::BSAnimationGraphManagerPtr graphMgr;
        if (a_thisObj->GetAnimationGraphManager(graphMgr) && graphMgr) {
            for (const auto project : graphMgr->graphs) {
                auto behaviourGraph = project ? project->behaviorGraph : nullptr;
                auto activeNodes = behaviourGraph ? behaviourGraph->activeNodes : nullptr;
                if (activeNodes) {
                    for (auto nodeInfo : *activeNodes) {
                        auto nodeClone = nodeInfo.nodeClone;
                        if (nodeClone && nodeClone->GetClassType()) {
                            auto clipGenrator = ToClipGenerator(nodeClone);
                            if (clipGenrator) {
                                if (strstr(clipGenrator->animationName.c_str(), test.c_str())) {
                                    return true;
                                }
                            }
                        }
                        }
                    }
                }
            }
        }
    return false;
}

uint32_t get_implicit_id_event(RE::hkbBehaviorGraph* graph, const char* name) {
    const auto& names = graph->data->stringData->eventNames;
    for (uint32_t i = 0; i < names.size(); i++) {
            if (!std::strcmp(names[i].c_str(), name)) return i;
    }
    return RE::hkbEventBase::SystemEventIDs::kNull;
}







float GetEffectiveCastSpeed(RE::ActorMagicCaster* a_AMC, RE::Actor* a,float perkfactor) {

        float workingperkfactor = perkfactor;
        auto source = a_AMC->GetCastingSource();
        RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "CastTime", 2, {a_AMC->currentSpell}); // Channel 2 will be the all purpose channel for everything

        if (source == RE::MagicSystem::CastingSource::kRightHand) {
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "CastTime", 3, {a_AMC->currentSpell}); // Channel 3 will be the Right Hand Channel
        } else if (source == RE::MagicSystem::CastingSource::kLeftHand) {
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "CastTime", 4, {a_AMC->currentSpell}); // Channel 4 will be the Left Hand Channel
        }

        return workingperkfactor;

}

float GetEffectiveConcentrationSpeed(RE::ActorMagicCaster* a_AMC, RE::Actor* a, float perkfactor) {
        float workingperkfactor = perkfactor;
        auto source = a_AMC->GetCastingSource();
        RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "ConcentrationTime", 2,
                             {a_AMC->currentSpell});  // Channel 2 will be the all purpose channel for everything

        if (source == RE::MagicSystem::CastingSource::kRightHand) {
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "ConcentrationTime", 3,
                                 {a_AMC->currentSpell});  // Channel 3 will be the Right Hand Channel
        } else if (source == RE::MagicSystem::CastingSource::kLeftHand) {
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "ConcentrationTime", 4,
                                 {a_AMC->currentSpell});  // Channel 4 will be the Left Hand Channel
        }

        return workingperkfactor;
}


float GetEffectiveDualCast(RE::ActorMagicCaster* a_AMC, RE::Actor* a, float perkfactor) {
        float workingperkfactor = perkfactor;
        RE::MagicItem* currentspell = a_AMC->currentSpell;
        auto source = a_AMC->GetCastingSource();
        if (source == RE::MagicSystem::CastingSource::kRightHand) {
            if (!a_AMC->currentSpell) {
                 currentspell = a->GetActorRuntimeData().selectedSpells[1];
            }
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "DualCast", 3,
                                 {currentspell});  // Channel 3 will be the Right Hand Channel
        } else if (source == RE::MagicSystem::CastingSource::kLeftHand) {
            if (!a_AMC->currentSpell) {
                 currentspell = a->GetActorRuntimeData().selectedSpells[0];
            }
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "DualCast", 4,
                                 {currentspell});  // Channel 4 will be the Left Hand Channel
        }
        RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "DualCast", 2,
            {currentspell});  // Channel 2 will be the all purpose channel for everything
        return workingperkfactor;
}

struct Hooks {
    struct Update {
            static void thunk(RE::ActorMagicCaster* a_AMC, float a_deltatime) {
                 float updatedtime = a_deltatime;
                 auto a = a_AMC->GetCasterAsActor();
                 auto source = a_AMC->GetCastingSource();
                 float castperkfactor = 1.00f;
                 float concentrationperkfactor = 1.00f;
                 int breakloop = 0;

                 if (source == RE::MagicSystem::CastingSource::kRightHand ||
                     source == RE::MagicSystem::CastingSource::kLeftHand) {
                    if (a_AMC->state.get() == RE::MagicCaster::State::kUnk02) {
                        castperkfactor = GetEffectiveCastSpeed(a_AMC, a, castperkfactor);
                        // perkfactor = std::min((a_AMC->currentSpell->GetChargeTime()) / a_deltatime, perkfactor);
                        castperkfactor = std::max(0.05f, castperkfactor);
                        updatedtime = a_deltatime * castperkfactor;

                    }

                    else if ((a_AMC->state.get() == RE::MagicCaster::State::kCasting) &&
                             (a_AMC->currentSpell->GetCastingType() == RE::MagicSystem::CastingType::kConcentration) &&
                             a == RE::PlayerCharacter::GetSingleton()) {
                        // if (a_AMC->currentSpell->ContainsKeywordString("MagicResourceStamina")) {
                        float magnitude = 0.0f;
                        for (auto& elements : a_AMC->currentSpell->effects) {
                        if (elements->baseEffect->ContainsKeywordString("MagicResourceStamina")) {
                            magnitude = elements->GetMagnitude();
                        }
                        }
                        if (magnitude > 0.0f) {
                            // float staminacost = 0.0f;
                            if (a_AMC->GetIsDualCasting()) {
                                magnitude = RE::MagicFormulas::CalcDualCastCost(magnitude);
                            }
                            RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &magnitude, "SpellCosts", 3,
                                                 {a_AMC->currentSpell});
                            if (magnitude * updatedtime <
                                a->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina)) {
                                a->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,
                                                                          RE::ActorValue::kStamina,
                                                                          -magnitude * updatedtime);
                            } else {
                                RE::DebugNotification("Not enough stamina to cast this spell", nullptr, true);

                                breakloop = 1;
                                // a_AMC->InterruptCast(false);
                            }
                        }
                        //}
                        // if (a_AMC->currentSpell->ContainsKeywordString("MagicResourceHealth")) {
                        magnitude = 0.0f;
                        for (auto& elements : a_AMC->currentSpell->effects) {
                        if (elements->baseEffect->ContainsKeywordString("MagicResourceHealth")) {
                            magnitude = elements->GetMagnitude();
                        }
                        }
                        // float healthcost = 0.0f;
                        if (magnitude > 0.0f) {
                            if (a_AMC->GetIsDualCasting()) {
                                magnitude = RE::MagicFormulas::CalcDualCastCost(magnitude);
                            }
                            RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &magnitude, "SpellCosts", 4,
                                                 {a_AMC->currentSpell});
                            if (magnitude * updatedtime <
                                a->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth)) {
                                a->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,
                                                                          RE::ActorValue::kHealth,
                                                                          -magnitude * updatedtime);
                            } else {
                                RE::DebugNotification("Not enough health to cast this spell", nullptr, true);

                                breakloop = 1;
                            }
                        }
                        // }

                        // if (a_AMC->currentSpell->ContainsKeywordString("MagicResourceStamina")) {
                        //             float magnitude = 0.0f;
                        //             for (auto& elements : a_AMC->currentSpell->effects) {
                        //                 if (elements->baseEffect->ContainsKeywordString("MagicResourceStamina")) {
                        //                     magnitude = elements->GetMagnitude();
                        //                 }
                        //             }
                        //             // float staminacost = 0.0f;
                        //             RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &magnitude,
                        //             "SpellCosts", 3,
                        //                                  {a_AMC->currentSpell});
                        //             a->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,
                        //                                                       RE::ActorValue::kStamina, -magnitude *
                        //                                                       updatedtime);
                        //         }
                        //         if (a_AMC->currentSpell->ContainsKeywordString("MagicResourceHealth")) {
                        //             float magnitude = 0.0f;
                        //             for (auto& elements : a_AMC->currentSpell->effects) {
                        //                 if (elements->baseEffect->ContainsKeywordString("MagicResourceHealth")) {
                        //                     magnitude = elements->GetMagnitude();
                        //                 }
                        //             }
                        //             // float healthcost = 0.0f;
                        //             RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &magnitude,
                        //             "SpellCosts", 4,
                        //                                  {a_AMC->currentSpell});
                        //             a->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,
                        //                                                       RE::ActorValue::kHealth, -magnitude *
                        //                                                       updatedtime);
                        //         }

                        // concentrationperkfactor = GetEffectiveConcentrationSpeed(a_AMC, a, concentrationperkfactor);
                        //// perkfactor = std::min((a_AMC->currentSpell->GetChargeTime()) / a_deltatime, perkfactor);
                        // concentrationperkfactor = std::max(0.05f, concentrationperkfactor);
                        // updatedtime = a_deltatime * concentrationperkfactor;
                    }
                 }
                 if (breakloop == 1) {
                    a_AMC->InterruptCast(false);
                 }
                 else {
                    func(a_AMC, updatedtime);
                 }
            }
        static inline REL::Relocation<decltype(thunk)> func;
    };


    struct DualCasting {
        static bool thunk(RE::ActorMagicCaster* a_AMC) {
            auto a = a_AMC->GetCasterAsActor();
            auto source = a_AMC->GetCastingSource();
            float dualcastfactor = 1.00f;
            if (source == RE::MagicSystem::CastingSource::kRightHand || source == RE::MagicSystem::CastingSource::kLeftHand) {
                //RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &dualcastfactor, "DualCast", 2,
                //                     {a_AMC->currentSpell});
                dualcastfactor = GetEffectiveDualCast(a_AMC, a, dualcastfactor);
                if (dualcastfactor >= 5) {
                    //a_AMC->flags.set(RE::ActorMagicCaster::Flags::kDualCasting);
                    return true;
                }
            }
            return func(a_AMC);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct StartChargingHook {
        static void thunk(RE::ActorMagicCaster* a_AMC) {
            auto a = a_AMC->GetCasterAsActor();
            auto source = a_AMC->GetCastingSource();
            //RE::ConsoleLog::GetSingleton()->Print("StartCharge!");
            std::vector<RE::SpellItem*> vecSpellsStart;
            RE::HandleEntryPoint(RE::PerkEntryPoint::kApplyReanimateSpell, a, &vecSpellsStart, "CastSpell", 13,
                                 {a_AMC->currentSpell, a});
            CastSpellsPerk(vecSpellsStart, a, a);
            func(a_AMC);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct FinishChargingHook {
        static void thunk(RE::ActorMagicCaster* a_AMC) {
            auto a = a_AMC->GetCasterAsActor();
            auto source = a_AMC->GetCastingSource();
           // RE::ConsoleLog::GetSingleton()->Print("FinishCharge!");
            func(a_AMC);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct StartCastingHook {
        static void thunk(RE::ActorMagicCaster* a_AMC) {
            auto a = a_AMC->GetCasterAsActor();
            auto source = a_AMC->GetCastingSource();
            int breakloop = 0;

            // RE::ConsoleLog::GetSingleton()->Print("StartCast!");
            if ((a_AMC->currentSpell->GetCastingType() != RE::MagicSystem::CastingType::kConcentration) &&
                a == RE::PlayerCharacter::GetSingleton()) {
                float magnitude = 0.0f;
                for (auto& elements : a_AMC->currentSpell->effects) {
                    if (elements->baseEffect->ContainsKeywordString("MagicResourceStamina")) {
                        magnitude = elements->GetMagnitude();
                    }
                }
                if (magnitude > 0.0f) {
                    if (a_AMC->GetIsDualCasting()) {
                        magnitude = RE::MagicFormulas::CalcDualCastCost(magnitude);
                    }
                    RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &magnitude, "SpellCosts", 3,
                                         {a_AMC->currentSpell});

                    if (magnitude < a->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina)) {
                        a->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,
                                                                  RE::ActorValue::kStamina, -magnitude);
                    } else {
                        RE::DebugNotification("Not enough stamina to cast this spell", nullptr, true);
                        breakloop = 1;
                    }
                }

                magnitude = 0.0f;
                for (auto& elements : a_AMC->currentSpell->effects) {
                    if (elements->baseEffect->ContainsKeywordString("MagicResourceHealth")) {
                        magnitude = elements->GetMagnitude();
                    }
                }
                if (a_AMC->GetIsDualCasting()) {
                    magnitude = RE::MagicFormulas::CalcDualCastCost(magnitude);
                }
                RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &magnitude, "SpellCosts", 4,
                                     {a_AMC->currentSpell});

                if (magnitude < a->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth)) {
                    a->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,
                                                              RE::ActorValue::kHealth, -magnitude);
                } else {
                    RE::DebugNotification("Not enough health to cast this spell", nullptr, true);
                    breakloop = 1;
                }
            }
            if (*Settings::CastingDrainsStamina == true) {
                if (a_AMC->currentSpell->GetCostliestEffectItem()->baseEffect->GetMinimumSkillLevel()) {
                float scalingcost = *Settings::MaxStaminaSpellCost;
                float basecost = *Settings::BaseStaminaSpellCost;
                float staminacost = (a_AMC->currentSpell->GetCostliestEffectItem()->baseEffect->GetMinimumSkillLevel() *
                                     scalingcost / 100.0f) +
                                    basecost;
                RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &staminacost, "CastSpell", 20,
                                     {a_AMC->currentSpell});
                if (staminacost < a->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina)) {
                    a->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,
                                                              RE::ActorValue::kStamina, -staminacost);
                } else {
                    a->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,
                                                              RE::ActorValue::kStamina, -(a->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina)));
                }
            
            }
            }

            std::vector<RE::SpellItem*> vecSpellsStartCast;
            RE::HandleEntryPoint(RE::PerkEntryPoint::kApplyReanimateSpell, a, &vecSpellsStartCast, "CastSpell", 15,
                                 {a_AMC->currentSpell, a});
            CastSpellsPerk(vecSpellsStartCast, a, a);
            if (breakloop == 1) {
                //RE::DebugNotification("TestString", nullptr, true);
                a_AMC->InterruptCast(true);
            } else {
                func(a_AMC);

            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct FinishCastingHook {
        static void thunk(RE::ActorMagicCaster* a_AMC) {
            auto a = a_AMC->GetCasterAsActor();
            auto source = a_AMC->GetCastingSource();

            std::vector<RE::SpellItem*> vecSpellsFinish;
            RE::HandleEntryPoint(RE::PerkEntryPoint::kApplyReanimateSpell, a, &vecSpellsFinish, "CastSpell", 14,
                                 {a_AMC->currentSpell, a});
            CastSpellsPerk(vecSpellsFinish, a, a);
            //RE::ConsoleLog::GetSingleton()->Print("FinishCast!");
            func(a_AMC);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct OverrideSpellHook {
        static void thunk(RE::ActorMagicCaster* a_AMC, RE::SpellItem* IntendedSpell) {
            auto a = a_AMC->GetCasterAsActor();
            auto source = a_AMC->GetCastingSource();
            //RE::ConsoleLog::GetSingleton()->Print("FinishCast!");
            std::vector<RE::SpellItem*> vecSpellsSpellHook;
            RE::HandleEntryPoint(RE::PerkEntryPoint::kApplyReanimateSpell, a, &vecSpellsSpellHook, "CastSpell", 16,
                                 {IntendedSpell, a});
            float adjustfactor = 1;
            RE::SpellItem* NewSpell = vecSpellsSpellHook[0];
            if (NewSpell) {
                RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &adjustfactor, "CastSpell", 17,
                                     {NewSpell});

                a_AMC->currentSpellCost = NewSpell->CalculateMagickaCost(a)*adjustfactor;

                func(a_AMC, NewSpell);
            } else {
                func(a_AMC, IntendedSpell);

            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
                            /// May have to manually change casting state for concentration spells? 
    struct OverrideSpellCostHook {
        static void thunk(RE::ActorMagicCaster* a_AMC) {
            auto a = a_AMC->GetCasterAsActor();
            auto source = a_AMC->GetCastingSource();
            //RE::ConsoleLog::GetSingleton()->Print("FinishCast!");
            func(a_AMC);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct ActorValueMaxHook {
        static float thunk(RE::ActorValueOwner* a_AVO, RE::ActorValue index) {
            float maxAV = 1.0f;
            if (index == RE::ActorValue::kHealth) {
                RE::Actor* a_actor = SKSE::stl::adjust_pointer<RE::Actor>(a_AVO, -0x0B8);
                maxAV = a_AVO->GetBaseActorValue(index) +
                        a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, index) +
                        a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, index);
                //char buffer[150];
                //sprintf_s(buffer, "avHealth: %f, %f, %f, %f", a_actor->GetActorRuntimeData().staminaModifiers.modifiers[0],
                //          a_AVO->GetBaseActorValue(index),
                //          a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, index),
                //          a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, index));
                //RE::ConsoleLog::GetSingleton()->Print(buffer);
            } else if (index == RE::ActorValue::kStamina) {
                RE::Actor* a_actor = SKSE::stl::adjust_pointer<RE::Actor>(a_AVO, -0x0B8);
                maxAV = a_AVO->GetBaseActorValue(index) +
                        a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, index) +
                        a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, index);
                //char buffer[150];
                //sprintf_s(buffer, "avStam: %f, %f, %f, %f", a_actor->GetActorRuntimeData().staminaModifiers.modifiers[0],
                //          a_AVO->GetBaseActorValue(index),
                //          a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, index),
                //          a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, index));
                //RE::ConsoleLog::GetSingleton()->Print(buffer);
                // char buffer[150];
                //sprintf_s(buffer, "av: %f, %f, %f, %f", a_actor->GetActorRuntimeData().staminaModifiers.modifiers[0],
                //          a_AVO->GetBaseActorValue(index),
                //     a_actor->GetActorRuntimeData().staminaModifiers.modifiers[RE::ACTOR_VALUE_MODIFIER::kPermanent],
                //     a_actor->GetActorRuntimeData().staminaModifiers.modifiers[RE::ACTOR_VALUE_MODIFIER::kDamage]);
                //RE::ConsoleLog::GetSingleton()->Print(buffer);
            } else if (index == RE::ActorValue::kMagicka) {
                RE::Actor* a_actor = SKSE::stl::adjust_pointer<RE::Actor>(a_AVO, -0x0B8);
                maxAV = a_AVO->GetBaseActorValue(index) +
                        a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, index) +
                        a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, index);
            }  
            return maxAV;
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };



    struct OnHitHook {
        static void thunk(RE::Actor* a_AMC, RE::HitData &hit) {
            //auto a = a_AMC->GetCasterAsActor();
            //auto source = a_AMC->GetCastingSource();

            func(a_AMC, hit);
            //if (hit.criticalDamageMult) {
            //    RE::ConsoleLog::GetSingleton()->Print("Hello, critski!");
            //}
            if (hit.flags.any(RE::HitData::Flag::kCritical) == 1) {
                CastSpellsWeapon(4, a_AMC, hit.weapon, hit.target.get().get());
                CastSpellsWeapon(5, a_AMC, hit.weapon, a_AMC);

                //RE::ConsoleLog::GetSingleton()->Print("Hello, critter!");
            } else if (hit.flags.any(RE::HitData::Flag::kSneakAttack) == 1) {
                CastSpellsWeapon(6, a_AMC, hit.weapon, hit.target.get().get());
                CastSpellsWeapon(7, a_AMC, hit.weapon, a_AMC);

               // RE::ConsoleLog::GetSingleton()->Print("Hello, scritter!");
            } else {
                CastSpellsWeapon(8, a_AMC, hit.weapon, a_AMC);
                 //RE::ConsoleLog::GetSingleton()->Print("Hello, scritter!");

            }

        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct StartHitting {
        static void thunk(RE::Actor* a_AI) {
            // auto a = a_AMC->GetCasterAsActor();
            // auto source = a_AMC->GetCastingSource();
            //if (a_AI->GetActorRuntimeData().currentProcess) 
            //   if (a_AI->GetActorRuntimeData().currentProcess->high)
            if (a_AI->GetActorRuntimeData().currentProcess->high->attackData.get()) {
                if (a_AI->GetAttackingWeapon()) {
                
                    if (RE::TESBoundObject* objectweapon = a_AI->GetAttackingWeapon()->object) {
                         if (RE::TESObjectWEAP* weap = objectweapon->As<RE::TESObjectWEAP>()) {
                            RE::AttackData A_Data =
                                a_AI->GetActorRuntimeData().currentProcess->high->attackData.get()->data;

                            if (A_Data.flags.any(RE::AttackData::AttackFlag::kPowerAttack) == 1 && a_AI->IsSneaking()) {
                                if (a_AI->GetActorRuntimeData().currentProcess->high->attackData.get()->data.pad34 !=
                                    77) {
                                // RE::ConsoleLog::GetSingleton()->Print("Hello, power!");
                                a_AI->GetActorRuntimeData().currentProcess->high->attackData.get()->data.pad34 = 77;
                                CastSpellsWeapon(9, a_AI, weap, a_AI);
                                } else {
                                a_AI->GetActorRuntimeData().currentProcess->high->attackData.get()->data.pad34 = 74;
                                }
                            } else if (A_Data.flags.any(RE::AttackData::AttackFlag::kPowerAttack) == 1 &&
                                       !a_AI->IsSneaking()) {
                                // RE::ConsoleLog::GetSingleton()->Print("Hello, normalcy!");
                                CastSpellsWeapon(10, a_AI, weap, a_AI);
                            } else {
                                CastSpellsWeapon(11, a_AI, weap, a_AI);
                            }
                    }
                }
            }
            }
            func(a_AI);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    //struct TimeMult {
    //    static void thunk(RE::Character* a_Killed, float timestep) {
    //        // RE::ConsoleLog::GetSingleton()->Print(a_Killed->GetUserData()->GetName());
    //        
    //        if ((rand() % (90 - 47) + 47) == 49) {
    //            char buffer[150];
    //            sprintf_s(buffer, "angles: %f ", a_Killed->GetActorRuntimeData().lastUpdate * 100);
    //            RE::ConsoleLog::GetSingleton()->Print(buffer);
    //            RE::ConsoleLog::GetSingleton()->Print(a_Killed->GetName());
    //        }
    //        //a_Killed->GetActorRuntimeData().lastUpdate *= 10;
    //        // RE::ConsoleLog::GetSingleton()->Print("Hello, playerski!");
    //        timestep = 1.0f;
    //        func(a_Killed, timestep);
    //    }
    //    static inline REL::Relocation<decltype(thunk)> func;
    //};   

    struct HealthRegenHook {
        static void thunk(RE::Character* a_actor, float timestepHealth) {
            //RE::ConsoleLog::GetSingleton()->Print("Hello, playerski!");
            // 
            //float timescalered = 1.0f;
            //if (a_actor == RE::PlayerCharacter::GetSingleton()) {
            //if (a_actor->AsActorValueOwner()) {
            //if (((rand() % (90 - 47) + 47) == 49)) {
            //RE::ConsoleLog::GetSingleton()->Print("Hello, playerski!");

            //RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timestepHealth, "TimeScale", 6,
            //                     {});
                    //}
            //}
            //}
            //auto testvalue = Serialization::GetSingleton()->GetGlobalValue(1);
            //float timescalered=
            //    a_actor->AsActorValueOwner()->GetActorValue(testvalue);
             //float timescaler =
             //   a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLeftMobilityCondition) / 100.0f;
            float timescaler = 1.0f;
            if (*Settings::UseVanillaAV == true) {
            timescaler = a_actor->AsActorValueOwner()->GetActorValue((LookupActorValueByName("LeftMobilityCondition"))) / 100.0f;
            }
                RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timescaler, "TimeScale", 6,
                                 {});
            //timestepHealth *= timescalered;
            //float test = 1.0f;
            //test += timestepHealth;

            func(a_actor, timestepHealth*timescaler);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };   

    struct MagickaRegenHook {
        static void thunk(RE::Character* a_actor, float timestepMagicka) {
            float timescaler = 1.0f;
            if (*Settings::UseVanillaAV == true) {
            timescaler =
                a_actor->AsActorValueOwner()->GetActorValue((LookupActorValueByName("LeftMobilityCondition"))) / 100.0f;
            }
            //float timescaler =
            //    a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLeftMobilityCondition) / 100.0f;
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timestepMagicka, "TimeScale", 6, {});
            timestepMagicka *= timescaler;

            func(a_actor, timestepMagicka);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };  

    struct StaminaRegenHook {
        static void thunk(RE::Character* a_actor, float timestepStamina) {
            float timescaler = 1.0f;
            if (*Settings::UseVanillaAV == true) {
            timescaler =
                a_actor->AsActorValueOwner()->GetActorValue((LookupActorValueByName("LeftMobilityCondition"))) / 100.0f;
            }
                //        float timescaler =
                //a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLeftMobilityCondition) / 100.0f;
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timestepStamina, "TimeScale", 6, {});
            timestepStamina *= timescaler;

            func(a_actor, timestepStamina);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };  

    struct VoiceRecoveryHook {
        static void thunk(RE::Actor* a_actor, float timestepVoice) {
            float timescaler = 1.0f;
            if (*Settings::UseVanillaAV == true) {
            timescaler =
                a_actor->AsActorValueOwner()->GetActorValue((LookupActorValueByName("LeftMobilityCondition"))) / 100.0f;
            }
                //        float timescaler =
                //a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLeftMobilityCondition) / 100.0f;
            //RE::ConsoleLog::GetSingleton()->Print("Hello, playerski!");
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timestepVoice, "TimeScale", 6, {});
            timestepVoice *= timescaler;

            func(a_actor, timestepVoice);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };  

    struct ShoutingTimeHook {
        static void thunk(RE::Character* a_actor, float timestepShout) {
            float timescaler = 1.0f;
            if (*Settings::UseVanillaAV == true) {
            timescaler =
                a_actor->AsActorValueOwner()->GetActorValue((LookupActorValueByName("LeftMobilityCondition"))) / 100.0f;
            }
            //float timescaler =
            //    a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLeftMobilityCondition) / 100.0f;
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timestepShout, "TimeScale", 6, {});
            timestepShout *= timescaler;

            func(a_actor, timestepShout);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };  

    struct CombatTimeHook {
        static void thunk(RE::Actor* a_actor, float timestepCombat) {
            //float timescale = 1.0f;
            float timescaler = 1.0f;
            if (*Settings::UseVanillaAV == true) {
            timescaler =
                a_actor->AsActorValueOwner()->GetActorValue((LookupActorValueByName("LeftMobilityCondition"))) / 100.0f;
            }
                //        float timescaler =
                //a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLeftMobilityCondition) / 100.0f;
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timestepCombat, "TimeScale", 6, {});
            timestepCombat *= timescaler;

            func(a_actor, timestepCombat);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };  

    struct MagicTargetUpdateHook {
        static void thunk(RE::MagicTarget* a_mt, float timestepMagicTarget) {
            auto a_actor = GetActorFromMagicTarget(a_mt);
            if (a_actor) {
                //float timescaler =
                //    a_actor->AsActorValueOwner()->GetActorValue((LookupActorValueByName("TimeScaleMagicTarget"))) / 100.0f;
            float timescaler = 1.0f;
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timestepMagicTarget, "TimeScale", 7,
                                 {});
            // RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timestepMagicTarget, "TimeScale",
            // 7,
            //                      {});
            timestepMagicTarget *= timescaler;
        }
            func(a_mt, timestepMagicTarget);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct StartDying {
        static bool thunk(RE::Character* a_Killed, RE::Character* a_Killer) {
            CastSpellsTarget(3, a_Killer, a_Killed);
            return func(a_Killed, a_Killer);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    //std::vector<RE::SpellItem*> reanimateSpells;

    //RE::HandleEntryPoint(RE::PerkEntryPoint::kApplyReanimateSpell, summoner, &reanimateSpells, "SummonSpell", 3,
    //                     {akCastedMagic, summonedactor});

    struct AnimationSpeed {
        static void thunk(RE::hkbClipGenerator* a_Killed, const RE::hkbContext& a_context, float a_timesteps) {
            if ((*Settings::CreatureWeaponSpeed)) {
            if (a_context.character) {
                    auto a_graph = GetActorFromHkbCharacter(a_context.character);
                    auto a_actor = a_graph->holder;
                    bool attacking = false;
                    std::string_view human = "ActorTypeNPC";
                    int hitframes = 0;
                    //if (a_actor && a_actor == RE::PlayerCharacter::GetSingleton()) {
                    //    // if (IsCurrentAnimSubString(a_Killed->animationName.c_str(), "Whirlwind")) {

                    //    //    a_Killed->playbackSpeed *= 1.5;
                    //    //}
                    //}
                    if (a_actor) {
                        if (!strstr(a_graph->projectName.c_str(), "DefaultMale") &&
                            !strstr(a_graph->projectName.c_str(), "DefaultFemale") &&
                            !strstr(a_graph->projectName.c_str(), "FirstPerson")) {
                            auto id = get_implicit_id_event(a_context.behavior, "HitFrame");
                            if (a_Killed->triggers.get()) {
                                for (uint32_t i = 0; i < a_Killed->triggers.get()->triggers.size(); i++) {
                                    if (a_Killed->triggers.get()->triggers[i].event.id == id) {
                                    ++hitframes;
                                    }
                                }
                                if (hitframes > 0) {
                                    auto weaponspeedmult =
                                        a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kWeaponSpeedMult);

                                    RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor,
                                                         &weaponspeedmult, "TimeScale", 3, {});
                                    if (weaponspeedmult == 0.0f) {
                                    if (*Settings::CreatureWeaponSpeedBaseValueZero == true) {
                                        a_Killed->playbackSpeed = 1.0f;
                                        
                                    }
                                    if (*Settings::CreatureWeaponSpeedDefaultMessage == true) {
                                        RE::DebugNotification("Creature Weaponspeedmult detected as 0, consider changing aTweaks toml settings", nullptr, true);
                                    }                                   
                                    } else {
                                        a_Killed->playbackSpeed = weaponspeedmult;
                                    
                                    }
                                }
                            }
                        } 
                        else {
                            auto id = get_implicit_id_event(a_context.behavior, "HitFrame");
                            if (a_Killed->triggers.get()) {
                                for (uint32_t i = 0; i < a_Killed->triggers.get()->triggers.size(); i++) {
                                    if (a_Killed->triggers.get()->triggers[i].event.id == id) {
                                    ++hitframes;
                                    }
                                }
                                if (hitframes > 1) {
                                    a_Killed->playbackSpeed *= 0.33f + float(1/(hitframes+1));
                                }
                            }
                        }
                    }
            }
        }
            return func(a_Killed, a_context, a_timesteps);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
 

     struct ModifyAnimationData {
        static void thunk(RE::Character* a_thisPlayer, RE::BSAnimationUpdateData& a_dataPlayer) {
            float AnimationScale = 1.0f;
            //float timescaler = a_this->AsActorValueOwner()->GetActorValue(RE::ActorValue::kRightAttackCondition)/100.0f;
            float AnimationScaler = 1.0f;
            if (*Settings::UseVanillaAV == true) {
            AnimationScale =
                a_thisPlayer->AsActorValueOwner()->GetActorValue((LookupActorValueByName("LeftMobilityCondition"))) /
                100.0f;
            AnimationScale *=
                a_thisPlayer->AsActorValueOwner()->GetActorValue((LookupActorValueByName("RightMobilityCondition"))) /
                100.0f;

            }
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_thisPlayer, &AnimationScaler, "TimeScale", 2,
                                 {});
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_thisPlayer, &AnimationScaler, "TimeScale", 6,
                                 {});

            a_dataPlayer.deltaTime *= (AnimationScaler * AnimationScale);
            //a_data.deltaTime *= timescaler;
            func(a_thisPlayer, a_dataPlayer);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
 
          struct ModifyAnimationDataNPC {
        static void thunk(RE::Character* a_thisNPC, RE::BSAnimationUpdateData& a_dataNPC) {
            float AnimationScale = 1.0f;
            // float timescaler =
            // a_this->AsActorValueOwner()->GetActorValue(RE::ActorValue::kRightAttackCondition)/100.0f;
            float AnimationScaler = 1.0f;
            if (*Settings::UseVanillaAV == true) {
            AnimationScale =
                a_thisNPC->AsActorValueOwner()->GetActorValue((LookupActorValueByName("LeftMobilityCondition"))) /
                100.0f;
            AnimationScale *=
                a_thisNPC->AsActorValueOwner()->GetActorValue((LookupActorValueByName("RightMobilityCondition"))) /
                100.0f;
            }
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_thisNPC, &AnimationScaler, "TimeScale", 2,
                                 {});
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_thisNPC, &AnimationScaler, "TimeScale", 6,
                                 {});

            a_dataNPC.deltaTime *= (AnimationScaler * AnimationScale);
            // a_data.deltaTime *= timescaler;
            func(a_thisNPC, a_dataNPC);
        }
        static inline REL::Relocation<decltype(thunk)> func;
          };


    struct ModifyMovement {
        static void thunk(RE::Actor* a_this, float a_deltaTime) {
            float timescale = 1.0f;
            //float timescaler =
            //    a_this->AsActorValueOwner()->GetActorValue(RE::ActorValue::kRightAttackCondition) / 100.0f;
            float timescaler = 1.0f;
            if (*Settings::UseVanillaAV == true) {
            timescaler =
                a_this->AsActorValueOwner()->GetActorValue((LookupActorValueByName("LeftMobilityCondition"))) / 100.0f;
            timescaler *=
                a_this->AsActorValueOwner()->GetActorValue((LookupActorValueByName("RightMobilityCondition"))) / 100.0f;
            }
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_this, &timescale, "TimeScale", 2, {});
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_this, &timescale, "TimeScale", 6, {});

            func(a_this, a_deltaTime * (timescaler*timescale));
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct ProjectileUpdateHook {
        static void thunk(RE::Projectile* a_proj, float a_deltTime) {
            // float timescale = 1.0f;
            float timescalered = 1.0f;
            if (a_proj) {
            if (a_proj->GetActorCause()) {
                    if (a_proj->GetActorCause()->actor) {
                        if (a_proj->GetActorCause()->actor.get()) {
            auto a_actor = a_proj->GetActorCause()->actor.get().get();

            if (a_actor == RE::PlayerCharacter::GetSingleton()) {
                                if (*Settings::UseVanillaAV == true) {
                                    timescalered = a_actor->AsActorValueOwner()->GetActorValue(
                                                     (LookupActorValueByName("LeftAttackCondition"))) /
                                                 100.0f;
                                }
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timescalered, "TimeScale", 5, {});
            } else if (a_actor) {
            if (*Settings::UseVanillaAV == true) {
                                    timescalered = a_actor->AsActorValueOwner()->GetActorValue(
                                                       (LookupActorValueByName("LeftAttackCondition"))) /
                                                   100.0f;
            }
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timescalered, "TimeScale", 4, {});
            //             if (a_proje->GetPosition().GetDistance(RE::PlayerCharacter::GetSingleton()->GetPosition()) <
            // (*Settings::ProjectileSlowZone)) {
            //                 RE::ConsoleLog::GetSingleton()->Print("Hello, weakerestboys!");

            //}
            }
                        }
                    }
            }
            }
            func(a_proj, (a_deltTime)*timescalered);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct ProjectileUpdateHook2 {
        static void thunk(RE::Projectile* a_proje, float a_deltTimes) {
            // float timescale = 1.0f;
            float timescalered = 1.0f;
            if (a_proje) {
            if (a_proje->GetActorCause()) {
                    if (a_proje->GetActorCause()->actor) {
                        if (a_proje->GetActorCause()->actor.get()) {

            auto a_actor = a_proje->GetActorCause()->actor.get().get();

            if (a_actor==RE::PlayerCharacter::GetSingleton()) {
            if (*Settings::UseVanillaAV == true) {
                                    timescalered = a_actor->AsActorValueOwner()->GetActorValue(
                                                       (LookupActorValueByName("LeftAttackCondition"))) /
                                                   100.0f;
            }
              RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timescalered, "TimeScale", 5,
              {});
            } else if (a_actor) {
              if (*Settings::UseVanillaAV == true) {
                                    timescalered = a_actor->AsActorValueOwner()->GetActorValue(
                                                       (LookupActorValueByName("LeftAttackCondition"))) /
                                                   100.0f;
              }
              RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timescalered, "TimeScale", 4,
               {}); 
              //             if (a_proje->GetPosition().GetDistance(RE::PlayerCharacter::GetSingleton()->GetPosition()) <
              // (*Settings::ProjectileSlowZone)) {
              //                 RE::ConsoleLog::GetSingleton()->Print("Hello, weakerestboys!");

              //}
            }
                        }
                    }
            }
            }
            func(a_proje, (a_deltTimes)*timescalered);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct ProjectileUpdateHook3 {
        static void thunk(RE::Projectile* a_proj, float a_deltTime) {
            // float timescale = 1.0f;
            float timescalered = 1.0f;
            if (a_proj) {
            if (a_proj->GetActorCause()) {
                    if (a_proj->GetActorCause()->actor) {
                        if (a_proj->GetActorCause()->actor.get()) {
            auto a_actor = a_proj->GetActorCause()->actor.get().get();

            if (a_actor == RE::PlayerCharacter::GetSingleton()) {
              if (*Settings::UseVanillaAV == true) {
                                    timescalered = a_actor->AsActorValueOwner()->GetActorValue(
                                                       (LookupActorValueByName("LeftAttackCondition"))) /
                                                   100.0f;
              }
              RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timescalered, "TimeScale", 5, {});
            } else if (a_actor) {
              if (*Settings::UseVanillaAV == true) {
                                    timescalered = a_actor->AsActorValueOwner()->GetActorValue(
                                                       (LookupActorValueByName("LeftAttackCondition"))) /
                                                   100.0f;
              }
              RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timescalered, "TimeScale", 4, {});
              //             if (a_proje->GetPosition().GetDistance(RE::PlayerCharacter::GetSingleton()->GetPosition())
              //             <
              // (*Settings::ProjectileSlowZone)) {
              //                 RE::ConsoleLog::GetSingleton()->Print("Hello, weakerestboys!");

              //}
            }
                        }
                    }
            }
            }
            func(a_proj, (a_deltTime)*timescalered);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct ProjectileUpdateHook4 {
        static void thunk(RE::Projectile* a_proje, float a_deltTimes) {
            // float timescale = 1.0f;
            float timescalered = 1.0f;
            if (a_proje) {
            if (a_proje->GetActorCause()) {
                    if (a_proje->GetActorCause()->actor) {
                        if (a_proje->GetActorCause()->actor.get()) {
            auto a_actor = a_proje->GetActorCause()->actor.get().get();

            if (a_actor == RE::PlayerCharacter::GetSingleton()) {
              if (*Settings::UseVanillaAV == true) {
                                    timescalered = a_actor->AsActorValueOwner()->GetActorValue(
                                                       (LookupActorValueByName("LeftAttackCondition"))) /
                                                   100.0f;
              }
              RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timescalered, "TimeScale", 5, {});
            } else if (a_actor) {
              if (*Settings::UseVanillaAV == true) {
                                    timescalered = a_actor->AsActorValueOwner()->GetActorValue(
                                                       (LookupActorValueByName("LeftAttackCondition"))) /
                                                   100.0f;
              }
              RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timescalered, "TimeScale", 4, {});
              //             if (a_proje->GetPosition().GetDistance(RE::PlayerCharacter::GetSingleton()->GetPosition())
              //             <
              // (*Settings::ProjectileSlowZone)) {
              //                 RE::ConsoleLog::GetSingleton()->Print("Hello, weakerestboys!");

              //}
            }
        }
    }
}
}
            func(a_proje, (a_deltTimes)*timescalered);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct ProjectileUpdateHook5 {
        static void thunk(RE::Projectile* a_proje, float a_deltTimes) {
            // float timescale = 1.0f;
            float timescalered = 1.0f;
            if (a_proje) {
if (a_proje->GetActorCause()) {
    if (a_proje->GetActorCause()->actor) {
        if (a_proje->GetActorCause()->actor.get()) {
            auto a_actor = a_proje->GetActorCause()->actor.get().get();

            if (a_actor == RE::PlayerCharacter::GetSingleton()) {
              if (*Settings::UseVanillaAV == true) {
                                    timescalered = a_actor->AsActorValueOwner()->GetActorValue(
                                                       (LookupActorValueByName("LeftAttackCondition"))) /
                                                   100.0f;
              }
              RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timescalered, "TimeScale", 5, {});
            } else if (a_actor) {
              if (*Settings::UseVanillaAV == true) {
                                    timescalered = a_actor->AsActorValueOwner()->GetActorValue(
                                                       (LookupActorValueByName("LeftAttackCondition"))) /
                                                   100.0f;
              }
              RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timescalered, "TimeScale", 4, {});
              //             if (a_proje->GetPosition().GetDistance(RE::PlayerCharacter::GetSingleton()->GetPosition())
              //             <
              // (*Settings::ProjectileSlowZone)) {
              //                 RE::ConsoleLog::GetSingleton()->Print("Hello, weakerestboys!");

              //}
            }
    }
}
            }
        }
            func(a_proje, (a_deltTimes)*timescalered);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct ModifyMovementActor {
        static RE::bhkCharacterController* thunk(RE::Actor* a_thismmpc, float a_arg2, const RE::NiPoint3& a_positionMMPC) {
            //float MovementAngle = a_this->AsActorValueOwner()->GetActorValue(LookupActorValueByName("RightAttackCondition"));
            float MovementAngle = Serialization::GetMovementAngle();

            auto Delta = a_positionMMPC;
            // if (*Settings::WhirlwindSprint) {
            //     RE::ConsoleLog::GetSingleton()->Print("buffer");
            // }
            


            if (*Settings::WhirlwindSprint == true && a_thismmpc == RE::PlayerCharacter::GetSingleton()) {
                //bool IsDashing = IsAnimPlaying(a_this, std::string("ForwardRoll"));
            bool IsDashing = IsAnimPlaying(a_thismmpc, std::string("Whirlwind"));

                Delta.z = 0.0f;
                float distance = Delta.Length();
                RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_thismmpc, &distance, "WhirlwindDash", 2,
                                     {});
                //distance *=
                //    a_this->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLeftAttackCondition) / 100.0f;
                double angleOfmovement = std::atan2(Delta.y, Delta.x) * 180 / (atan(1) * 4);
                if (IsDashing) {
                                double newangle = double(MovementAngle);
                                float newY = std::sin(float(newangle) * atan(1) * 4 / 180) * distance;
                                float newX = std::cos(float(newangle) * atan(1) * 4 / 180) * distance;
                                 //char buffer[150];
                                 //sprintf_s(buffer, "angles: %f, %f, %f, %f, %f", Delta.y, Delta.x, newangle, newY,
                                 //newX); RE::ConsoleLog::GetSingleton()->Print(buffer);
                                Delta.x = newX;
                                Delta.y = newY;
                } else if (distance > 0.5f) {
                                Serialization::SetMovementAngle(angleOfmovement);
                                //a_this->AsActorValueOwner()->SetActorValue((LookupActorValueByName("RightAttackCondition")),
                                //                                           (angleOfmovement));

                                // char buffer[150];

                                // sprintf_s(buffer, "angle2s: %f, %f, %f, %f", Delta.y, Delta.x, angleOfmovement,
                                //           a_this->AsActorValueOwner()->GetActorValue(LookupActorValueByName("MovementAngle")));
                                // RE::ConsoleLog::GetSingleton()->Print(buffer);
                }
                Delta.z = a_positionMMPC.z;
            }
            if (*Settings::MovementSpeedFix == true &&
                a_thismmpc->AsActorValueOwner()->GetActorValue(RE::ActorValue::kSpeedMult) < 1.0f) {
                Delta.x *= 0.01f;
                Delta.y *= 0.01f;

            }
            return func(a_thismmpc, a_arg2, Delta);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

        struct ModifyMovementActorNPC {
        static RE::bhkCharacterController* thunk(RE::Actor* a_NPCmma, float a_arg2a, const RE::NiPoint3& a_positionMMA) {
            auto Delta = a_positionMMA;
            if (*Settings::MovementSpeedFix == true &&
                a_NPCmma->AsActorValueOwner()->GetActorValue(RE::ActorValue::kSpeedMult) < 1.0f) {
                Delta.x *= 0.01f;
                Delta.y *= 0.01f;
            }
            return func(a_NPCmma, a_arg2a, Delta);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };



    struct PlayerCharacter_Update {
        static void thunk(RE::PlayerCharacter* a_player, float a_delta) {
            float healthregen = 0.0f;
            float staminaregen = 0.0f;
            float magickaregen = 0.0f;
            float currenttimescale = 1.0f;
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_player, &healthregen, "TimeScale", 8, {});
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_player, &staminaregen, "TimeScale", 9, {});
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_player, &magickaregen, "TimeScale", 10, {});
            if (healthregen > 0.0f) {
                if (*Settings::UseVanillaAV == true) {
                                currenttimescale = a_player->AsActorValueOwner()->GetActorValue(
                                                 (LookupActorValueByName("LeftMobilityCondition"))) /
                                             100.0f;
                }
                //float currenttimescale =
                //    a_player->AsActorValueOwner()->GetActorValue((RE::ActorValue::kLeftMobilityCondition)) / 100.0f;

                RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_player, &currenttimescale, "TimeScale", 6, {});
                a_player->AsActorValueOwner()->RestoreActorValue(
                    RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth,
                    healthregen * a_delta * currenttimescale);
            }
            if (staminaregen > 0.0f) {
                if (*Settings::UseVanillaAV == true) {
                                currenttimescale = a_player->AsActorValueOwner()->GetActorValue(
                                                       (LookupActorValueByName("LeftMobilityCondition"))) /
                                                   100.0f;
                }
                //float currenttimescale =
                    //a_player->AsActorValueOwner()->GetActorValue((RE::ActorValue::kLeftMobilityCondition)) / 100.0f;
                RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_player, &currenttimescale, "TimeScale",
                                     6, {});
                a_player->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,
                                                                 RE::ActorValue::kStamina,
                                                                 staminaregen * a_delta * currenttimescale);
            }
            if (magickaregen > 0.0f) {
                if (*Settings::UseVanillaAV == true) {
                                currenttimescale = a_player->AsActorValueOwner()->GetActorValue(
                                                       (LookupActorValueByName("LeftMobilityCondition"))) /
                                                   100.0f;
                }
                //float currenttimescale =
                    //a_player->AsActorValueOwner()->GetActorValue((RE::ActorValue::kLeftMobilityCondition)) / 100.0f;
                RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_player, &currenttimescale, "TimeScale",
                                     6, {});
                a_player->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,
                                                                 RE::ActorValue::kMagicka,
                                                                 magickaregen * a_delta * currenttimescale);
            }
            /// add manual regen perk entry points. 

            func(a_player, a_delta);

        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct Character_Update {
        static void thunk(RE::Character* a_char, float a_delta)
        { 
//            if (a_char && *Settings::VariableNPC==true) {
//                if (a_char->AsActorValueOwner()) {
//                                if (a_char->AsActorValueOwner()->GetActorValue((LookupActorValueByName("npcAV"))) ==
//                                    100.0f) {
//                        std::random_device rd;
//
//                        std::mt19937 e2(rd());
//                        std::uniform_int_distribution<> dist(0, 100);
//                        float diceroll1 = float(dist(e2))/100.0f;
//                        float diceroll2 = float(dist(e2))/100.0f;
//                        
//
//                        a_char->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIERS::kPermanent, RE::ActorValue::kSpeedMult,
//                                              diceroll1 * (*Settings::VariableNPCmovementspeed));
//
//                        a_char->AsActorValueOwner()->RestoreActorValue(
//                            RE::ACTOR_VALUE_MODIFIERS::kPermanent, RE::ActorValue::kWeaponSpeedMult,
//                            diceroll2 * (*Settings::VariableNPCweaponspeed) / 100.0f);
//                        a_char->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIERS::kPermanent, (LookupActorValueByName("npcAV")), 10.0f);
//                        // 
///*                        char buffer[150];
//
//                        sprintf_s(buffer, "angle2s: %f %f %f",
//                            a_char->AsActorValueOwner()->GetActorValue((LookupActorValueByName("npcAV"))),
//                                  diceroll1 * (*Settings::VariableNPCmovementspeed),
//                                  diceroll2 * (*Settings::VariableNPCweaponspeed) / 100.0f);
//                        RE::ConsoleLog::GetSingleton()->Print(buffer);                  */       
//                        //a_char->AsActorValueOwner()->ModActorValue(RE::ActorValue::kSpeedMult, 1.0f);
//                        //a_char->AsActorValueOwner()->ModActorValue(RE::ActorValue::kWeaponSpeedMult, 1.0f);
//                        ////a_char->AsActorValueOwner()->ModActorValue((LookupActorValueByName("DummyAV")), 1.0f);
//                                }
//                }
//            }







                    float healthregen = 0.0f;
            float staminaregen = 0.0f;
            float magickaregen = 0.0f;
            float currenttimescale = 1.0f;
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_char, &healthregen, "TimeScale", 8, {});
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_char, &staminaregen, "TimeScale", 9, {});
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_char, &magickaregen, "TimeScale", 10, {});
            if (healthregen > 0.0f) {
                if (*Settings::UseVanillaAV == true) {
                                currenttimescale = a_char->AsActorValueOwner()->GetActorValue(
                                                       (LookupActorValueByName("LeftMobilityCondition"))) /
                                                   100.0f;
                }
                //float currenttimescale =
                    //a_char->AsActorValueOwner()->GetActorValue((RE::ActorValue::kLeftMobilityCondition)) / 100.0f;
                RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_char, &currenttimescale, "TimeScale",
                                     6, {});
                a_char->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,
                                                                 RE::ActorValue::kHealth,
                                                                 healthregen * a_delta * currenttimescale);
            }
            if (staminaregen > 0.0f) {
                if (*Settings::UseVanillaAV == true) {
                                currenttimescale = a_char->AsActorValueOwner()->GetActorValue(
                                                       (LookupActorValueByName("LeftMobilityCondition"))) /
                                                   100.0f;
                }
                //float currenttimescale =
                    //a_char->AsActorValueOwner()->GetActorValue((RE::ActorValue::kLeftMobilityCondition)) / 100.0f;
                RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_char, &currenttimescale, "TimeScale",
                                     6, {});
                a_char->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,
                                                                 RE::ActorValue::kStamina,
                                                                 staminaregen * a_delta * currenttimescale);
            }
            if (magickaregen > 0.0f) {
                if (*Settings::UseVanillaAV == true) {
                                currenttimescale = a_char->AsActorValueOwner()->GetActorValue(
                                                       (LookupActorValueByName("LeftMobilityCondition"))) /
                                                   100.0f;
                }
                //float currenttimescale =
                    //a_char->AsActorValueOwner()->GetActorValue((RE::ActorValue::kLeftMobilityCondition)) / 100.0f;
                RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_char, &currenttimescale, "TimeScale",
                                     6, {});
                a_char->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,
                                                                 RE::ActorValue::kMagicka,
                                                                 magickaregen * a_delta * currenttimescale);
            }

            /// add manual regen perk entry points. 
            func(a_char, a_delta);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };


        struct GetMaxActorValueAE {
            static auto Call(RE::ActorValueOwner* a_AVOm, RE::ActorValue indexs) -> float {
            float maxAV = (a_AVOm->GetPermanentActorValue(indexs));
            RE::Actor* a_actor = SKSE::stl::adjust_pointer<RE::Actor>(a_AVOm, -0x0B8);
            if (a_actor) {
                maxAV = a_AVOm->GetBaseActorValue(indexs) +
                        a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, indexs) +
                        a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, indexs);
            }
            return maxAV;
            }
        };

                struct GetMaxActorValueHMSE {
            static float Call(RE::ActorValueOwner* a_AVO, RE::ActorValue index) {
            float maxAV = (a_AVO->GetPermanentActorValue(index));
            RE::Actor* a_actor = SKSE::stl::adjust_pointer<RE::Actor>(a_AVO, -0x0B0);
            if (a_actor) {
                maxAV = a_AVO->GetBaseActorValue(index) +
                        a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, index) +
                        a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, index);
            }
            return maxAV;
            }
        };

                struct GetMaxActorValueStamSE {
            static float Call(RE::ActorValueOwner* a_AVOc) {
            float maxAV = (a_AVOc->GetPermanentActorValue(RE::ActorValue::kStamina));

                RE::Actor* a_actor = SKSE::stl::adjust_pointer<RE::Actor>(a_AVOc, -0x0B0);
                maxAV = a_AVOc->GetBaseActorValue(RE::ActorValue::kStamina) +
                        a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kStamina) +
                        a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, RE::ActorValue::kStamina);
            return maxAV;
            }
        };


                struct arrowImpactHook {
            static void thunk(RE::ArrowProjectile* a_AP, RE::TESObjectREFR* a_Target, void* a_arg3, float effective_toggle, float arg_unk) {

                auto* shooter = a_AP->GetProjectileRuntimeData().actorCause.get()->actor.get().get();
            if (shooter && *Settings::BowEnchantFix) {
                //a_AP->GetProjectileRuntimeData().power =
                //    shooter->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLeftMobilityCondition);

                 //char buffer[150];

                 //sprintf_s(buffer, "angle2s: %f", a_AP->GetProjectileRuntimeData().power);
                 //RE::ConsoleLog::GetSingleton()->Print(buffer);
                effective_toggle = -1.0f;

            }
            func(a_AP, a_Target, a_arg3, effective_toggle, arg_unk);
            }
            static inline REL::Relocation<decltype(thunk)> func;
};

        struct MeleeEnchantHook {
            static void thunk(RE::MagicCaster* a_MC, RE::MagicItem* a_MI, RE::Actor* a_target, void* boundobject,
                              bool arg_bool) {
            if (a_MC->GetCasterAsActor()) {
                if (a_MC->GetCasterAsActor()->GetActorRuntimeData().currentProcess) {
                                if (a_MC->GetCasterAsActor()
                                        ->GetActorRuntimeData()
                                        .currentProcess->high->attackData.get()) {
                        if ((a_MC->GetCasterAsActor()
                                ->GetActorRuntimeData()
                                .currentProcess->high->attackData.get()->data.flags.any(
             RE::AttackData::AttackFlag::kPowerAttack) == 1) &&
        *Settings::PowerAttackEnchant) {
                            float a_effectiveness = *Settings::PowerAttackEnchantBaseMag;
                            float extracost = 0.0f;
                            RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a_MC->GetCasterAsActor(),
                                                 &a_effectiveness, "Enchant",
                                                 4, {a_MI});
                            RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a_MC->GetCasterAsActor(),
                                                 &extracost, "Enchant", 5, {a_MI});

                            if (extracost > 0.0f && a_MC->GetCasterAsActor()->AsActorValueOwner()->GetActorValue(
                                                        RE::ActorValue::kMagicka) > extracost) {
                                a_MC->GetCasterAsActor()->AsActorValueOwner()->RestoreActorValue(
                                    RE::ACTOR_VALUE_MODIFIERS::kDamage, RE::ActorValue::kMagicka, -extracost);
                                a_MC->CastSpellImmediate(a_MI, 0, a_target, a_effectiveness, 0, 0.0f, 0i64);
                            } else if (extracost <= 0.0f) {
                                a_MC->CastSpellImmediate(a_MI, 0, a_target, a_effectiveness, 0, 0.0f, 0i64);
                            }
                            //RE::ConsoleLog::GetSingleton()->Print("success");

                        } else {
                            //RE::ConsoleLog::GetSingleton()->Print("fail");
                            func(a_MC, a_MI, a_target, boundobject, arg_bool);
                        }
                                }
                }
            }
            }
            static inline REL::Relocation<decltype(thunk)> func;
};

    static void Install() {
        stl::write_vfunc<RE::PlayerCharacter, 0xAD, PlayerCharacter_Update, 0>();
        stl::write_vfunc<RE::Character, 0xAD, Character_Update, 0>();

        stl::write_vfunc<RE::ActorMagicCaster, 0x1D, Update, 0>();
        stl::write_vfunc<RE::ActorMagicCaster, 0x16, DualCasting, 0>();
       // stl::write_vfunc<RE::PlayerCharacter, 0x02, ActorValueMaxHook, 5>();

       stl::write_vfunc<RE::ActorMagicCaster, 0x04, StartChargingHook, 0>();
       stl::write_vfunc<RE::ActorMagicCaster, 0x05, FinishChargingHook, 0>();
       stl::write_vfunc<RE::ActorMagicCaster, 0x06, StartCastingHook, 0>();
       stl::write_vfunc<RE::ActorMagicCaster, 0x07, FinishCastingHook, 0>();        
       // 
       // 
       //// 
        stl::write_vfunc<RE::PlayerCharacter, 0x0EF, StartHitting, 0>();
        stl::write_vfunc<RE::PlayerCharacter, 0x0C8, ModifyMovementActor, 0>();
        stl::write_vfunc<RE::Character, 0x0C8, ModifyMovementActorNPC, 0>();


       // //stl::write_vfunc<RE::PlayerCharacter, 0x1, StartApplying, 4>();
       // //stl::write_vfunc<RE::Character, 0x1, StartApplying, 4>();
       stl::write_vfunc<RE::PlayerCharacter, 0x79, ModifyAnimationData, 0>();
       stl::write_vfunc<RE::Character, 0x79, ModifyAnimationDataNPC, 0>();
       stl::write_vfunc<RE::hkbClipGenerator, 0x05, AnimationSpeed, 0>();

        // stl::write_vfunc<RE::Projectile, 0xAB, ProjectileUpdateHook>();
        stl::write_vfunc<RE::ConeProjectile, 0xAB, ProjectileUpdateHook3>();
        stl::write_vfunc<RE::MissileProjectile, 0xAB, ProjectileUpdateHook>();
        stl::write_vfunc<RE::ArrowProjectile, 0xAB, ProjectileUpdateHook2>();

        stl::write_vfunc<RE::BeamProjectile, 0xAB, ProjectileUpdateHook4>();
        stl::write_vfunc<RE::FlameProjectile, 0xAB, ProjectileUpdateHook5>();



        REL::Relocation<std::uintptr_t> HealthRegenHookAddress{RELOCATION_ID(37831, 38785),
                                                               REL::Relocate(0x183, 0x196)};
        stl::write_thunk_call<HealthRegenHook>(HealthRegenHookAddress.address());

        REL::Relocation<std::uintptr_t> StaminaRegenHookAddress{RELOCATION_ID(37831, 38785),
                                                                REL::Relocate(0x18E, 0x1A1)};
        stl::write_thunk_call<StaminaRegenHook>(StaminaRegenHookAddress.address());

        REL::Relocation<std::uintptr_t> MagickaRegenHookAddress{RELOCATION_ID(37831, 38785),
                                                                REL::Relocate(0x1A5, 0x208)};
        stl::write_thunk_call<MagickaRegenHook>(MagickaRegenHookAddress.address());

        REL::Relocation<std::uintptr_t> VoiceRecoveryHookAddress{RELOCATION_ID(37831, 38785),
                                                                 REL::Relocate(0x1C7, 0x229)};
        stl::write_thunk_call<VoiceRecoveryHook>(VoiceRecoveryHookAddress.address());

        REL::Relocation<std::uintptr_t> ShoutingTimeHookAddress{RELOCATION_ID(37831, 38785),
                                                                REL::Relocate(0x123, 0x12D)};
        stl::write_thunk_call<ShoutingTimeHook>(ShoutingTimeHookAddress.address());

        REL::Relocation<std::uintptr_t> CombatTimeHookAddress{RELOCATION_ID(37831, 38785), REL::Relocate(0xEA, 0xF4)};
        stl::write_thunk_call<CombatTimeHook>(CombatTimeHookAddress.address());

        //REL::Relocation<std::uintptr_t> MagicTargetUpdateHookAddress{RELOCATION_ID(37831, 38785),
        //                                                             REL::Relocate(0x15C, 0x168)};
        //stl::write_thunk_call<MagicTargetUpdateHook>(MagicTargetUpdateHookAddress.address());

        
        //if SKYRIM_REL_CONSTEXPR (REL::Module::IsAE()) {
        //    REL::Relocation<std::uintptr_t> DualCastSpellCheck{RELOCATION_ID(0, 38765), REL::Relocate(0x0, 0x8D)};
        //    REL::safe_fill(DualCastSpellCheck.address(), REL::NOP, 0x9);

        //} else {
        //    REL::Relocation<std::uintptr_t> DualCastSpellCheck{RELOCATION_ID(0, 0), REL::Relocate(0x0, 0x0)};
        //    REL::safe_fill(DualCastSpellCheck.address(), REL::NOP, 0x9);
        //}

        //REL::Relocation<std::uintptr_t> MagickaRegenHookAddress{RELOCATION_ID(37831, 0),
        //                                                        REL::Relocate(0x196, 0)};
        //stl::write_thunk_call<MagickaRegenHook>(MagickaRegenHookAddress.address());





        if SKYRIM_REL_CONSTEXPR (REL::Module::IsAE()) {
            REL::Relocation<std::uintptr_t> targetHMSAE{RELOCATION_ID(0, 38460), REL::Relocate(0x0, 0x1A1)};

            // lea rcx, [rdi+0xB8]
            stl::safe_write(targetHMSAE.address() + 0x00,
                            std::array<std::uint8_t, 7>{0x48, 0x8D, 0x8F, 0xB8, 0x00, 0x00, 0x00});

            // mov edx, r15d
            stl::safe_write(targetHMSAE.address() + 0x07, std::array<std::uint8_t, 3>{0x41, 0x8B, 0xD7});

            // (float)(RE::ActorValueOwner*, RE::ActorValue)
            stl::write_call<GetMaxActorValueAE>(targetHMSAE.address() + 0x0A);

            // NOP
            stl::safe_write(targetHMSAE.address() + 0x0F, std::array<std::uint8_t, 5>{0x90, 0x90, 0x90, 0x90, 0x90});
        } else {
            REL::Relocation<std::uintptr_t> targetHMSE{RELOCATION_ID(37515, 0), REL::Relocate(0x1A6, 0)};

            // mov edx, r15d
            stl::safe_write(targetHMSE.address() + 0x00, std::array<std::uint8_t, 3>{0x41, 0x8B, 0xD7});

            // mov rcx, [r14]
            stl::safe_write(targetHMSE.address() + 0x03, std::array<std::uint8_t, 3>{0x4C, 0x89, 0xF1});

            // (float)(RE::ActorValueOwner*, RE::ActorValue)
            stl::write_call<GetMaxActorValueHMSE>(targetHMSE.address() + 0x06);

            // NOP
            stl::safe_write(targetHMSE.address() + 0x0B, std::array<std::uint8_t, 1>{0x90});

            REL::Relocation<std::uintptr_t> targetStaminaSE{RELOCATION_ID(37510, 0), REL::Relocate(0xE6, 0)};

            // mov rcx, rdi
            stl::safe_write(targetStaminaSE.address() + 0x00, std::array<std::uint8_t, 3>{0x48, 0x89, 0xF9});

            // (float)(RE::ActorValueOwner*)
            stl::write_call<GetMaxActorValueStamSE>(targetStaminaSE.address() + 0x03);

            // NOP
            stl::safe_write(targetStaminaSE.address() + 0x08,
                            std::array<std::uint8_t, 6>{0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
        }

        REL::Relocation<std::uintptr_t> FunctionModifyMovement{RELOCATION_ID(36359, 37350), REL::Relocate(0xF0, 0xFB)};
        stl::write_thunk_call<ModifyMovement>(FunctionModifyMovement.address());

        REL::Relocation<std::uintptr_t> functionA{RELOCATION_ID(37673, 38627), REL::Relocate(0x3C0, 0x4A8)};
        stl::write_thunk_call<OnHitHook>(functionA.address());

        REL::Relocation<std::uintptr_t> functionC{RELOCATION_ID(36872, 37896), REL::Relocate(0x588, 0x5F8)};
        stl::write_thunk_call<StartDying>(functionC.address());

         REL::Relocation<std::uintptr_t> arrowImpact{RELOCATION_ID(42547, 43710), REL::Relocate(0x1CE, 0x241)};
        stl::write_thunk_call<arrowImpactHook>(arrowImpact.address());

         REL::Relocation<std::uintptr_t> meleeImpact{RELOCATION_ID(37799, 38748), REL::Relocate(0x138, 0x13B)};
        stl::write_thunk_call<MeleeEnchantHook>(meleeImpact.address());


    }
};

//void CastSpellMult(RE::StaticFunctionTag*, RE::Actor* akSource, RE::SpellItem* akSpell,
//                     RE::Actor* akTarget, float a_effectiveness) {
//    akSource->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
//        ->CastSpellImmediate(akSpell, false, akTarget, a_effectiveness, false, 0.0f, nullptr);
//}
//
//void CastEnchantmentMult(RE::StaticFunctionTag*, RE::Actor* akSource, RE::EnchantmentItem* akEnchantment,
//                         RE::Actor* akTarget, float a_effectiveness) {
//    akSource->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
//        ->CastSpellImmediate(akEnchantment, false, akTarget, a_effectiveness, false, 0.0f, nullptr);
//}
//
//void CastPotionMult(RE::StaticFunctionTag*, RE::Actor* akSource, RE::AlchemyItem* akPotion, RE::Actor* akTarget, float a_effectiveness) {
//    akSource->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
//        ->CastSpellImmediate(akPotion, false, akTarget, a_effectiveness, false, 0.0f, nullptr);
//}
//
//
//void CastIngredientMult(RE::StaticFunctionTag*, RE::Actor* akSource, RE::IngredientItem* akIngredient,
//                    RE::Actor* akTarget, float a_effectiveness) {
//    akSource->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
//        ->CastSpellImmediate(akIngredient, false, akTarget, a_effectiveness, false, 0.0f, nullptr);
//}

void CastSpellItemMult(RE::StaticFunctionTag*, RE::Actor* akSource, RE::SpellItem* akSpell,
                           RE::EnchantmentItem* akEnchantment,
                           RE::AlchemyItem* akPotion,
                           RE::IngredientItem* akIngredient,
                       RE::TESObjectREFR* akTarget, float a_effectiveness, float a_override) {
    RE::MagicItem* akMagic = nullptr;
    if (akSpell) {
       akMagic = akSpell;
    } else if (akEnchantment) {
       akMagic = akEnchantment;
    } else if (akPotion) {
       akMagic = akPotion;
    } else if (akIngredient) {
       akMagic = akIngredient;
    }
    auto akCastedMagic = akMagic;
    akSource->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
        ->CastSpellImmediate(akCastedMagic, false, akTarget, a_effectiveness, false, a_override, akSource);
}

 
void CastSpellMult(RE::StaticFunctionTag*, RE::Actor* akSource, RE::TESForm* MagicItem, RE::TESObjectREFR* akTarget,
                   float a_effectiveness, float a_override) {
    if (MagicItem) {
       RE::MagicItem* akCastedMagic = MagicItem->As<RE::MagicItem>();

       akSource->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
           ->CastSpellImmediate(akCastedMagic, false, akTarget, a_effectiveness, false, a_override, akSource);
    }
}


bool GetEffectWasDualCast(RE::StaticFunctionTag*, RE::ActiveEffect* a_activeEffect) {
    if (a_activeEffect) {
        if (a_activeEffect->flags.any(RE::ActiveEffect::Flag::kDual) == true) {
            return true;
        }
            return false;
    } else {
        return false;
    }
}

bool GetEnchantCostOverrideFlag(RE::StaticFunctionTag*, RE::EnchantmentItem* a_enchant) {
    if (a_enchant) {
        if (a_enchant->data.flags.any(RE::EnchantmentItem::EnchantmentFlag::kCostOverride) == true) {
            return true;
        }
        return false;
    } else {
        return false;
    }
}

void SetEnchantCostOverrideFlag(RE::StaticFunctionTag*, RE::EnchantmentItem* a_enchant, bool set) {
    if (a_enchant) {
        if (set == 1) {
            (a_enchant->data.flags.set(RE::EnchantmentItem::EnchantmentFlag::kCostOverride));

        } else if (set == 0) {
            (a_enchant->data.flags.reset(RE::EnchantmentItem::EnchantmentFlag::kCostOverride));
        }
    }
}

int GetEnchantCostOverrideValue(RE::StaticFunctionTag*, RE::EnchantmentItem* a_enchant) {
    if (a_enchant) {
        return a_enchant->data.costOverride;
    } else {
        return 11111;
    }
}

int GetEnchantChargeOverrideValue(RE::StaticFunctionTag*, RE::EnchantmentItem* a_enchant) {
    if (a_enchant) {
        return a_enchant->data.chargeOverride;
    } else {
        return 11111;
    }
}

void setEnchantCostOverrideValue(RE::StaticFunctionTag*, RE::EnchantmentItem* a_enchant, int CostOverride) {
    if (a_enchant) {
        a_enchant->data.costOverride = CostOverride;
    }
}

void setEnchantChargeOverrideValue(RE::StaticFunctionTag*, RE::EnchantmentItem* a_enchant, int ChargeOverride) {
    if (a_enchant) {  
    a_enchant->data.chargeOverride = ChargeOverride;
    }
}

void AdjustActiveEffectMagnitude(RE::StaticFunctionTag*, RE::ActiveEffect* a_activeEffect, float a_power) {
    if (a_activeEffect) {
        float magnitude = a_activeEffect->magnitude;
        a_activeEffect->magnitude = magnitude * a_power;
    }
}


void AdjustActiveEffectDuration(RE::StaticFunctionTag*, RE::ActiveEffect* a_activeEffect, float a_duration) {
    if (a_activeEffect) {
        float duration = a_activeEffect->duration;
        a_activeEffect->magnitude = duration * a_duration;
    }
}


bool PapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
    vm->RegisterFunction("CastSpellItemMult", "ASR_PapyrusFunctions", CastSpellItemMult);
    vm->RegisterFunction("CastSpellMult", "ASR_PapyrusFunctions", CastSpellMult);
    vm->RegisterFunction("GetEffectWasDualCast", "ASR_PapyrusFunctions", GetEffectWasDualCast);
    vm->RegisterFunction("AdjustActiveEffectMagnitude", "ASR_PapyrusFunctions", AdjustActiveEffectMagnitude);
    vm->RegisterFunction("AdjustActiveEffectDuration", "ASR_PapyrusFunctions", AdjustActiveEffectDuration);
    vm->RegisterFunction("GetEnchantCostOverrideFlag", "ASR_PapyrusFunctions", GetEnchantCostOverrideFlag);
    vm->RegisterFunction("SetEnchantCostOverrideFlag", "ASR_PapyrusFunctions", SetEnchantCostOverrideFlag);
    vm->RegisterFunction("GetEnchantCostOverrideValue", "ASR_PapyrusFunctions", GetEnchantCostOverrideValue);
    vm->RegisterFunction("GetEnchantChargeOverrideValue", "ASR_PapyrusFunctions", GetEnchantChargeOverrideValue);
    vm->RegisterFunction("setEnchantCostOverrideValue", "ASR_PapyrusFunctions", setEnchantCostOverrideValue);
    vm->RegisterFunction("setEnchantChargeOverrideValue", "ASR_PapyrusFunctions", setEnchantChargeOverrideValue);
    return true;
}



SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);
    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message *message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        
        if (Settings::load_config("Data/SKSE/Plugins/Asrak.toml"s)) {
                RE::ConsoleLog::GetSingleton()->Print("Atweaks Settings Loaded");
        } else {
                RE::ConsoleLog::GetSingleton()->Print("Atweaks Settings not found");

        }

        //    Serialization::GetSingleton()->SetGlobalValue(1, LookupActorValueByName("Health"));
            Hooks::Install();

        //RE::ConsoleLog::GetSingleton()->Print("Hello, worldssss!");
   }
        });
    SKSE::GetPapyrusInterface()->Register(PapyrusFunctions);
    //(Settings::load_config("Data/SKSE/Plugins/Asrak.toml"s));
    return true;
}
//
//
//
//                    if (IsCurrentAnimSubString(a_Killed->animationName.c_str(), "Stagger")) {
//    // RE::ConsoleLog::GetSingleton()->Print(a_Killed->animationName.c_str());
//    // a_Killed->playbackSpeed *= 0.98;
//    // a_Killed->playbackSpeed = std::max(a_Killed->playbackSpeed, 0.03f);
//    // a_Killed->time += *g_deltaTime;
//
//    //   a_Killed->binding->animation.get()->duration
//
//    if (a_Killed->time > 1.0f) a_Killed->playbackSpeed += -0.05f;
//    // if (a_Killed->time  1.0f) a_Killed->playbackSpeed = 0.05f;
//
//    // a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kRightMobilityCondition) / 10000.0f;
//    // a_timesteps *= 2.0f;
//
//    // a_Killed->playbackSpeed = std::min(a_Killed->playbackSpeed, 1.0f);
//    char buffer[150];
//    sprintf_s(buffer, "angles: %f %f %f %f %f %i", a_Killed->playbackSpeed, a_Killed->localTime, a_Killed->time,
//              a_Killed->binding->animation.get()->duration, a_timesteps, a_Killed->atEnd);
//    RE::ConsoleLog::GetSingleton()->Print(buffer);
//
//    // a_timesteps *= a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kRightMobilityCondition) / 100.0f;
//    // a_actor->GetActorRuntimeData().currentProcess->middleHigh->unk2DC = 3.0f;
//}
//if (IsAnimPlaying(a_actor, "Stagger")) RE::ConsoleLog::GetSingleton()->Print(a_Killed->animationName.c_str());


//   struct StartApplying {
//    static bool thunk(RE::MagicTarget* a_MT, RE::MagicTarget::AddTargetData& ATD) {
//        // auto a = a_AMC->GetCasterAsActor();
//        // auto source = a_AMC->GetCastingSource();
//        RE::ConsoleLog::GetSingleton()->Print("Hello, scritter!");
//        RE::Effect* firsteffect = ATD.magicItem->effects[0];
//        if (ATD.effect == firsteffect) {
//            if (ATD.magicItem->GetFullName()) RE::ConsoleLog::GetSingleton()->Print((ATD.magicItem->GetFullName()));
//        }
//        return func(a_MT, ATD);
//    }
//    static inline REL::Relocation<decltype(thunk)> func;
//};
//struct StartApplyinge {
//    static void thunk(RE::BSTEventSource<RE::TESHitEvent>& a_source, RE::TESHitEvent& a_event) {
//        // auto a = a_AMC->GetCasterAsActor();
//        // auto source = a_AMC->GetCastingSource();
//        const auto aggressor = a_event.cause.get();
//        const auto target = a_event.target.get();
//        const auto source = RE::TESForm::LookupByID(a_event.source);
//        const auto projectile = RE::TESForm::LookupByID<RE::BGSProjectile>(a_event.projectile);
//
//        if (aggressor && target && source && projectile) {
//            const auto aggressorActor = aggressor->As<RE::Actor>();
//            const auto targetActor = target->As<RE::Actor>();
//
//            auto spell = source->As<RE::MagicItem>();
//            if (!spell) {
//                if (const auto weapon = source->As<RE::TESObjectWEAP>(); weapon && weapon->IsStaff()) {
//                spell = weapon->formEnchanting;
//                }
//            }
//
//            RE::ConsoleLog::GetSingleton()->Print("Hello, sdf!");
//            if (spell->GetFullName()) RE::ConsoleLog::GetSingleton()->Print((spell->GetFullName()));
//        }
//        func(a_source, a_event);
//    }
//    static inline REL::Relocation<decltype(thunk)> func;
//};
#pragma once

class Settings
{
public:
    using bSetting = AutoTOML::bSetting;
    using iSetting = AutoTOML::iSetting;
    using sSetting = AutoTOML::sSetting;
    using fSetting = AutoTOML::fSetting;

    static inline bSetting CreatureWeaponSpeed{ "Patches", "CreatureWeaponSpeed", true };
    static inline bSetting WhirlwindSprint{"Patches", "WhirlwindSprint", true};
    static inline fSetting ProjectileSlowZone{"Patches", "ProjectileSlowZone", 50.0f};
    static inline bSetting MovementSpeedFix{"Patches", "MovementSpeedFix", true};
    static inline bSetting VariableNPC{"Patches", "VariableNPC", true};
    static inline fSetting VariableNPCweaponspeed{"Patches", "VariableNPCweaponspeed", 10.0f};
    static inline fSetting VariableNPCmovementspeed{"Patches", "VariableNPCmovementspeed", 10.0f};
    static inline bSetting BowEnchantFix{"Patches", "BowEnchantFix", true};
    static inline bSetting PowerAttackEnchant{"Patches", "PowerAttackEnchant", true};
    static inline fSetting PowerAttackEnchantBaseMag{"Patches", "PowerAttackEnchantBaseMag", 2.0f};
    





    //static inline bSetting RequiresPerk{ "Settings", "RequiresPerk", false };
    //static inline iSetting RequiredPerkID{ "Settings", "RequiredPerkID", 0xBEE97 };
    //static inline sSetting RequiredPerkModName{ "Settings", "RequiredPerkModName", "Skyrim.esm" };

    static bool load_config(const std::string& a_path)
    {
        try {
            const auto table = toml::parse_file(a_path);

            const auto& settings = ISetting::get_settings();

            for (const auto& setting : settings) {
                try {
                    setting->load(table);
                } catch (const std::exception& e) {
                    //logs::warn(fmt::runtime(e.what()));
                }
            }
        } catch (const toml::parse_error& e) {
            //std::ostringstream ss;
            //ss
            //    << "Error parsing file \'" << *e.source().path
            //    << "\':\n"
            //    << e.description()
            //    << "\n  (" << e.source().begin << ")\n";
            //logs::error(fmt::runtime(ss.str()));
            return false;
        }
        return true;
    }
private:
    using ISetting = AutoTOML::ISetting;

    Settings();   
};
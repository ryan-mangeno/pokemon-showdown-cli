#include <pkmpch.h>

#include "battlestate.h"
#include "parser.h"
#include "ident.h"

#include <nlohmann/json.hpp>

namespace pkm::protocol {

    const Pokemon& BattleState::active_pokemon() const {
        for (const Pokemon& p : m_your_team) {
            if (p.active) return p;
        }
        // should never happen in a valid battle state
        PK_ERROR("No active pokemon found!");
        return m_your_team[0];
    }

    int BattleState::turn() const {
        return m_turn;
    }

    Pokemon* BattleState::find_pokemon(const Ident& ident) {
        std::vector<Pokemon>& team = (ident.side == m_your_side) ? m_your_team : m_opponent_team;

        for (Pokemon& p : team) {
            if (p.name == ident.name) return &p;
        }

        return nullptr;
    }

    void BattleState::apply(const Message& msg) {
        if (msg.type == "request") {
            apply_request(msg);
        } else if (msg.type == "-damage" || msg.type == "-heal") {
            apply_hp(msg);
        } else if (msg.type == "faint") {
            apply_faint(msg);
        } else if (msg.type == "switch" || msg.type == "drag") {
            apply_switch(msg);
        } else if (msg.type == "turn") {
            m_turn = std::stoi(msg.args[0]);
        }
    }

    void BattleState::apply_request(const Message& msg) {
        if (msg.args[0].empty()) return;
        const auto j = nlohmann::json::parse(msg.args[0]);

        if (m_your_side.empty()) {
            m_your_team.clear(); 
            m_your_name = j["side"]["name"].get<std::string>();
            m_your_side = j["side"]["id"].get<std::string>(); // Grabs "p1" or "p2"

            // clean up the opponent team
            // If our lead pokemon was accidentally put on the opponents team 
            // during the opening switches, this erases it
            m_opponent_team.erase(
                std::remove_if(m_opponent_team.begin(), m_opponent_team.end(),
                    [this](const Pokemon& p) {
                        // check if the pokemon actually belongs to us
                        return parse_ident(p.ident).side == m_your_side;
                    }),
                m_opponent_team.end()
            );

            PK_INFO("Your Team:");
            for (auto& p : j["side"]["pokemon"]) {
                Pokemon pkm;
                pkm.ident   = p["ident"].get<std::string>();
                pkm.name    = parse_ident(pkm.ident).name;
                pkm.active  = p["active"].get<bool>(); 
                pkm.fainted = false;

                std::string condition = p["condition"].get<std::string>();
                size_t slash = condition.find('/');
                if (slash != std::string::npos) {
                    pkm.hp_current = std::stoi(condition.substr(0, slash));
                    pkm.hp_max     = std::stoi(condition.substr(slash + 1));
                }

                for (auto& m : p["moves"]) {
                    pkm.moves.push_back(m.get<std::string>());
                }
                
                PK_INFO(" - {}", pkm.name.c_str());
                m_your_team.emplace_back(pkm);
            }
        }

        // update available moves every request
        m_available_moves.clear();
        m_force_switch = j.contains("forceSwitch");
        if (j.contains("active")) {
            for (auto& m : j["active"][0]["moves"]) {
                MoveOption opt;
                opt.name     = m["move"].get<std::string>();
                opt.id       = m["id"].get<std::string>();
                opt.pp       = m["pp"].get<int>();
                opt.max_pp   = m["maxpp"].get<int>();
                opt.disabled = m["disabled"].get<bool>();
                m_available_moves.push_back(std::move(opt));
            }
        }
    }

    void BattleState::apply_hp(const Message& msg) {
        // args[0] = ident, args[1] = "250/288" or "0 fnt"
        Ident ident = parse_ident(msg.args[0]);
        Pokemon* pkm  = find_pokemon(ident);
        if (!pkm) return;

        std::string condition = msg.args[1];
        size_t slash = condition.find('/');
        if (slash != std::string::npos) {
            pkm->hp_current = std::stoi(condition.substr(0, slash));
            pkm->hp_max     = std::stoi(condition.substr(slash + 1));
        }
    }

    void BattleState::apply_faint(const Message& msg) {
        Ident ident = parse_ident(msg.args[0]);
        Pokemon* pkm  = find_pokemon(ident);
        if (!pkm) return;
        pkm->fainted   = true;
        pkm->active    = false;
        pkm->hp_current = 0;
    }

    void BattleState::apply_switch(const Message& msg) {
        // args[0] = ident, args[1] = details, args[2] = condition
        Ident ident = parse_ident(msg.args[0]);
        std::vector<Pokemon>& team = (ident.side == m_your_side) ? m_your_team : m_opponent_team;
        
        // deactivate all
        for (Pokemon& p : team) p.active = false;

        // find or create the switching pokemon
        Pokemon* pkm = find_pokemon(ident);
        if (!pkm) {
            // opponent pokemon we havent seen yet
            Pokemon newPkm;
            newPkm.ident = msg.args[0];
            newPkm.name  = ident.name;
            team.push_back(std::move(newPkm));
            pkm = &team.back();
        }

        pkm->active  = true;
        pkm->fainted = false;

        // update hp from condition
        std::string condition = msg.args[2];
        size_t slash = condition.find('/');
        if (slash != std::string::npos) {
            pkm->hp_current = std::stoi(condition.substr(0, slash));
            pkm->hp_max     = std::stoi(condition.substr(slash + 1));
        }
    }
}

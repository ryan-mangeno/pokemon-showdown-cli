#pragma once

#include <string>
#include <vector>

#include "message.h"
#include "ident.h"

namespace pkm::protocol {

    struct Pokemon {
        std::string name;
        std::string ident;        // "p1: Donphan"
        int hp_current;
        int hp_max;
        bool active;
        bool fainted;
        std::vector<std::string> moves;
        std::string status;       // burned, paralyzed etc
    };

    // current available moves from request
    struct MoveOption {
        std::string name;
        std::string id;
        int pp;
        int max_pp;
        bool disabled;
    };
    
    class BattleState {
        public:
            void apply(const Message& msg);
        
            const Pokemon& active_pokemon() const;

            inline const std::string& get_battleroom() const noexcept { return m_room_id; }
            inline const std::vector<Pokemon>& your_team() const noexcept { return m_your_team; };
            inline const std::vector<Pokemon>& opponent_team() const noexcept { return m_opponent_team; };
            inline const std::vector<MoveOption>& available_moves() const noexcept { return m_available_moves; };
            inline bool is_force_switch() const noexcept { return m_force_switch; }
            
            int turn() const;
        
        private:
            Pokemon* find_pokemon(const Ident& ident);

            void apply_request(const Message& msg);
            void apply_hp(const Message& msg);
            void apply_faint(const Message& msg);
            void apply_switch(const Message& msg);

        private:
            bool m_force_switch;
            int m_turn;
            std::string m_room_id;
            std::string m_your_name;
            std::string m_your_side;
            std::string m_opponent_name;
            std::string m_tera_option;
            std::vector<Pokemon> m_your_team;
            std::vector<Pokemon> m_opponent_team;
            std::vector<MoveOption> m_available_moves;

    };
}

// world/world.hpp -- The World class defines the game world as a whole and handles the passage of time, as well as keeping track of the player's current activities.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Mobile;                   // defined in world/mobile.hpp
class Player;                   // defined in world/player.hpp
class Room;                     // defined in world/room.hpp
enum class LinkTag : uint16_t;  // defined in world/room.hpp
enum class RoomTag : uint16_t;  // defined in world/room.hpp
enum class Security : uint8_t;  // defined in world/room.hpp


class World
{
public:
            World();    // Constructor, loads the room YAML data.
    const std::shared_ptr<Room> get_room(uint32_t room_id) const;           // Retrieves a specified Room by ID.
    const std::shared_ptr<Room> get_room(const std::string &room_id) const; // As above, but with a Room ID string.
    const std::shared_ptr<Mobile>   player() const; // Retrieves a pointer to the Player object.

private:
    static const std::map<std::string, uint8_t>     LIGHT_LEVEL_MAP;    // Lookup table for converting textual light levels (e.g. "bright") to integer values.
    static const std::map<std::string, LinkTag>     LINK_TAG_MAP;       // Lookup table for converting LinkTag text names into enums.
    static const std::map<std::string, RoomTag>     ROOM_TAG_MAP;       // Lookup table for converting RoomTag text names into enums.
    static const std::map<std::string, Security>    SECURITY_MAP;       // Lookup table for converting textual room security (e.g. "anarchy") to enum values.

    std::shared_ptr<Player> m_player;   // The player character.
    std::map<uint32_t, std::shared_ptr<Room>>   m_room_pool; // All the Rooms in the game.

    void    load_room_pool();   // Loads the Room YAML data into memory.
};

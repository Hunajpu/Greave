// world/world.cpp -- The World class defines the game world as a whole and handles the passage of time, as well as keeping track of the player's current activities.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "3rdparty/yaml-cpp/yaml.h"
#include "actions/look.hpp"
#include "core/core.hpp"
#include "core/filex.hpp"
#include "core/guru.hpp"
#include "core/message.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/player.hpp"
#include "world/room.hpp"
#include "world/time-weather.hpp"
#include "world/world.hpp"


// Lookup table for converting EquipSlot text names into enums.
const std::map<std::string, EquipSlot>  World::EQUIP_SLOT_MAP = { { "about", EquipSlot::ABOUT_BODY }, { "armour", EquipSlot::ARMOUR }, { "body", EquipSlot::BODY },
    { "feet", EquipSlot::FEET }, { "hands", EquipSlot::HANDS }, { "head", EquipSlot::HEAD }, { "held", EquipSlot::HAND_MAIN } };

// Lookup table for converting ItemSub text names into enums.
const std::map<std::string, ItemSub>    World::ITEM_SUBTYPE_MAP = { { "clothing", ItemSub::CLOTHING }, { "heavy", ItemSub::HEAVY }, { "light", ItemSub::LIGHT },
    { "medium", ItemSub::MEDIUM }, { "melee", ItemSub::MELEE }, { "none", ItemSub::NONE } };

// Lookup table for converting ItemTag text names into enums.
const std::map<std::string, ItemTag>    World::ITEM_TAG_MAP = { { "preferoffhand", ItemTag::PreferOffHand }, { "twohanded", ItemTag::TwoHanded } };

// Lookup table for converting ItemType text names into enums.
const std::map<std::string, ItemType>   World::ITEM_TYPE_MAP = { { "armour", ItemType::ARMOUR }, { "key", ItemType::KEY }, { "light", ItemType::LIGHT },
    { "none", ItemType::NONE }, { "weapon", ItemType::WEAPON } };

// Lookup table for converting textual light levels (e.g. "bright") to integer values.
const std::map<std::string, uint8_t>    World::LIGHT_LEVEL_MAP = { { "bright", 7 }, { "dim", 5 }, { "wilderness", 5 }, { "dark", 3 }, { "none", 0 } };

// Lookup table for converting LinkTag text names into enums.
const std::map<std::string, LinkTag>    World::LINK_TAG_MAP = { { "autoclose", LinkTag::AutoClose }, { "autolock", LinkTag::AutoLock }, { "decline", LinkTag::Decline },
    { "doormetal", LinkTag::DoorMetal }, { "doorshop", LinkTag::DoorShop }, { "doublelength", LinkTag::DoubleLength }, { "hidden", LinkTag::Hidden }, { "incline", LinkTag::Incline },
    { "lockable", LinkTag::Lockable },  { "locked", LinkTag::LockedByDefault }, { "lockstrong", LinkTag::LockStrong }, { "lockswhenclosed", LinkTag::LocksWhenClosed },
    { "lockweak", LinkTag::LockWeak }, { "noblockexit", LinkTag::NoBlockExit }, { "nomobroam", LinkTag::NoMobRoam }, { "ocean", LinkTag::Ocean }, { "open", LinkTag::Open },
    { "openable", LinkTag::Openable }, { "permalock", LinkTag::Permalock }, { "sky", LinkTag::Sky}, { "sky2", LinkTag::Sky2 }, { "sky3", LinkTag::Sky3 },
    { "triplelength", LinkTag::TripleLength }, { "window", LinkTag::Window } };

// Lookup table for converting RoomTag text names into enums.
const std::map<std::string, RoomTag>    World::ROOM_TAG_MAP = { { "canseeoutside", RoomTag::CanSeeOutside }, { "churchaltar", RoomTag::ChurchAltar }, { "digok", RoomTag::DigOK },
    { "gamepoker", RoomTag::GamePoker }, { "gameslots", RoomTag::GameSlots }, { "gross", RoomTag::Gross }, { "heatedinterior", RoomTag::HeatedInterior },
    { "hidecampfirescar", RoomTag::HideCampfireScar }, { "indoors", RoomTag::Indoors }, { "maze", RoomTag::Maze }, { "nexus", RoomTag::Nexus },
    { "noexplorecredit", RoomTag::NoExploreCredit }, { "permacampfire", RoomTag::PermaCampfire }, { "private", RoomTag::Private }, { "radiationlight", RoomTag::RadiationLight },
    { "shopbuyscontraband", RoomTag::ShopBuysContraband }, { "shoprespawningowner", RoomTag::ShopRespawningOwner }, { "sleepok", RoomTag::SleepOK },
    { "sludgepit", RoomTag::SludgePit }, { "smelly", RoomTag::Smelly }, { "trees", RoomTag::Trees }, { "underground", RoomTag::Underground }, { "verywide", RoomTag::VeryWide },
    { "waterclean", RoomTag::WaterClean }, { "waterdeep", RoomTag::WaterDeep }, { "watersalt", RoomTag::WaterSalt }, { "watershallow", RoomTag::WaterShallow },
    { "watertainted", RoomTag::WaterTainted }, { "wide", RoomTag::Wide } };

// Lookup table for converting textual room security (e.g. "anarchy") to enum values.
const std::map<std::string, Security>   World::SECURITY_MAP = { { "anarchy", Security::ANARCHY }, { "low", Security::LOW }, { "high", Security::HIGH },
    { "sanctuary", Security::SANCTUARY }, { "inaccessible", Security::INACCESSIBLE } };


// Constructor, loads the room YAML data.
World::World() : m_player(std::make_shared<Player>()), m_time_weather(std::make_shared<TimeWeather>())
{
    load_room_pool();
    load_item_pool();
    load_mob_pool();
    load_generic_descs();
}

// Adds a Mobile to the world.
void World::add_mobile(std::shared_ptr<Mobile> mob)
{
    bool parser_id_valid;
    int tries = 0;
    do
    {
        parser_id_valid = true;
        if (!mob->parser_id()) parser_id_valid = false;
        else
        {
            for (unsigned int i = 0; i < m_mobiles.size(); i++)
                if (m_mobiles.at(i)->parser_id() == mob->parser_id()) parser_id_valid = false;
        }
        if (!parser_id_valid) mob->new_parser_id();
    } while (!parser_id_valid && ++tries < 100000);
    m_mobiles.push_back(mob);
}

// Retrieves a generic description string.
std::string World::generic_desc(const std::string &id) const
{
    auto it = m_generic_descs.find(id);
    if (it == m_generic_descs.end())
    {
        core()->guru()->nonfatal("Invalid generic description requested: " + id, Guru::ERROR);
        return "-";
    }
    return it->second;
}

// Retrieves a specified Item by ID.
const std::shared_ptr<Item> World::get_item(const std::string &item_id) const
{
    if (!item_id.size()) throw std::runtime_error("Blank item ID requested.");
    const auto it = m_item_pool.find(StrX::hash(item_id));
    if (it == m_item_pool.end()) throw std::runtime_error("Invalid item ID requested: " + item_id);
    return std::make_shared<Item>(*it->second);
}

// Retrieves a specified Mobile by ID.
const std::shared_ptr<Mobile> World::get_mob(const std::string &mob_id) const
{
    if (!mob_id.size()) throw std::runtime_error("Blank mobile ID requested.");
    const auto it = m_mob_pool.find(StrX::hash(mob_id));
    if (it == m_mob_pool.end()) throw std::runtime_error("Invalid mobile ID requested: " + mob_id);
    return std::make_shared<Mobile>(*it->second);
}

// Retrieves a specified Room by ID.
const std::shared_ptr<Room> World::get_room(uint32_t room_id) const
{
    const auto it = m_room_pool.find(room_id);
    if (it == m_room_pool.end()) throw std::runtime_error("Invalid room ID requested: " + std::to_string(room_id));
    return it->second;
}

// As above, but with a Room ID string.
const std::shared_ptr<Room> World::get_room(const std::string &room_id) const
{
    if (!room_id.size()) throw std::runtime_error("Blank room ID requested.");
    else return get_room(StrX::hash(room_id));
}

// Checks if a specified item ID exists.
bool World::item_exists(const std::string &str) const { return m_item_pool.find(StrX::hash(str)) != m_item_pool.end(); }

// Loads the World and all things within it.
void World::load(std::shared_ptr<SQLite::Database> save_db)
{
    core()->messagelog()->load(save_db);
    for (auto room : m_room_pool)
        room.second->load(save_db);
    const uint32_t player_sql_id = m_player->load(save_db, 0);
    m_time_weather->load(save_db);

    SQLite::Statement query(*save_db, "SELECT sql_id FROM mobiles WHERE sql_id != " + std::to_string(player_sql_id) + " ORDER BY sql_id ASC");
    while (query.executeStep())
    {
        auto new_mob = std::make_shared<Mobile>();
        new_mob->load(save_db, query.getColumn("sql_id").getUInt());
        add_mobile(new_mob);
    }
}

// Retrieves a Mobile by vector position.
const std::shared_ptr<Mobile> World::mob(uint32_t vec_pos) const
{
    if (vec_pos >= m_mobiles.size()) throw std::runtime_error("Invalid mobile vector position.");
    return m_mobiles.at(vec_pos);
}

// Returns the number of Mobiles currently active.
unsigned int World::mob_count() const { return m_mobiles.size(); }

// Loads the generic descriptions YAML data into memory.
void World::load_generic_descs()
{
    const YAML::Node yaml_descs = YAML::LoadFile("data/generic descriptions.yml");
    for (auto desc : yaml_descs)
        m_generic_descs.insert(std::make_pair(desc.first.as<std::string>(), desc.second.as<std::string>()));
}

// Loads the Item YAML data into memory.
void World::load_item_pool()
{
    const std::vector<std::string> item_files = FileX::files_in_dir("data/items", true);
    for (auto item_file : item_files)
    {
        const YAML::Node yaml_items = YAML::LoadFile("data/items/" + item_file);
        for (auto item : yaml_items)
        {
            const YAML::Node item_data = item.second;

            // Create a new Item object.
            const std::string item_id_str = item.first.as<std::string>();
            const uint32_t item_id = StrX::hash(item_id_str);
            const auto new_item(std::make_shared<Item>());

            // Check to make sure there are no hash collisions.
            if (m_item_pool.find(item_id) != m_item_pool.end()) throw std::runtime_error("Item ID hash conflict: " + item_id_str);

            // The Item's name.
            if (!item_data["name"]) throw std::runtime_error("Missing item name: " + item_id_str);
            new_item->set_name(item_data["name"].as<std::string>());

            // The Item's type and subtype.
            if (!item_data["type"]) throw std::runtime_error("Missing item type: " + item_id_str);
            std::string item_type_str, item_subtype_str;
            if (item_data["type"].IsSequence())
            {
                const unsigned int seq_size = item_data["type"].size();
                if (seq_size < 1 || seq_size > 2) throw std::runtime_error("Item type data malforned: " + item_id_str);
                item_type_str = item_data["type"][0].as<std::string>();
                if (seq_size == 2) item_subtype_str = item_data["type"][1].as<std::string>();
            }
            else item_type_str = item_data["type"].as<std::string>();
            ItemType type = ItemType::NONE;
            ItemSub subtype = ItemSub::NONE;
            if (item_type_str.size())
            {
                const auto it = ITEM_TYPE_MAP.find(item_type_str);
                if (it == ITEM_TYPE_MAP.end()) core()->guru()->nonfatal("Invalid item type on " + item_id_str + ": " + item_type_str, Guru::ERROR);
                else type = it->second;
            }
            if (item_subtype_str.size())
            {
                const auto it = ITEM_SUBTYPE_MAP.find(item_subtype_str);
                if (it == ITEM_SUBTYPE_MAP.end()) core()->guru()->nonfatal("Invalid item subtype on " + item_id_str + ": " + item_subtype_str, Guru::ERROR);
                else subtype = it->second;
            }
            new_item->set_type(type, subtype);

            // The Item's tags, if any.
            if (item_data["tags"])
            {
                if (!item_data["tags"].IsSequence()) core()->guru()->nonfatal("{r}Malformed item tags: " + item_id_str, Guru::ERROR);
                else for (auto tag : item_data["tags"])
                {
                    const std::string tag_str = StrX::str_tolower(tag.as<std::string>());
                    const auto tag_it = ITEM_TAG_MAP.find(tag_str);
                    if (tag_it == ITEM_TAG_MAP.end()) core()->guru()->nonfatal("Unrecognized item tag (" + tag_str + "): " + item_id_str, Guru::ERROR);
                    else new_item->set_tag(tag_it->second);
                }
            }

            // The Item's metadata, if any.
            if (item_data["metadata"]) StrX::string_to_metadata(item_data["metadata"].as<std::string>(), *new_item->meta_raw());

            // The Item's EquipSlot, if any.
            if (item_data["slot"])
            {
                const std::string slot_str = item_data["slot"].as<std::string>();
                const auto slot_it = EQUIP_SLOT_MAP.find(slot_str);
                if (slot_it == EQUIP_SLOT_MAP.end()) core()->guru()->nonfatal("Unrecognized equipment slot (" + slot_str + "): " + item_id_str, Guru::ERROR);
                else new_item->set_equip_slot(slot_it->second);
            }

            // The Item's power, if any.
            if (item_data["power"]) new_item->set_power(item_data["power"].as<int>());

            // The Item's description, if any.
            if (item_data["description"]) new_item->set_description(item_data["description"].as<std::string>());

            // The Item's speed, if any.
            if (item_data["speed"]) new_item->set_speed(item_data["speed"].as<float>());

            // Add the new Item to the item pool.
            m_item_pool.insert(std::make_pair(item_id, new_item));
        }
    }
}

// Loads the Mobile YAML data into memory.
void World::load_mob_pool()
{
    const std::vector<std::string> mobile_files = FileX::files_in_dir("data/mobiles", true);
    for (auto mobile_file : mobile_files)
    {
        const YAML::Node yaml_mobiles = YAML::LoadFile("data/mobiles/" + mobile_file);
        for (auto mobile : yaml_mobiles)
        {
            const YAML::Node mobile_data = mobile.second;

            // Create a new Mobile object, and remember its unique ID.
            const std::string mobile_id_str = mobile.first.as<std::string>();
            const uint32_t mobile_id = StrX::hash(mobile_id_str);
            const auto new_mob(std::make_shared<Mobile>());

            // Check to make sure there are no hash collisions.
            if (m_mob_pool.find(mobile_id) != m_mob_pool.end()) throw std::runtime_error("Mobile ID hash conflict: " + mobile_id_str);

            // The Mobile's name.
            if (!mobile_data["name"]) core()->guru()->nonfatal("Missing mobile name: " + mobile_id_str, Guru::ERROR);
            else new_mob->set_name(mobile_data["name"].as<std::string>());

            // The Mobile's hit points.
            if (!mobile_data["hp"]) core()->guru()->nonfatal("Missing mobile hit points: "+ mobile_id_str, Guru::ERROR);
            else new_mob->set_hp(mobile_data["hp"].as<int>(), mobile_data["hp"].as<int>());

            // Add the Mobile to the mob pool.
            m_mob_pool.insert(std::make_pair(mobile_id, new_mob));
        }
    }
}

// Loads the Room YAML data into memory.
void World::load_room_pool()
{
    const std::vector<std::string> area_files = FileX::files_in_dir("data/areas", true);
    for (auto area_file : area_files)
    {
        const YAML::Node yaml_rooms = YAML::LoadFile("data/areas/" + area_file);
        for (auto room : yaml_rooms)
        {
            const YAML::Node room_data = room.second;

            // Create a new Room object, and set its unique ID.
            const std::string room_id = room.first.as<std::string>();
            const auto new_room(std::make_shared<Room>(room_id));

            // Check to make sure there are no hash collisions.
            if (m_room_pool.find(new_room->id()) != m_room_pool.end()) throw std::runtime_error("Room ID hash conflict: " + room_id);

            // The Room's long and short names.
            if (!room_data["name"] || room_data["name"].size() < 2) core()->guru()->nonfatal("Missing or invalid room name(s): " + room_id, Guru::ERROR);
            else new_room->set_name(room_data["name"][0].as<std::string>(), room_data["name"][1].as<std::string>());

            // The Room's description.
            if (!room_data["desc"]) core()->guru()->nonfatal("Missing room description: " + room_id, Guru::WARN);
            else new_room->set_desc(room_data["desc"].as<std::string>());

            // Links to other Rooms.
            if (room_data["exits"])
            {
                for (unsigned int e = 0; e < Room::ROOM_LINKS_MAX; e++)
                {
                    const Direction dir = static_cast<Direction>(e);
                    const std::string dir_str = StrX::dir_to_name(dir);
                    if (room_data["exits"][dir_str]) new_room->set_link(dir, room_data["exits"][dir_str].as<std::string>());
                }
            }

            // The light level of the Room.
            if (!room_data["light"]) core()->guru()->nonfatal("Missing room light level: " + room_id, Guru::ERROR);
            else
            {
                const std::string light_str = room_data["light"].as<std::string>();
                auto level_it = LIGHT_LEVEL_MAP.find(light_str);
                if (level_it == LIGHT_LEVEL_MAP.end()) core()->guru()->nonfatal("Invalid light level value: " + room_id, Guru::ERROR);
                else new_room->set_base_light(level_it->second);
            }

            // The security level of this Room.
            if (!room_data["security"]) core()->guru()->nonfatal("Missing room security level: " + room_id, Guru::ERROR);
            else
            {
                const std::string sec_str = room_data["security"].as<std::string>();
                auto sec_it = SECURITY_MAP.find(sec_str);
                if (sec_it == SECURITY_MAP.end()) core()->guru()->nonfatal("Invalid security level value: " + room_id, Guru::ERROR);
                else new_room->set_security(sec_it->second);
            }

            // Room tags, if any.
            if (room_data["tags"])
            {
                if (!room_data["tags"].IsSequence()) core()->guru()->nonfatal("{r}Malformed room tags: " + room_id, Guru::ERROR);
                else for (auto tag : room_data["tags"])
                {
                    const std::string tag_str = StrX::str_tolower(tag.as<std::string>());
                    bool directional_tag = false;
                    int dt_int = 0, dt_offset = 0;

                    if (tag_str.size() > 9)
                    {
                        if (tag_str.substr(0, 9) == "northeast")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::NORTHEAST);
                            dt_offset = 9;
                        }
                        else if (tag_str.substr(0, 9) == "northwest")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::NORTHWEST);
                            dt_offset = 9;
                        }
                        else if (tag_str.substr(0, 9) == "southeast")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::SOUTHEAST);
                            dt_offset = 9;
                        }
                        else if (tag_str.substr(0, 9) == "southwest")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::SOUTHWEST);
                            dt_offset = 9;
                        }
                    }
                    if (tag_str.size() > 5 && !directional_tag)
                    {
                        if (tag_str.substr(0, 5) == "north")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::NORTH);
                            dt_offset = 5;
                        }
                        else if (tag_str.substr(0, 5) == "south")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::SOUTH);
                            dt_offset = 5;
                        }
                    }
                    if (tag_str.size() > 4 && !directional_tag)
                    {
                        if (tag_str.substr(0, 4) == "east")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::EAST);
                            dt_offset = 4;
                        }
                        else if (tag_str.substr(0, 4) == "west")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::WEST);
                            dt_offset = 4;
                        }
                        else if (tag_str.substr(0, 4) == "down")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::DOWN);
                            dt_offset = 4;
                        }
                    }
                    if (tag_str.size() > 2 && !directional_tag)
                    {
                        if (tag_str.substr(0, 2) == "up")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::UP);
                            dt_offset = 2;
                        }
                    }

                    if (!directional_tag)
                    {
                        const auto tag_it = ROOM_TAG_MAP.find(tag_str);
                        if (tag_it == ROOM_TAG_MAP.end()) core()->guru()->nonfatal("Unrecognized room tag (" + tag_str + "): " + room_id, Guru::WARN);
                        else new_room->set_tag(tag_it->second);
                    }
                    else
                    {
                        const std::string dtag_str = tag_str.substr(dt_offset);
                        const auto dtag_it = LINK_TAG_MAP.find(dtag_str);
                        if (dtag_it == LINK_TAG_MAP.end()) core()->guru()->nonfatal("Unrecognized link tag (" + dtag_str + "): " + room_id, Guru::WARN);
                        else
                        {
                            const LinkTag lt = dtag_it->second;
                            switch (lt)
                            {
                                case LinkTag::Lockable:
                                case LinkTag::Window:
                                    new_room->set_link_tag(dt_int, LinkTag::Openable);
                                    break;
                                case LinkTag::LockedByDefault:
                                    new_room->set_link_tag(dt_int, LinkTag::Lockable);
                                    new_room->set_link_tag(dt_int, LinkTag::Openable);
                                    break;
                                case LinkTag::Open:
                                    new_room->set_link_tag(dt_int, LinkTag::Openable);
                                    break;
                                default: break;
                            }
                            new_room->set_link_tag(dt_int, lt);
                        }
                    }
                }
            }

            // Add the new Room to the room pool.
            m_room_pool.insert(std::make_pair(new_room->id(), new_room));
        }
    }
}

// Sets up for a new game.
void World::new_game()
{
    m_player->set_location("OUTSIDE_QUEENS_GATE");
    auto new_mob = get_mob("TEST_GOBLIN");
    new_mob->set_location("OUTSIDE_QUEENS_GATE");
    add_mobile(new_mob);
    ActionLook::look(m_player);
}

// Retrieves a pointer to the Player object.
const std::shared_ptr<Player> World::player() const { return m_player; }

// Purges null entries from the active Mobiles. Only call this from the main loop, for safety.
void World::purge_mobs()
{
    for (unsigned int i = 0; i < m_mobiles.size(); i++)
    {
        if (m_mobiles.at(i) == nullptr)
        {
            m_mobiles.erase(m_mobiles.begin() + i);
            i--;
            continue;
        }
    }
}

// Removes a Mobile from the world.
void World::remove_mobile(std::shared_ptr<Mobile> mob)
{
    for (unsigned int i = 0; i < m_mobiles.size(); i++)
    {
        if (m_mobiles.at(i) == mob)
        {
            m_mobiles.at(i) = nullptr;
            return;
        }
    }
    core()->guru()->nonfatal("Attempt to remove mobile that does not exist in the world: " + mob->name(), Guru::ERROR);
}

// Checks if a specified room ID exists.
bool World::room_exists(const std::string &str) const { return m_room_pool.find(StrX::hash(str)) != m_room_pool.end(); }

// Saves the World and all things within it.
void World::save(std::shared_ptr<SQLite::Database> save_db)
{
    save_db->exec(Room::SQL_ROOMS);
    save_db->exec(Player::SQL_PLAYER);
    save_db->exec(Mobile::SQL_MOBILES);
    save_db->exec(Item::SQL_ITEMS);
    save_db->exec(MessageLog::SQL_MSGLOG);
    save_db->exec(TimeWeather::SQL_TIME_WEATHER);

    m_player->save(save_db);
    core()->messagelog()->save(save_db);
    m_time_weather->save(save_db);

    for (auto room : m_room_pool)
        room.second->save(save_db);
    for (auto mob : m_mobiles)
        mob->save(save_db);
}

// Gets a pointer to the TimeWeather object.
const std::shared_ptr<TimeWeather> World::time_weather() const { return m_time_weather; }

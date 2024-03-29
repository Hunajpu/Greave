// actions/look.cpp -- Look around you. Just look around you.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/look.hpp"
#include "core/core.hpp"
#include "core/guru.hpp"
#include "core/mathx.hpp"
#include "core/parser.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/mobile.hpp"
#include "world/room.hpp"
#include "world/world.hpp"


// Examines an Item or Mobile.
void ActionLook::examine(std::shared_ptr<Mobile> mob, ParserTarget target_type, size_t target)
{
    switch (target_type)
    {
        case ParserTarget::TARGET_EQUIPMENT: examine_item(mob, mob->equ()->get(target)); break;
        case ParserTarget::TARGET_INVENTORY: examine_item(mob, mob->inv()->get(target)); break;
        case ParserTarget::TARGET_MOBILE: examine_mobile(mob, core()->world()->mob_vec(target)); break;
        case ParserTarget::TARGET_ROOM: examine_item(mob, core()->world()->get_room(mob->location())->inv()->get(target)); break;
        default: core()->guru()->nonfatal("Invalid examine target.", Guru::ERROR);
    }
}

// Examines an Item.
void ActionLook::examine_item(std::shared_ptr<Mobile>, std::shared_ptr<Item> target)
{
    core()->message("You are looking at: " + target->name(Item::NAME_FLAG_FULL_STATS | Item::NAME_FLAG_ID | Item::NAME_FLAG_RARE));
    if (target->desc().size()) core()->message("{0}" + target->desc());
    std::string stat_string;
    switch (target->type())
    {
        case ItemType::ARMOUR:
        {
            switch (target->subtype())
            {
                case ItemSub::CLOTHING: stat_string = "This is {U}clothing {w}that can be worn"; break;
                case ItemSub::HEAVY: stat_string = "This is {U}heavy armour {w}that can be worn"; break;
                case ItemSub::LIGHT: stat_string = "This is {U}lightweight armour {w}that can be worn"; break;
                case ItemSub::MEDIUM: stat_string = "This is {U}medium armour {w}that can be worn"; break;
                default: break;
            }
            std::string slot;
            switch (target->equip_slot())
            {
                case EquipSlot::ABOUT_BODY: slot = "about the body"; break;
                case EquipSlot::ARMOUR: slot = "over your body"; break;
                case EquipSlot::BODY: slot = "on your body"; break;
                case EquipSlot::FEET: slot = "on your feet"; break;
                case EquipSlot::HANDS: slot = "on your hands"; break;
                case EquipSlot::HEAD: slot = "on your head"; break;
                default: break;
            }
            stat_string += " {U}" + slot + "{w}. It has an armour value of {U}" + std::to_string(target->power());
            const int warmth = target->warmth();
            if (warmth) stat_string += "{w}, and a warmth rating of {U}" + std::to_string(warmth);
            stat_string += "{w}. ";
            break;
        }
        case ItemType::KEY: stat_string = "This is a {U}key {w}which can unlock certain doors. "; break;
        case ItemType::LIGHT:
            stat_string = "This is a {U}light source {w}which can be held. It provides a brightness level of {Y}" + std::to_string(target->power()) + "{w} when used. ";
            break;
        case ItemType::NONE: break;
        case ItemType::SHIELD:
            stat_string = "This is a {U}shield {w}which can be wielded. It has an armour value of {U}" + std::to_string(target->power()) + "{w}. ";
            break;
        case ItemType::WEAPON:
        {
            switch (target->subtype())
            {
                case ItemSub::MELEE: stat_string = "This is a {U}melee weapon {w}which can be wielded. "; break;
                default: break;
            }
            if (target->tag(ItemTag::TwoHanded)) stat_string += "It is heavy and requires {U}two hands {w}to wield. ";
            std::string damage_type_str;
            switch (target->damage_type())
            {
                case DamageType::ACID: damage_type_str = "acid"; break;
                case DamageType::BALLISTIC: damage_type_str = "ballistic"; break;
                case DamageType::CRUSHING: damage_type_str = "crushing"; break;
                case DamageType::EDGED: damage_type_str = "edged"; break;
                case DamageType::ENERGY: damage_type_str = "energy"; break;
                case DamageType::EXPLOSIVE: damage_type_str = "explosive"; break;
                case DamageType::KINETIC: damage_type_str = "kinetic"; break;
                case DamageType::PIERCING: damage_type_str = "piercing"; break;
                case DamageType::PLASMA: damage_type_str = "plasma"; break;
                case DamageType::POISON: damage_type_str = "poison"; break;
                case DamageType::RENDING: damage_type_str = "randing"; break;
                default: core()->guru()->nonfatal("Unable to determine item damage type: " + target->name(), Guru::ERROR);
            }
            stat_string += "It has a damage value of {U}" + std::to_string(target->power()) + " " + damage_type_str + "{w}, a speed of {U}" +
                StrX::ftos(target->speed(), true) + "{w}, and a critical hit chance of {U}" + std::to_string(target->crit()) + "%{w}. ";
            break;
        }
    }

    const int dodge_mod = target->dodge_mod();
    if (dodge_mod > 0) stat_string += "It {G}boosts your chance to dodge {w}by {G}" + std::to_string(dodge_mod) + "%{w}. ";
    else if (dodge_mod < 0) stat_string += "It {Y}reduces your chance to dodge {w}by {R}" + std::to_string(-dodge_mod) + "%{w}. ";

    const int parry_mod = target->parry_mod();
    if (parry_mod > 0) stat_string += "It {G}boosts your chance to parry {w}by {G}" + std::to_string(parry_mod) + "%{w}. ";
    else if (parry_mod < 0) stat_string += "It {Y}reduces your chance to parry {w}by {R}" + std::to_string(-parry_mod) + "%{w}. ";

    const int block_mod = target->block_mod();
    if (block_mod > 0) stat_string += "It {G}boosts your chance to shield-block {w}by {G}" + std::to_string(block_mod) + "%{w}. ";
    else if (block_mod < 0) stat_string += "It {Y}reduces your chance to shield-block {w}by {R}" + std::to_string(-block_mod) + "%{w}. ";

    uint32_t weight = MathX::fuzz(target->weight());
    stat_string += "{w}It weighs around {U}" + StrX::intostr_pretty(weight) + (weight == 1 ? " pac{w}. " : " pacs{w}. ");
    if (stat_string.size())
    {
        stat_string.pop_back();
        core()->message("{0}" + stat_string);
    }
}

// Examines a Mobile.
void ActionLook::examine_mobile(std::shared_ptr<Mobile>, std::shared_ptr<Mobile> target)
{
    core()->message("You are looking at: " + target->name());
}

// Take a look around at your surroundings.
void ActionLook::look(std::shared_ptr<Mobile> mob)
{
    // We're going to just assume the Mobile specified is the player. If not... we'll look around anyway, from the Mobile's point of view. Why? Because even if it doesn't make much
    // sense right now, you never know, this code might be used in the future for some sort of "see through the target's eyes" spell or something.
    const std::shared_ptr<Room> room = core()->world()->get_room(mob->location());

    if (room->light(mob) < Room::LIGHT_VISIBLE)
    {
        core()->message("{U}Darkness");
        core()->message("{0}```{u}It is {B}pitch black{u}, and you can see {B}nothing{u}. You are likely to be eaten by a grue.");
        return;
    }

    room->set_tag(RoomTag::Explored);

    // Room name, description, and obvious exits.
    core()->message("{G}" + room->name());
    core()->message("{0}```" + room->desc());

    // Show any scar effects on the room.
    const std::string scar_desc = room->scar_desc();
    if (scar_desc.size()) core()->message("{0}```" + scar_desc);

    // Show the obvious exits.
    obvious_exits(mob, true);

    // Items nearby.
    if (room->inv()->count())
    {
        std::vector<std::string> items_nearby;
        for (size_t i = 0; i < room->inv()->count(); i++)
            items_nearby.push_back(room->inv()->get(i)->name(Item::NAME_FLAG_CORE_STATS) + "{w}");
        core()->message("{0}{g}```Items: {w}" + StrX::comma_list(items_nearby, StrX::CL_FLAG_USE_AND));
    }

    // Mobiles nearby.
    std::vector<std::string> mobs_nearby;
    for (size_t i = 0; i < core()->world()->mob_count(); i++)
    {
        const auto world_mob = core()->world()->mob_vec(i);
        if (!world_mob) continue; // Ignore any nullptr Mobiles.
        if (world_mob->location() != mob->location()) continue; // Ignore any Mobiles not in this Room.
        mobs_nearby.push_back((world_mob->is_hostile() ? "{R}" : "{Y}") + world_mob->name(Mobile::NAME_FLAG_NO_COLOUR) + "{w}");
    }
    if (mobs_nearby.size()) core()->message("{0}{g}```Nearby: {w}" + StrX::comma_list(mobs_nearby, StrX::CL_FLAG_USE_AND));
}

// Lists the exits from this area.
void ActionLook::obvious_exits(std::shared_ptr<Mobile> mob, bool indent)
{
    const std::shared_ptr<Room> room = core()->world()->get_room(mob->location());
    if (room->light(mob) < Room::LIGHT_VISIBLE)
    {
        core()->message(std::string(indent ? "{0}" : "") + "{u}It's so {B}dark{u}, you can't see where the exits are!");
        return;
    }

    std::vector<std::string> exits_vec;
    for (int e = 0; e < Room::ROOM_LINKS_MAX; e++)
    {
        uint32_t room_link = room->link(e);
        if (!room_link || room_link == Room::BLOCKED) continue;
        if (room->link_tag(e, LinkTag::Hidden)) continue;   // Never list hidden exits.
        std::string exit_name = "{c}" + StrX::dir_to_name(e);
        const std::string door_name = room->door_name(e);

        if (room_link == Room::UNFINISHED)  // An exit that is due to be finished later.
            exit_name = "{r}(" + StrX::dir_to_name(e) + "){c}";
        else if (room_link == Room::FALSE_ROOM)
        {
            if (room->link_tag(e, LinkTag::KnownLocked)) exit_name += " {m}[locked<>]{c}";
            else exit_name += " {u}[closed<>]{c}";
        }
        else
        {
            const std::shared_ptr<Room> link_room = core()->world()->get_room(room_link);
            if (link_room->tag(RoomTag::Explored) && !link_room->tag(RoomTag::Maze)) exit_name += " {B}(" + link_room->name(true) + "){c}";
            if (room->link_tag(e, LinkTag::KnownLocked)) exit_name += " {m}[locked<>]{c}";
            else if (room->link_tag(e, LinkTag::Openable))
            {
                if (room->link_tag(e, LinkTag::Open)) exit_name += " {u}[open<>]{c}";
                else exit_name += " {u}[closed<>]{c}";
            }
        }

        if (door_name == "door" || door_name == "metal door") StrX::find_and_replace(exit_name, "<>", "");   // Don't specify 'door' if it's just a door.
        else StrX::find_and_replace(exit_name, "<>", " " + door_name);  // Any other 'door' types (windows, grates, etc.) should be specified.

        exits_vec.push_back(exit_name);
    }
    if (exits_vec.size()) core()->message(std::string(indent ? "{0}```" : "") + "{g}Obvious exits: " + StrX::comma_list(exits_vec, StrX::CL_FLAG_USE_AND));
}

// world/inventory.hpp -- The Inventory class stores a collection of Items, and handles stacking, organizing, saving/loading, etc.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Item;                             // defined in world/item.hpp
enum class EquipSlot : uint8_t;         // defined in world/item.hpp
namespace SQLite { class Database; }    // defined in 3rdparty/SQLiteCpp/Database.h


class Inventory
{
public:
    void        add_item(std::shared_ptr<Item> item);   // Adds an Item to this Inventory (this will later handle auto-stacking, etc.)
    void        add_item(const std::string &id);        // As above, but generates a new Item from a template with a specified ID.
    size_t      count() const;                          // Returns the number of Items in this Inventory.
    void        erase(size_t pos);                      // Deletes an Item from this Inventory.
    std::shared_ptr<Item> get(size_t pos) const;        // Retrieves an Item from this Inventory.
    std::shared_ptr<Item> get(EquipSlot es) const;      // As above, but retrieves an item based on a given equipment slot.
    void        load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id);   // Loads an Inventory from the save file.
    void        remove_item(size_t pos);                // Removes an Item from this Inventory.
    void        remove_item(EquipSlot es);              // As above, but with a specified equipment slot.
    uint32_t    save(std::shared_ptr<SQLite::Database> save_db);    // Saves this Inventory, returns its SQL ID.

private:
    bool            parser_id_exists(uint16_t id);          // Checks if a given parser ID already exists on an Item in this Inventory.

    std::vector<std::shared_ptr<Item>>  m_items;            // The Items stored in this Inventory.
};

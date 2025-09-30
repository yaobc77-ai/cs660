// src/db/Database.cpp

#include <db/Database.hpp>
#include <stdexcept>
#include <utility>

namespace db {

    BufferPool &Database::getBufferPool() {
        return bufferPool;
    }

    void Database::add(std::unique_ptr<DbFile> file) {
        // Guard against null and duplicate names.
        if (!file) {
            throw std::logic_error("DbFile pointer is null.");
        }
        const std::string name = file->getName();
        auto [it, inserted] = catalog.emplace(name, std::move(file));
        if (!inserted) {
            throw std::logic_error("DbFile with name '" + name + "' already exists.");
        }
    }

    std::unique_ptr<DbFile> Database::remove(const std::string &name) {
        // Find file; if absent, throw.
        auto it = catalog.find(name);
        if (it == catalog.end()) {
            throw std::logic_error("DbFile with name '" + name + "' does not exist.");
        }

        // Flush any dirty pages for this file before removing from the catalog.
        bufferPool.flushFile(name);

        // Move ownership to caller and erase from catalog.
        std::unique_ptr<DbFile> out = std::move(it->second);
        catalog.erase(it);
        return out;
    }

    DbFile &Database::get(const std::string &name) const {
        auto it = catalog.find(name);
        if (it == catalog.end()) {
            throw std::logic_error("DbFile with name '" + name + "' does not exist.");
        }
        return *(it->second);
    }

    // Global accessor to the singleton Database.
    Database &getDatabase() {
        static Database instance;
        return instance;
    }

} // namespace db

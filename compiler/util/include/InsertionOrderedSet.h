#ifndef INSERTIONORDEREDSET_H
#define INSERTIONORDEREDSET_H

#include <list>
#include <unordered_map>

template <typename T>
class InsertionOrderedSet {
private:
    // List to maintain insertion order (front = most recent)
    std::list<T> items;

    // Unordered map for fast lookup: maps element to its iterator in the list
    std::unordered_map<T, typename std::list<T>::iterator> item_map;

public:
    // Insert an element.
    // If the element already exists, move it to the front.
    // Returns true if a new element was inserted, false if an existing element was moved.
    bool insert(const T &value)
    {
        auto it = item_map.find(value);
        if (it != item_map.end()) {
            // Element exists, move it to the front
            items.splice(items.begin(), items, it->second);
            return false; // Existing element was moved
        }
        // Insert new element at the front
        items.emplace_front(value);
        item_map[value] = items.begin();
        return true; // New element inserted
    }

    // Check if an element exists in the set
    bool contains(const T &value) const
    {
        return item_map.find(value) != item_map.end();
    }

    // Erase an element from the set.
    // Returns true if the element was erased, false if it was not found.
    bool erase(const T &value)
    {
        auto it = item_map.find(value);
        if (it == item_map.end()) {
            return false; // Element not found
        }
        items.erase(it->second);
        item_map.erase(it);
        return true;
    }

    // Get the number of elements in the set
    std::size_t size() const
    {
        return item_map.size();
    }

    // Check if the set is empty
    bool empty() const
    {
        return item_map.empty();
    }

    // Iterate over elements in insertion order (most recent first)
    auto begin() { return items.begin(); }

    auto end() { return items.end(); }

    auto begin() const { return items.begin(); }

    auto end() const { return items.end(); }

    auto rbegin() { return items.rbegin(); }

    auto rend() { return items.rend(); }

    auto rbegin() const { return items.rbegin(); }

    auto rend() const { return items.rend(); }

    // Clear the set
    void clear()
    {
        items.clear();
        item_map.clear();
    }
};

#endif
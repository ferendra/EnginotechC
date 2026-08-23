// Collections Library for EnginotechC++
#ifndef ENG_STD_COLLECTIONS_H
#define ENG_STD_COLLECTIONS_H

#include <vector>
#include <map>
#include <string>
#include <functional>

namespace eng {
namespace std {

// Generic Vector template
template<typename T>
class Vector {
public:
    Vector() = default;
    
    void push(const T& item) { data.push_back(item); }
    void pop() { if (!data.empty()) data.pop_back(); }
    T& get(int index) { return data[index]; }
    const T& get(int index) const { return data[index]; }
    bool empty() const { return data.empty(); }
    int size() const { return (int)data.size(); }
    
    T& operator[](int index) { return data[index]; }
    const T& operator[](int index) const { return data[index]; }
    
private:
    std::vector<T> data;
};

// Generic Map template
template<typename K, typename V>
class Map {
public:
    void put(const K& key, const V& value) { data[key] = value; }
    V get(const K& key) const {
        auto it = data.find(key);
        return it != data.end() ? it->second : V{};
    }
    bool contains(const K& key) const { return data.find(key) != data.end(); }
    void remove(const K& key) { data.erase(key); }
    bool empty() const { return data.empty(); }
    int size() const { return (int)data.size(); }
    
private:
    std::map<K, V> data;
};

// Type aliases
template<typename T>
using Vec = Vector<T>;

template<typename K, typename V>
using Dict = Map<K, V>;

} // namespace std
} // namespace eng

#endif // ENG_STD_COLLECTIONS_H

/* 
 * MultiSetMap is a header-only library which defines a generic map with sets as values
 * Copyright (C) 2025 Adam Riha
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 * For more information regarding this project please contact me at rihaadam1<at>seznam.cz.
 */

#ifndef MULTI_SET_MAP_H
#define MULTI_SET_MAP_H

#include <algorithm>
#include <cctype>
#include <concepts>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

template <typename T>
concept HasStartsWith = requires(T t, const std::string & s)
{
	{ t.starts_with(s) } -> std::convertible_to<bool>;
};

/// <summary>
/// A generic map with keys associated to sets of (possibly) multiple values
/// </summary>
/// <typeparam name="Key">Type of key used to identify the set of values associated to it</typeparam>
/// <typeparam name="Value">Type of values inside the set</typeparam>
/// <typeparam name="MapCompare">Comparator for how to sort keys</typeparam>
/// <typeparam name="SetCompare">Comparator for how to sort values in sets</typeparam>
template <typename Key, typename Value, typename MapCompare = std::less<Key>, typename SetCompare = std::less<Key>>
class MultiSetMap
{
public:
	/// <summary>
	/// Adds value to a set identified by key
	/// </summary>
	/// <param name="key">Key to identify the set</param>
	/// <param name="value">Value to insert into the set</param>
	void add(const Key& key, const Value& value)
	{
		map_[key].insert(value);
	}

	/// <summary>
	/// Removes entire key-value pair identified by key from the map
	/// </summary>
	/// <param name="key">Key to remove</param>
	void remove(const Key& key)
	{
		map_.erase(key);
	}

	/// <summary>
	/// Removes value from a set identified by key,
	/// removes whole key-value pair from map if no values left
	/// </summary>
	/// <param name="key">Key to identify the set</param>
	/// <param name="value">Value to remove from the set</param>
	void remove(const Key& key, const Value& value)
	{
		auto it = map_.find(key);

		// Do nothing if key does not exist
		if (it == map_.end())
		{
			return;
		}

		// Remove value from set
		it->second.erase(value);

		// Remove key if set is empty
		if (it->second.empty())
		{
			map_.erase(it);
		}
	}

	/// <summary>
	/// Finds set by key
	/// </summary>
	/// <param name="key">Key to identify the set</param>
	/// <returns>Const reference to the set or empty set if key does not exist</returns>
	const std::set<Value, SetCompare>& find(const Key& key) const
	{
		auto it = map_.find(key);
		return it != map_.end() ? it->second : emptySet;
	}

	/// <summary>
	/// Finds all key-value pairs whose key starts with prefix.
	/// Only defined for types with starts_with method
	/// and case-insensitivity for std::string only
	/// </summary>
	/// <typeparam name="K"></typeparam>
	/// <param name="prefix">Prefix to select keys by</param>
	/// <param name="caseSensitive">Whether to ignore cases</param>
	/// <returns>Vector of found pairs: <key with prefix, set of values></returns>
	template<typename K = Key>
		requires HasStartsWith<K>
	inline std::vector<std::pair<Key, const std::set<Value, SetCompare>&>> findByPrefix(const Key& prefix, bool caseSensitive = true) const
	{
		std::vector<std::pair<Key, const std::set<Value, SetCompare>&>> prefixKVPs;

		// Case sensitive
		if (caseSensitive)
		{
			auto it = map_.lower_bound(prefix);
			while (it != map_.end() && it->first.starts_with(prefix))
			{
				prefixKVPs.emplace_back(it->first, it->second);
				++it;
			}
		}
		// Case insensitive
		else if (std::is_same<Key, std::string>::value) // For std::string only
		{
			for (auto it = map_.begin(); it != map_.end(); ++it)
			{
				Key keyLower = it->first;
				Key prefixLower = prefix;

				// Convert to lower case
				std::transform(keyLower.begin(), keyLower.end(), keyLower.begin(), [](unsigned char c) {
					return std::tolower(c);
				});
				std::transform(prefixLower.begin(), prefixLower.end(), prefixLower.begin(), [](unsigned char c) {
					return std::tolower(c);
				});

				if (keyLower.starts_with(prefixLower))
				{
					prefixKVPs.emplace_back(it->first, it->second);
				}
			}
		}

		return prefixKVPs;
	}

private:
	std::map<Key, std::set<Value, SetCompare>, MapCompare> map_;

	inline static const std::set<Value, SetCompare> emptySet{};
};

#endif // MULTI_SET_MAP_H

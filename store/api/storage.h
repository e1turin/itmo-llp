#pragma once
#include "node.h"
#include "store/dom/value.h"
#include "store/memory/memory_manager.h"
#include "util/util.h"

#include <optional>
#include <variant>

class Storage final {
public:
  using Data = std::variant<bool, std::int32_t, float, std::string,
                            std::nullptr_t>; // nullptr stands for null-object
  using ObjEntries = std::vector<std::pair<Node, Node>>;

  explicit Storage(std::string_view, bool replace_if_exists = false);

  /**
   * Returns node of tree root if everything is OK.
   * @return Optional node (reference to value).
   */
  [[nodiscard]] std::optional<Node> root() const;
  /**
   * Returns type of value underneath the node.
   * @param node Node which type we need to know.
   * @return Type of given node
   */
  [[nodiscard]] std::optional<dom::Value::Type> get_type(Node node) const;

  /**
   * Get value stored underneath the given node.
   * @param node Node which value is required to read.
   * @return Sum type of all available data types for the storage.
   */
  [[nodiscard]] std::optional<Data> read(Node node);
  /**
   * Get child node of given parent node by given string key.
   * @param node Node which child is wanted to get.
   * @param key String value of key, by
   * @return Optional node. Null-node if no key is presented.
   */
  [[nodiscard]] std::optional<Node> get(Node node, std::string_view key);
  /**
   * Return collection of nodes stored in object value.
   * @param node Node which children are required to get.
   * @return Optional collection of nodes. Emplty collection if node has no entries.
   */
  [[nodiscard]] std::optional<ObjEntries> get_entries(Node node);
  /**
   * Set value to given node with specified key. Updates value if entry already
   * exists or creates key and sets it's value if not exists. Also frees memory
   * occupied of already stored value.
   * @param node Node to be edit.
   * @param key Name of key respondent to a value which well be edited.
   * @param data Sum type of data in storage.
   * @return Optional node of value that have been just stored.
   */
  std::optional<Node> set(Node node, std::string_view key, const Data &data);
  /**
   * Truncates the data assigned to the node, e.g. char array for string or
   * array of object entries. Performs recursively if necessary.
   * The node will be substituted with null-object on complete.
   * @param node Node which data is to be cleared.
   * @return true if truncation was performed.
   */
  bool truncate(Node node);
  /**
   * Truncates the data assigned to the specified key of object-node,
   * Performs recursively if necessary. The node's key and corresponding
   * value will be truncated from the entries.
   * @param node Node of object value, which entry value with given key must be cleared.
   * @param key Name of object entry key. Value assosiated with this key must be cleared.
   * @return true if truncation was performed.
   */
  bool truncate(Node node, std::string_view key);

private:
  /**
   * Unpack integer value from given value object in DOM.
   * @param v Value object which is supposed to store integer.
   * @return Integer value underneath Value object if object is Int32Value type else nullopt.
   */
  [[nodiscard]]
  static std::optional<std::int32_t> read(const dom::Int32Value &v);
  /**
   * Unpack float value from given value object in DOM.
   * @param v Value object which is supposed to store float.
   * @return Float value underneath Value object if object is Float32Value type else nullopt.
   */
  [[nodiscard]]
  static std::optional<float> read(const dom::Float32Value &v);
  /**
   * Unpack bool value from given value object in DOM.
   * @param v Value object which is supposed to store bool.
   * @return Bool value underneath Value object if object is BoolValue type else nullopt.
   */
  [[nodiscard]]
  static std::optional<bool> read(const dom::BoolValue &v);
  /**
   * Gets string value from given value object in DOM.
   * @param v Value object which is supposed to store reference to string data.
   * @return Copy of string data referenced by StringValue object if object is
   * StringValue type and has correct reference else nullopt.
   */
  [[nodiscard]]
  std::optional<std::string> read(const dom::StringValue &v) const;
  [[nodiscard]] /* DEPRECATED ??? */
  std::optional<std::vector<dom::Entry>> read(const dom::ObjectValue &) const;
  /**
   * Appends entry to object node with given key and null-object value with
   * additional reallocation if necessary.
   * @param node Node referenced ObjectValue in DOM. Given node is supposed to
   * not have such key before.
   * @param key Name of key in entry to be appended to object data.
   * @return
   */
  [[nodiscard]] std::optional<Node> insert_key(Node node, std::string_view key) const;

private:
  std::unique_ptr<mem::MemoryManager> memm_;
};

#pragma once
#include "node.h"
#include "store/dom/value.h"
#include "store/memory/memory_manager.h"
#include "util/util.h"

#include <optional>
#include <variant>

class Storage final {
public:
  using Data = std::variant<bool, std::int32_t, float, std::string_view,
                            std::nullptr_t>; // nullptr stands for null-object
  using ObjEntries = std::vector<std::pair<Node, Node>>;

  Storage(std::string_view, bool replace_if_exists = false);

  [[nodiscard]] std::optional<Node> root() const;
  [[nodiscard]] std::optional<dom::Value::Type> get_type(Node) const;

  [[nodiscard]] std::optional<Data> read(Node);
  [[nodiscard]] std::optional<Node> get(Node, std::string_view);
  [[nodiscard]] std::optional<ObjEntries> get_entries(Node);
  std::optional<Node> set(Node, std::string_view, const Data &);
  /**
   * Truncates the data assigned to the node, e.g. char array for string or
   * array of object entries. Performs recursively if necessary.
   * The node will be substituted with null-object on complete.
   * @return true if truncation was performed.
   */
  bool truncate(Node);
  /**
   * Truncates the data assigned to the specified key of object-node,
   * Performs recursively if necessary. The node's key and corresponding
   * value will be truncated from the entries.
   * @return true if truncation was performed.
   */
  bool truncate(Node, std::string_view);

private:
  [[nodiscard]]
  static std::optional<std::int32_t> read(const dom::Int32Value &);
  [[nodiscard]]
  static std::optional<float> read(const dom::Float32Value &);
  [[nodiscard]]
  static std::optional<bool> read(const dom::BoolValue &);
  [[nodiscard]]
  std::optional<std::string_view> read(const dom::StringValue &) const;
  [[nodiscard]]
  std::optional<std::vector<dom::Entry>> read(const dom::ObjectValue &) const;
  [[nodiscard]] std::optional<Node> insert_key(Node, std::string_view) const;

private:
  std::unique_ptr<mem::MemoryManager> memm_;
};

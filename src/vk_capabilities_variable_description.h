/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk_capabilities::VariableDescriptions class
 * -------------------------------------------------------------------------- */

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace kuu
{
namespace vk_capabilities
{

/* -------------------------------------------------------------------------- *
   Reads variable descriptions from the variable description file.
 * -------------------------------------------------------------------------- */
class VariableDescriptions
{
public:
    struct Variable
    {
        std::string name;
        std::string description;
    };

    using VariableTransformFun = std::function<std::string(std::string)>;

    VariableDescriptions(
        const std::string& filePath,
        const VariableTransformFun& variableTransform = nullptr);

    std::vector<Variable> variables() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk_capabilities
} // namespace kuu

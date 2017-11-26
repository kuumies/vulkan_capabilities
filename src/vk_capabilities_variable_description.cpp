/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_capabilities::VariableDescriptions class
 * -------------------------------------------------------------------------- */

#include "vk_capabilities_variable_description.h"

/* -------------------------------------------------------------------------- */

#include <iostream>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

namespace kuu
{
namespace vk_capabilities
{

/* -------------------------------------------------------------------------- */

struct VariableDescriptions::Impl
{
    std::vector<Variable> variables;
};

/* -------------------------------------------------------------------------- */

VariableDescriptions::VariableDescriptions(
    const std::string& filePath,
    const VariableTransformFun& variableTransformFun)
    : impl(std::make_shared<Impl>())
{
    QFile qssFile(QString::fromStdString(filePath));
    if (!qssFile.open(QIODevice::ReadOnly))
    {
        std::cerr << __FUNCTION__
                  << "Failed to read formats from "
                  << filePath
                  << std::endl;
        return;
    }

    QString variable;
    QString desc;

    QTextStream ts(&qssFile);
    while(!ts.atEnd())
    {
        QString line = ts.readLine().trimmed();
        if (line.startsWith(";"))
            continue;
        if (line.isEmpty() && desc.isEmpty() && variable.isEmpty())
            continue;
        if (line.startsWith("@@"))
        {
            if (!desc.isEmpty())
            {
                if (variableTransformFun)
                    variable = QString::fromStdString(
                        variableTransformFun(variable.toStdString()));

                impl->variables.push_back(
                {
                    variable.toStdString(),
                    desc.toStdString()
                });
            }

            variable = QString();
            desc     = QString();

            continue;
        }

        if (variable.isEmpty())
        {
            variable = line;
            continue;
        }

        if (!desc.isEmpty())
            desc += "\n";
        desc += line;
    }

    if (!desc.isEmpty())
    {
        if (variableTransformFun)
            variable = QString::fromStdString(
                variableTransformFun(variable.toStdString()));

        impl->variables.push_back(
        {
            variable.toStdString(),
            desc.toStdString()
        });
    }
}

/* -------------------------------------------------------------------------- */

std::vector<VariableDescriptions::Variable>
    VariableDescriptions::variables() const
{ return impl->variables; }

} // namespace vk_capabilities
} // namespace kuu

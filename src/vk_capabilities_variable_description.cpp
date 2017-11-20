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
namespace
{

/* -------------------------------------------------------------------------- *
   Splits a long description text into smaller lines.
 * -------------------------------------------------------------------------- */
QString splitToLines(const QString& in, const int wordMax = 10)
{
    QStringList inWords = in.split(" ");
    QStringList outWords;
    QString out;
    int wordCount = 0;
    for (int i = 0; i < inWords.size(); ++i)
    {
        if (wordCount < wordMax)
        {
            outWords << inWords[i];
            wordCount++;
        }
        else
        {
            wordCount = 0;
            out += outWords.join(" ") += "\n";
            outWords.clear();
        }
    }
    out += outWords.join(" ");
    return out;
}

} // anonymous namespace

/* -------------------------------------------------------------------------- */

struct VariableDescriptions::Impl
{
    std::vector<VariableDescription> descriptions;
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

    QTextStream ts(&qssFile);
    while(!ts.atEnd())
    {
        QString line = ts.readLine().trimmed();
        if (line.startsWith(";"))
            continue;
        if (line.isEmpty())
            continue;

        QString variable = line.section(" ", 0, 0);
        if (variableTransformFun)
            variable = QString::fromStdString(
                variableTransformFun(variable.toStdString()));

        QString desc = variable + " " + line.section(" ", 1);
        desc = splitToLines(desc, 10);

        impl->descriptions.push_back(
        {
            variable.toStdString(),
            desc.toStdString()
        });
    }
}

/* -------------------------------------------------------------------------- */

std::vector<VariableDescriptions::VariableDescription>
    VariableDescriptions::variableDescriptions() const
{ return impl->descriptions; }

} // namespace vk_capabilities
} // namespace kuu

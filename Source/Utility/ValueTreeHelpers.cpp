#include "ValueTreeHelpers.h"
#include "Crc.h"

static void dumpValueTreeContentInternal (juce::ValueTree vt, bool displayProperties, int indentLevel, std::function<void (juce::String)> displayFunction)
{
    jassert (displayFunction != nullptr);
    juce::String indentString { std::string (indentLevel, ' ') };
    displayFunction (indentString + vt.getType ().toString ());
    if (displayProperties)
        for (auto propIndex { 0 }; propIndex < vt.getNumProperties (); ++propIndex)
        {
            const auto propertyName { vt.getPropertyName (propIndex) };
            displayFunction (indentString + "  " + propertyName + " = '" + vt.getProperty (propertyName).toString () + "'");
        }
    for (const auto& child : vt)
        dumpValueTreeContentInternal (child, displayProperties, indentLevel + 1, displayFunction);
}

namespace ValueTreeHelpers
{
    juce::ValueTree fromXmlString (juce::StringRef xmlString)
    {
        return juce::ValueTree::fromXml (*(juce::XmlDocument::parse (xmlString).get ()));
    }

    juce::ValueTree fromXmlData (const void* data, size_t size)
    {
        return fromXmlString (juce::MemoryBlock (data, size).toString ());
    }

    void forEachChild (juce::ValueTree parent, std::function<bool (juce::ValueTree child)> childCallback)
    {
        for (auto child : parent)
        {
            if (! childCallback (child))
                break;
        }
    }

    void forEachChildOfType (juce::ValueTree parent, juce::Identifier childType, std::function<bool (juce::ValueTree child)> childCallback)
    {
        forEachChild (parent, [childCallback, childType] (juce::ValueTree child)
        {
            if (child.getType () != childType)
                return true;

            return childCallback (child);
        });
    }

    void forEachProperty (juce::ValueTree vt, std::function<bool (juce::Identifier property)> propertyCallback)
    {
        jassert (propertyCallback != nullptr);
        const auto numProperties { vt.getNumProperties () };
        for (auto propertyIndex { 0 }; propertyIndex < numProperties; ++propertyIndex)
        {
            if (! propertyCallback (vt.getPropertyName(propertyIndex)))
                break;
        }

    }

    juce::ValueTree findChild (juce::ValueTree parent, std::function<bool (juce::ValueTree child)> findChildCallback)
    {
        const auto numChildren { parent.getNumChildren () };
        for (auto childIndex { 0 }; childIndex < numChildren; ++childIndex)
        {
            if (auto child { parent.getChild (childIndex) }; findChildCallback (child))
                return child;
        }

        return {};
    }

    juce::ValueTree getParentOfType (const juce::ValueTree child, const juce::Identifier parentId)
    {
        auto parent { child.getParent () };
        while (parent.isValid () && parent.getType () != parentId)
            parent = parent.getParent ();
        return parent;
    }

    juce::ValueTree getTypeFromRoot (const juce::ValueTree child, const juce::Identifier type)
    {
        auto parent { child.getParent () };
        while (parent.isValid () && parent.getParent ().isValid ())
            parent = parent.getParent ();

        if (! parent.isValid ())
            return {};

        return parent.getChildWithName (type);
    }

    void overwriteExistingChildren (juce::ValueTree source, juce::ValueTree dest)
    {
        for (auto child : source)
        {
            juce::ValueTree destChild = dest.getOrCreateChildWithName (child.getType (), nullptr);
            destChild.copyPropertiesAndChildrenFrom (child, nullptr);
        }
    }

    void overwriteExistingProperties (juce::ValueTree source, juce::ValueTree dest)
    {
        for (int i = 0; i < source.getNumProperties (); i++)
            dest.setProperty (source.getPropertyName (i), source.getProperty (source.getPropertyName (i)), nullptr);
    }

    void overwriteExistingChildrenAndProperties (juce::ValueTree source, juce::ValueTree dest)
    {
        overwriteExistingChildren (source, dest);
        overwriteExistingProperties (source, dest);
    }

    bool compareChidrenAndThierPropertiesUnordered (juce::ValueTree firstVT, juce::ValueTree secondVT, LogCompareFailures logCompareFailures, StopAtFirstFailure stopAtFirstFailure)
    {
        auto result { true };
        auto firstVtIterator { firstVT.begin () };
        auto secondVtIterator { secondVT.begin () };
        if (firstVT.getNumChildren () != secondVT.getNumChildren ())
        {
//             if (logCompareFailures == LogCompareFailures::yes)
//                 DebugLog ("compareChidrenAndThierPropertiesUnordered number of children differ - first: " + String (firstVT.getNumChildren ()) + ", second: " + String (secondVT.getNumChildren ()));
            jassertfalse;
            return false;
        }
        auto childIndex { 0 };
        while (firstVtIterator != firstVT.end ())
        {
            if (! comparePropertiesUnOrdered (*firstVtIterator, *secondVtIterator, logCompareFailures, stopAtFirstFailure))
            {
//                 if (logCompareFailures == LogCompareFailures::yes)
//                     DebugLog ("compareChidrenAndThierPropertiesUnordered/comparePropertiesUnOrdered failed. child index: " + String (childIndex));
                jassertfalse;
                return false;
            }
            ++firstVtIterator;
            ++secondVtIterator;
            ++childIndex;
        }
        return result;
    }

    bool comparePropertiesUnOrdered (juce::ValueTree firstVT, juce::ValueTree secondVT, LogCompareFailures logCompareFailures, StopAtFirstFailure stopAtFirstFailure)
    {
        auto success { true };
        if (logCompareFailures == LogCompareFailures::yes)
        {
//             if (firstVT.getNumProperties () != secondVT.getNumProperties ())
//                 DebugLog ("comparePropertiesUnOrdered - firstVT.getNumProperties (): " + String (firstVT.getNumProperties ()) +
//                     ", secondVT.getNumProperties ():" + String (secondVT.getNumProperties ()));
        }
        for (auto propertyIndex { 0 }; propertyIndex < firstVT.getNumProperties (); ++propertyIndex)
        {
            const auto propertyName { firstVT.getPropertyName (propertyIndex) };
            if (logCompareFailures == LogCompareFailures::yes)
            {
                //DebugLog ("checking property("+String(propertyIndex)+"): " + propertyName);
//                 if (! secondVT.hasProperty (propertyName))
//                     DebugLog ("comparePropertiesUnOrdered - ! secondVT.hasProperty (" + propertyName + ")");
//                 if (firstVT.getProperty (propertyName) != secondVT.getProperty (propertyName))
//                     DebugLog ("comparePropertiesUnOrdered - firstVT.getProperty (" + propertyName + ") [" + firstVT.getProperty (propertyName).toString () + "] != [" + secondVT.getProperty (propertyName).toString () + "] secondVT.getProperty (" + propertyName + ")");
            }
            if (! secondVT.hasProperty (propertyName) || firstVT.getProperty (propertyName) != secondVT.getProperty (propertyName))
            {
                success = false;
                if (stopAtFirstFailure == StopAtFirstFailure::yes)
                    break;
            }
        }
        return success;
    }

    uint32_t getCrc (juce::ValueTree tree)
    {
        Crc32 crc;
        juce::MemoryBlock block;
        juce::MemoryOutputStream stream (block, false);

        tree.writeToStream (stream);

        for (size_t i = 0; i < stream.getDataSize (); i++)
            crc.update (((uint8_t*) stream.getData ()) [i]);

        return crc.getCrc ();
    }

    void removePropertyIfExists (juce::ValueTree vt, juce::Identifier property)
    {
        if (vt.hasProperty (property))
            vt.removeProperty (property, nullptr);
    }

    void dumpValueTreeContent (juce::ValueTree vt, bool displayProperties, std::function<void (juce::String)> displayFunction)
    {
        dumpValueTreeContentInternal (vt, displayProperties, 0, displayFunction);
    }
};

#include "ValueTreeFile.h"

ValueTreeFile::ValueTreeFile () noexcept
    : Thread ("ValueTreeFile")
{

}

ValueTreeFile::ValueTreeFile (juce::ValueTree vt, juce::File f, bool enableAutoSave)
    : ValueTreeFile ()
{
    init (vt, f, enableAutoSave);
}

ValueTreeFile::~ValueTreeFile ()
{
    signalThreadShouldExit ();
    signalSave.signal ();
    stopThread (5000);
    vtData.removeListener (this);
}

void ValueTreeFile::init (juce::ValueTree vt, juce::File f, bool enableAutoSave)
{
    startThread ();
    vtData = vt;
    file = f;
    autoSaveEnabled = enableAutoSave;
    vtData.addListener (this);
    load ();
}

void ValueTreeFile::enableAutoSave (bool isEnabled) noexcept
{
    const auto wasEnabled { autoSaveEnabled };
    autoSaveEnabled = isEnabled;

    // if toggling from disabled to enabled, then request a delayed save
    if (! wasEnabled && isEnabled)
        requestAutoSave ();
}

void ValueTreeFile::save ()
{
    auto xmlToWrite { vtData.createXml () };
    save (xmlToWrite.get ());
}

void ValueTreeFile::save (juce::XmlElement* xmlToWrite)
{
    xmlToWrite->writeTo (file, {});
}

void ValueTreeFile::load ()
{
    if (file.exists ())
    {
        juce::XmlDocument xmlDoc (file);
        auto xmlToRead { xmlDoc.getDocumentElement () };
        if (xmlToRead == nullptr)
        {
            juce::Logger::writeToLog ("File \"" + file.getFullPathName () + "\" is corrupted and cannot be read!");
            jassertfalse;
            return;
        }

        loading.store (true);
        vtData.copyPropertiesAndChildrenFrom (juce::ValueTree::fromXml (*xmlToRead), nullptr);
        loading.store (false);
    }
}

void ValueTreeFile::setAutoSaveTimes (uint32_t sdt, uint32_t msdt) noexcept
{
    saveDelayTime = sdt;
    maxSaveDelayTime = juce::jmax (sdt, msdt);
}

void ValueTreeFile::requestAutoSave () noexcept
{
    if (autoSaveEnabled && ! loading.load ())
    {
        if (initialSaveRequestedTime == 0)
        {
            startTimer (saveDelayTime / 2);
            initialSaveRequestedTime = juce::Time::getMillisecondCounter ();
        }
        mostRecentSaveRequestedTime = juce::Time::getMillisecondCounter ();
    }
}

void ValueTreeFile::timerCallback ()
{
    const uint32_t curTime { juce::Time::getMillisecondCounter () };
    if (curTime - mostRecentSaveRequestedTime >= saveDelayTime ||
        curTime - initialSaveRequestedTime >= maxSaveDelayTime)
    {
        juce::ScopedLock xmlLock (xmlDataCS);
        if (xml.get () == nullptr)
        {
            stopTimer ();
            xml = vtData.createXml ();
            signalSave.signal ();
            initialSaveRequestedTime = 0;
        }
    }
}

void ValueTreeFile::run ()
{
    while (! threadShouldExit ())
    {
        signalSave.wait ();
        if (! threadShouldExit ())
        {
            std::unique_ptr<juce::XmlElement> xmlToWrite;
            {
                juce::ScopedLock xmlLock (xmlDataCS);
                xmlToWrite.reset (xml.release ());
            }
            if (xmlToWrite.get () != nullptr)
                save (xmlToWrite.get ());
        }
    }
}

void ValueTreeFile::valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier&)
{
    requestAutoSave ();
}
void ValueTreeFile::valueTreeChildAdded (juce::ValueTree&, juce::ValueTree&)
{
    requestAutoSave ();
}
void ValueTreeFile::valueTreeChildRemoved (juce::ValueTree&, juce::ValueTree& , int)
{
    requestAutoSave ();
}
void ValueTreeFile::valueTreeChildOrderChanged (juce::ValueTree&, int , int)
{
    requestAutoSave ();
}
void ValueTreeFile::valueTreeRedirected (juce::ValueTree& /*treeWhichHasBeenChanged*/)
{
    requestAutoSave ();
}
void ValueTreeFile::valueTreeParentChanged (juce::ValueTree&) noexcept
{
    // do nothing, because we are only saving the contents of our valuetree
}

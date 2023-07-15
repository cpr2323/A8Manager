#ifndef _VALUE_TREE_FILE_H_
#define _VALUE_TREE_FILE_H_

#include <JuceHeader.h>

// ValueTreeFile connects a ValueTree and a file, with auto-save functionality
//
// Auto-save is triggered when every anything in the ValueTree changes. The auto-save takes place
// on an independent thread, so as not to interrupt the message thread.
//
// To reduce the number of saves when there are many changes in a short period of time, it is a deferred
// save, waiting 'saveDelayTime' before doing the actual save. If second change happens
// the time is restarted. To keep a long series of rapid changes from keeping the save from
// happening, a save will be done if the total time since the first change exceeds maxSaveDelayTime.

class ValueTreeFile : private juce::ValueTree::Listener,
                      private juce::Timer,
                      private juce::Thread
{
public:
    ValueTreeFile () noexcept;
    ValueTreeFile (juce::ValueTree vt, juce::File f, bool enableAutoSave);
    ~ValueTreeFile ();

    void init (juce::ValueTree vt, juce::File f, bool enableAutoSave);
    void save ();
    void load ();
    void requestAutoSave () noexcept;
    void setAutoSaveTimes (uint32_t sdt, uint32_t msdt) noexcept;
    void enableAutoSave (bool isEnabled) noexcept;

private:
    juce::ValueTree vtData;
    juce::File file;
    uint32_t saveDelayTime    { 1000 };
    uint32_t maxSaveDelayTime { 5000 };
    uint32_t mostRecentSaveRequestedTime { 0 };
    uint32_t initialSaveRequestedTime    { 0 };
    bool autoSaveEnabled { false };
    juce::CriticalSection xmlDataCS;
    std::unique_ptr<juce::XmlElement> xml;
    juce::WaitableEvent signalSave;
    std::atomic<bool> loading { false };

    void save (juce::XmlElement* xmlToWrite);
    void run () override;
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;
    void valueTreeChildRemoved (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;
    void valueTreeChildOrderChanged (juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex) override;
    void valueTreeParentChanged (juce::ValueTree& treeWhoseParentHasChanged)  noexcept override;
    void valueTreeRedirected (juce::ValueTree& treeWhichHasBeenChanged) override;

    void timerCallback () override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueTreeFile)
};

#endif // _VALUE_TREE_FILE_H_

#include <JuceHeader.h>
#include "GUI/MainComponent.h"
#include "Utility/DebugLog.h"
#include "Utility/PersistentRootProperties.h"
#include "Utility/RuntimeRootProperties.h"
#include "Utility/ValueTreeFile.h"
#include <map>

const juce::String PropertiesFileExtension { ".properties" };

void crashHandler (void* /*data*/)
{
    FlushDebugLog ();
    juce::Logger::writeToLog (juce::SystemStats::getStackBacktrace ());
}

class A8ManagerApplication : public juce::JUCEApplication, public juce::Timer
{
public:
    A8ManagerApplication () {}

    const juce::String getApplicationName () override { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion () override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed () override { return true; }

    void initialise ([[maybe_unused]] const juce::String& commandLine) override
    {
        initAppDirectory ();
        initLogger ();
        initCrashHandler ();
        initPropertyRoots ();
        initAssimil8or();
        initUi ();

        // async quit timer
        startTimer (125);
    }

    void shutdown () override
    {
        appPropertiesFile.save ();
        mainWindow = nullptr; // (deletes our window)
        juce::Logger::setCurrentLogger (nullptr);
    }

    void anotherInstanceStarted ([[maybe_unused]] const juce::String& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }

    void suspended ()
    {
        runtimeRootProperties.triggerAppSuspended ();
    }

    void resumed ()
    {
        runtimeRootProperties.triggerAppResumed ();
    }

    void systemRequestedQuit ()
    {
        // reset preferred quit state
        runtimeRootProperties.setPreferredQuitState (RuntimeRootProperties::QuitState::now);
        // listeners for 'onSystemRequestedQuit' can do runtimeRootPropertiesVT.setPreferredQuitState (RuntimeRootProperties::QuitState::idle);
        // if they need to do something, which also makes them responsible for calling runtimeRootPropertiesVT.setQuitState (RuntimeRootProperties::QuitState::now); when they are done...
        runtimeRootProperties.triggerSystemRequestedQuit ();
        localQuitState.store (runtimeRootProperties.getPreferredQuitState ());
    }

    void timerCallback ()
    {
        if (localQuitState.load () == RuntimeRootProperties::QuitState::now)
            quit ();
    }

    void initAssimil8or ()
    {
//        bezier.wrap (persistentRootProperties.getValueTree (), ValueTreeWrapper::WrapperType::owner, ValueTreeWrapper::EnableCallbacks::no);
        // open file
        auto testPresetFile { appDirectory.getChildFile("test_preset.yml") };
        if (!testPresetFile.exists ())
        {
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon, "Test File Missing Error",
                "Unable to find test file: '" + testPresetFile.getFullPathName () + "'", {}, nullptr,
                juce::ModalCallbackFunction::create ([this] (int) {}));
            return;
        }
        juce::StringArray fileContents;
        testPresetFile.readLines (fileContents);
        // parse assumptions
        //  indent using 2 spaces
//         Preset 1 :
//           Name : template001
//           Channel 1 :
//             Release :  2.5000
//             ReleaseMod : 0B 1.00
//             ZonesCV : 0A
//             ZonesRT : 1
//             Zone 1 :
//               Sample : sampleA.wav
//               MinVoltage : +4.56
        auto scopeDepth { 0 };
        juce::Identifier PresetSectionId  { "Preset" };
        juce::Identifier PresetNamePropertyId { "Name" };

        juce::Identifier ChannelSectionId { "Channel" };
        juce::Identifier ZoneSectionId    { "Zone" };

        juce::ValueTree assimil8orData { "Assimil8or" };

        enum class ParseState
        {
            SeekingPresetSection,
            ParsingPresetSection,
            ParsingChannelSection,
            ParsingZoneSection,
        };
        ParseState parseState { ParseState::SeekingPresetSection };

        auto setParseState = [&parseState] (ParseState newParseState)
        {
            auto getParseStateString = [] (ParseState parseState) -> juce::String
            {
                switch (parseState)
                {
                    case ParseState::SeekingPresetSection: { return "SeekingPresetSection"; } break;
                    case ParseState::ParsingPresetSection: { return "ParsingPresetSection"; } break;
                    case ParseState::ParsingChannelSection: { return "ParsingChannelSection"; } break;
                    case ParseState::ParsingZoneSection: { return "ParsingZoneSection"; } break;
                    default: { return "[error]"; } break;
                }
            };
            parseState = newParseState;
//            juce::Logger::outputDebugString ("new state: " + getParseStateString (parseState));
        };

        juce::ValueTree curPresetSection;
        juce::ValueTree curChannelSection;
        juce::ValueTree curZoneSection;
        for (auto& lineFromFile : fileContents)
        {
            auto indent { lineFromFile.initialSectionContainingOnly (" ") };
            const auto previousScopeDepth { scopeDepth };
            scopeDepth = indent.length () / 2;
            const auto scopeDifference {scopeDepth - previousScopeDepth};
            if (scopeDifference < 0)
            {
//                juce::Logger::outputDebugString ("scopeDepth reduced by: " + juce::String(scopeDifference * -1));
                for (auto remainingScopes {scopeDifference * -1}; remainingScopes > 0; --remainingScopes)
                {
                    switch (parseState)
                    {
                    case ParseState::SeekingPresetSection:
                    {
                        jassertfalse;
                    }
                    break;
                    case ParseState::ParsingPresetSection:
                    {
                        curPresetSection = {};
                        setParseState (ParseState::SeekingPresetSection);
                    }
                    break;
                    case ParseState::ParsingChannelSection:
                    {
                        curChannelSection = {};
                        setParseState (ParseState::ParsingPresetSection);
                    }
                    break;
                    case ParseState::ParsingZoneSection:
                    {
                        curZoneSection = {};
                        setParseState (ParseState::ParsingChannelSection);
                    }
                    break;
                    default:
                    {
                        jassertfalse;
                    }
                    break;
                    }
                }
            }

            juce::Logger::outputDebugString (juce::String(scopeDepth) + "-" + lineFromFile);

            lineFromFile = lineFromFile.trim ();
            auto key { lineFromFile.upToFirstOccurrenceOf(":", false, false).trim () };
            auto values { lineFromFile.fromFirstOccurrenceOf (":", false, false).trim () };
            auto valueList { juce::StringArray::fromTokens (values, " ", "\"") };

            auto keyIs = [&key] (juce::String desiredKey)
            {
                return key.upToFirstOccurrenceOf (" ", false, false) == desiredKey;
            };
            auto getSectionIndex = [&key] ()
            {
                return key.fromFirstOccurrenceOf (" ", false, false);
            };
            auto addValueTreeChild = [&getSectionIndex] (juce::Identifier sectionId, juce::ValueTree parent)
            {
                auto section = juce::ValueTree { sectionId };
                section.setProperty ("index", getSectionIndex (), nullptr);
                parent.addChild (section, -1, nullptr);
                return section;
            };
            auto addChildValue = [] (juce::ValueTree parent, juce::String childName, std::function<void (juce::ValueTree)> setProperties)
            {
                auto child { juce::ValueTree {childName} };
                setProperties (child);
                parent.addChild (child, -1, nullptr);
            };

            switch (parseState)
            {
                case ParseState::SeekingPresetSection:
                {
                    if (keyIs (PresetSectionId.toString ()))
                    {
                        curPresetSection = addValueTreeChild (PresetSectionId, assimil8orData);
                        setParseState (ParseState::ParsingPresetSection);
                    }
                }
                break;
                case ParseState::ParsingPresetSection:
                {
                    if (keyIs (ChannelSectionId.toString ()))
                    {
                        curChannelSection = addValueTreeChild (ChannelSectionId, curPresetSection);
                        setParseState (ParseState::ParsingChannelSection);
                    }
                    else
                    {
                        // Name: template001
                        if (keyIs ("Name"))
                        {
                            auto nameChild { juce::ValueTree {"Name"}};
                            nameChild.setProperty ("name", valueList [0], nullptr);
                            curPresetSection.addChild (nameChild, -1, nullptr);
                        }
                        else
                            jassertfalse;
                    }
                }
                break;
                case ParseState::ParsingChannelSection:
                {
                    if (keyIs (ZoneSectionId.toString ()))
                    {
                        curZoneSection = addValueTreeChild (ZoneSectionId, curChannelSection);
                        setParseState (ParseState::ParsingZoneSection);
                    }
                    else
                    {
                        // Pitch : -16.79
                        // Level : -10.0
                        // Pan : -0.30
                        // Release : 99.0000
                        // MixLevel : -90.0
                        // PMSource : 8
                        // PitchCV : 0A 0.50
                        // PMIndexMod : 0C 0.18
                        // ReleaseMod : 0C 1.00
                        // PanMod : Off 0.00
                        // LoopStartMod : 0C 0.00
                        // LoopMode : 1
                        // SpliceSmoothing : 1
                        // ZonesCV : 0B
                        // ZonesRT : 1
                        if (keyIs ("Pitch"))
                        {
                            addChildValue (curChannelSection, "Pitch", [&valueList] (juce::ValueTree child)
                                {
                                    child.setProperty ("amount", valueList [0], nullptr);
                                });
                        }
                        else if (keyIs ("Level"))
                        {
                            addChildValue (curChannelSection, "Level", [&valueList] (juce::ValueTree child)
                                {
                                    child.setProperty ("amount", valueList [0], nullptr);
                                });
                        }
                        else if (keyIs ("Pan"))
                        {
                            addChildValue (curChannelSection, "Pan", [&valueList] (juce::ValueTree child)
                                {
                                    child.setProperty ("amount", valueList [0], nullptr);
                                });
                        }
                        else if (keyIs ("Release"))
                        {
                            addChildValue (curChannelSection, "Release", [&valueList] (juce::ValueTree child)
                                {
                                    child.setProperty ("amount", valueList [0], nullptr);
                                });
                        }
                        else if (keyIs ("MixLevel"))
                        {
                            addChildValue (curChannelSection, "MixLevel", [&valueList] (juce::ValueTree child)
                                {
                                    child.setProperty ("amount", valueList [0], nullptr);
                                });
                        }
                        else if (keyIs ("PMSource"))
                        {
                            addChildValue (curChannelSection, "PMSource", [&valueList] (juce::ValueTree child)
                                {
                                    child.setProperty ("amount", valueList [0], nullptr);
                                });
                        }
                        else if (keyIs ("PitchCV"))
                        {
                            addChildValue (curChannelSection, "PitchCV", [&valueList] (juce::ValueTree child)
                                {
                                    child.setProperty ("cvInput", valueList [0], nullptr);
                                    child.setProperty ("amount", valueList [1], nullptr);
                                });
                        }
                        else if (keyIs ("PMIndexMod"))
                        {
                            addChildValue (curChannelSection, "PMIndexMod", [&valueList] (juce::ValueTree child)
                                {
                                    child.setProperty ("cvInput", valueList [0], nullptr);
                                    child.setProperty ("amount", valueList [1], nullptr);
                                });
                        }
                        else if (keyIs ("ReleaseMod"))
                        {
                            addChildValue (curChannelSection, "ReleaseMod", [&valueList] (juce::ValueTree child)
                                {
                                    child.setProperty ("cvInput", valueList [0], nullptr);
                                    child.setProperty ("amount", valueList [1], nullptr);
                                });
                        }
                        else if (keyIs ("PanMod"))
                        {
                            addChildValue (curChannelSection, "PanMod", [&valueList] (juce::ValueTree child)
                                {
                                    child.setProperty ("cvInput", valueList [0], nullptr);
                                    child.setProperty ("amount", valueList [1], nullptr);
                                });
                        }
                        else if (keyIs ("LoopStartMod"))
                        {
                            addChildValue (curChannelSection, "LoopStartMod", [&valueList] (juce::ValueTree child)
                                {
                                    child.setProperty ("cvInput", valueList [0], nullptr);
                                    child.setProperty ("amount", valueList [1], nullptr);
                                });
                        }
                        else if (keyIs ("LoopMode"))
                        {
                            addChildValue (curChannelSection, "LoopMode", [&valueList] (juce::ValueTree child)
                                {
                                    child.setProperty ("mode", valueList [0], nullptr);
                                });
                        }
                        else if (keyIs ("SpliceSmoothing"))
                        {
                            addChildValue (curChannelSection, "SpliceSmoothing", [&valueList] (juce::ValueTree child)
                                {
                                    child.setProperty ("mode", valueList [0], nullptr);
                                });
                        }
                        else if (keyIs ("ZonesCV"))
                        {
                            addChildValue (curChannelSection, "ZonesCV", [&valueList] (juce::ValueTree child)
                                {
                                    child.setProperty ("cvInput", valueList [0], nullptr);
                                });
                        }
                        else if (keyIs ("ZonesRT"))
                        {
                            addChildValue (curChannelSection, "ZonesRT", [&valueList] (juce::ValueTree child)
                                {
                                    child.setProperty ("value", valueList [0], nullptr);
                                });
                        }
                        else
                        {
                            jassertfalse;
                        }
                    }
                }
                break;
                case ParseState::ParsingZoneSection:
                {
                    // Sample : sampleA.wav
                    // SampleStart : 79416
                    // SampleEnd : 6058
                    // MinVoltage : +4.56
                    // LoopLength: 256.0000
                    if (keyIs ("Sample"))
                    {
                        addChildValue (curZoneSection, "Sample", [&valueList] (juce::ValueTree sampleChild)
                            {
                                sampleChild.setProperty ("fileName", valueList [0], nullptr);
                            });
                    }
                    else if (keyIs ("SampleStart"))
                    {
                        addChildValue (curZoneSection, "SampleStart", [&valueList] (juce::ValueTree sampleStartChild)
                            {
                                sampleStartChild.setProperty ("sampleOffset", valueList [0], nullptr);
                            });
                    }
                    else if (keyIs ("SampleEnd"))
                    {
                        addChildValue (curZoneSection, "SampleEnd", [&valueList] (juce::ValueTree sampleEndChild)
                            {
                                sampleEndChild.setProperty ("sampleOffset", valueList [0], nullptr);
                            });
                    }
                    else if (keyIs ("MinVoltage"))
                    {
                        addChildValue (curZoneSection, "MinVoltage", [&valueList] (juce::ValueTree sampleEndChild)
                            {
                                sampleEndChild.setProperty ("voltage", valueList [0], nullptr);
                            });
                    }
                    else if (keyIs ("LoopLength"))
                    {
                        addChildValue (curZoneSection, "LoopLength", [&valueList] (juce::ValueTree sampleEndChild)
                            {
                                sampleEndChild.setProperty ("sampleCount", valueList [0], nullptr);
                            });
                    }
                    else
                    {
                        jassertfalse;
                    }
                }
                break;
                default:
                    jassertfalse;
                break;
            }
        }
    }

    void initUi ()
    {
        mainWindow.reset (new MainWindow (getApplicationName (),
                                          persistentRootProperties.getValueTree (), runtimeRootProperties.getValueTree ()));
    }

    void initPropertyRoots ()
    {
        runtimeRootProperties.wrap ({}, ValueTreeWrapper::WrapperType::owner, ValueTreeWrapper::EnableCallbacks::yes);
        runtimeRootProperties.setAppVersion (getApplicationVersion ());
        runtimeRootProperties.setAppDataPath (appDirectory.getFullPathName ());
        runtimeRootProperties.onQuitStateChanged = [this] (RuntimeRootProperties::QuitState quitState) { localQuitState.store (quitState); };

        persistentRootProperties.wrap ({}, ValueTreeWrapper::WrapperType::owner, ValueTreeWrapper::EnableCallbacks::no);
        // connect the Properties file and the AppProperties ValueTree with the propertiesFile (ValueTreeFile with auto-save)
        appPropertiesFile.init (persistentRootProperties.getValueTree (), appDirectory.getChildFile (getApplicationName () + PropertiesFileExtension), true);
    }

    void initAppDirectory ()
    {
        // locate the appProperties file in the User Application Data Directory

        //   On Windows, something like "\Documents and Settings\username\Application Data".
        //   On the Mac, it is "~/Library/Artiphon".
        //   On GNU/Linux it is "~/.config".

        const juce::String propertiesFilePath { juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getFullPathName () };
        appDirectory = juce::File (propertiesFilePath).getChildFile (ProjectInfo::companyName).getChildFile (getApplicationName ());
        if (! appDirectory.exists ())
        {
            const auto result { appDirectory.createDirectory () };
            if (! result.wasOk ())
            {
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon, "Application Startup Error",
                    "Unable to create " + getApplicationName () + " preferences directory, '" + appDirectory.getFullPathName () + "'", {}, nullptr,
                    juce::ModalCallbackFunction::create ([this] (int) { quit (); }));
                return;
            }
        }
    }

    void initLogger ()
    {
        auto getSessionTextForLogFile = [this] ()
        {
            auto resultOrNa = [] (juce::String result)
            {
                if (result.isEmpty ())
                    return juce::String ("n/a");
                else
                    return result;
            };
            const auto nl { juce::String ("\n") };
            auto welcomeText { juce::String (getApplicationName () + " - v" + getApplicationVersion () + " Log File" + nl) };
            welcomeText += " OS: " + resultOrNa (juce::SystemStats::getOperatingSystemName ()) + nl;
            welcomeText += " Device Description: " + resultOrNa (juce::SystemStats::getDeviceDescription ()) + nl;
            welcomeText += " Device Manufacturer: " + resultOrNa (juce::SystemStats::getDeviceManufacturer ()) + nl;
            welcomeText += " CPU Vendor: " + resultOrNa (juce::SystemStats::getCpuVendor ()) + nl;
            welcomeText += " CPU Model: " + resultOrNa (juce::SystemStats::getCpuModel ()) + nl;
            welcomeText += " CPU Speed: " + resultOrNa (juce::String (juce::SystemStats::getCpuSpeedInMegahertz ())) + nl;
            welcomeText += " Logical/Physicals CPUs: " + resultOrNa (juce::String (juce::SystemStats::getNumCpus ())) + "/" + resultOrNa (juce::String (juce::SystemStats::getNumPhysicalCpus ())) + nl;
            welcomeText += " Memory: " + resultOrNa (juce::String (juce::SystemStats::getMemorySizeInMegabytes ())) + "mb" + nl;
            return welcomeText;
        };
        fileLogger = std::make_unique<juce::FileLogger> (appDirectory.getChildFile ("DebugLog"), getSessionTextForLogFile ());
        juce::Logger::setCurrentLogger (fileLogger.get ());
    }

    void initCrashHandler ()
    {
        juce::SystemStats::setApplicationCrashHandler (crashHandler);
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name, juce::ValueTree persistentRootProperties, juce::ValueTree runtimeRootProperties)
            : DocumentWindow (name,
                              juce::Desktop::getInstance ().getDefaultLookAndFeel ()
                                                          .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent (persistentRootProperties, runtimeRootProperties), true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
            centreWithSize (getWidth (), getHeight ());
           #endif

            setVisible (true);
        }

        void closeButtonPressed () override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            JUCEApplication::getInstance ()->systemRequestedQuit ();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    juce::File appDirectory;
    ValueTreeFile appPropertiesFile;
    PersistentRootProperties persistentRootProperties;
    RuntimeRootProperties runtimeRootProperties;
    std::unique_ptr<juce::FileLogger> fileLogger;
    std::atomic<RuntimeRootProperties::QuitState> localQuitState { RuntimeRootProperties::QuitState::idle };
    std::unique_ptr<MainWindow> mainWindow;

//    BezierProperties bezier;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (A8ManagerApplication)

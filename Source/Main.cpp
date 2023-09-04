#include <JuceHeader.h>
#include "AppProperties.h"
#include "GUI/MainComponent.h"
#include "Utility/DebugLog.h"
#include "Utility/PersistentRootProperties.h"
#include "Utility/RootProperties.h"
#include "Utility/RuntimeRootProperties.h"
#include "Utility/ValueTreeFile.h"
#include "Utility/ValueTreeMonitor.h"
#include "Assimil8or/Assimil8orValidator.h"
#include "Assimil8or/PresetManagerProperties.h"
#include "Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "Assimil8or/Preset/PresetProperties.h"

// this requires the third party Melatonin Inspector be installed and added to the project
// https://github.com/sudara/melatonin_inspector
#define ENABLE_MELATONIN_INSPECTOR 0

const juce::String PropertiesFileExtension { ".properties" };

void crashHandler (void* /*data*/)
{
    FlushDebugLog ();
    juce::Logger::writeToLog (juce::SystemStats::getStackBacktrace ());
    FlushDebugLog ();
}

class A8ManagerApplication : public juce::JUCEApplication, public juce::Timer
{
public:
    A8ManagerApplication () {}
    const juce::String getApplicationName () override { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion () override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed () override { return true; }

    void valueTreeTest ()
    {
        juce::ValueTree root { "Root" };
        juce::ValueTree child1 { "RootChild1" };
        juce::ValueTree child2 { "RootChild2" };
        juce::ValueTree child3 { "RootChild3" };
        juce::ValueTree child4 { "RootChild4" };
        ValueTreeMonitor rootListenerLogger;
        rootListenerLogger.assign (root);
        ValueTreeMonitor child1ListenerLogger;
        child1ListenerLogger.assign (child1);
        ValueTreeMonitor child2ListenerLogger;
        child2ListenerLogger.assign (child2);
        ValueTreeMonitor child3ListenerLogger;
        child3ListenerLogger.assign (child3);
        root.addListener (&rootListenerLogger);
        child1.addListener (&child1ListenerLogger);
        child2.addListener (&child2ListenerLogger);
        child3.addListener (&child3ListenerLogger);

        auto basicPropertyTest = [] (juce::ValueTree& vt)
        {
            vt.setProperty ("propOne", "1", nullptr);
            vt.setProperty ("propOne", "2", nullptr);
            vt.removeProperty ("propOne", nullptr);
            vt.setProperty ("propTwo", "3", nullptr);
            vt.setProperty ("propThree", "4", nullptr);
        };
        basicPropertyTest (root);

        root.addChild (child1, -1, nullptr);
        basicPropertyTest (child1);
        child1.addChild (child2, -1, nullptr);
        child3.copyPropertiesAndChildrenFrom (child1, nullptr);
        child2 = child4;
        basicPropertyTest (child4);
        root.removeChild (child1, nullptr);
    }

#if 0
    // TEST CODE
    // encoded flag - 0000 1xxx | xxxx xxxx | xxxx xxxx | xxxy yyyy
    //                mask
    //                      whole value
    //                                                       up to 5 bits of fractional index
    // uint32_t rawValue
    // double finalValue
    // if bits 24-32 are all 0
    //     finalValue = rawValue
    // else
    //  mask = rawValue >> 23
    //  numberOfIndexBits = mask + 1
    //  wholeValue = rawValue >> numIndexBits
    //  fractionalResolution = 1.0 / (1L << (numberOfIndexBits))
    //  fractionalIndex = rawValue & mask
    //  fractionalValue = fractionalResolution * fractionalIndex
    //  finalValue = static_cast<double>(wholeValue) + fractionalValue
    // max 8 digit number - 0101 1111 0101 1110 0000 1111 1111
    // 2048 = 1000 0000 0000, 11/1
    // 1024 = 0100 0000 0000, 10/2
    //  512 = 0010 0000 0000,  9/3
    //  256 = 0001 0000 0000,  8/4
    //  128 = 0000 1000 0000,  7/5
    uint32_t encodeLoopLengthValue (double rawValue)
    {
        if (rawValue < 4.0)
            rawValue = 4.0;
        else if (rawValue > 99999999.0)
            rawValue = 99999999.0;

        if (rawValue < 2048)
        {
            // encode
            const auto wholeValue { static_cast<uint32_t>(rawValue) };
            const auto fractionalValue { rawValue - static_cast<double>(wholeValue) };
            const auto mask = [wholeValue] ()
            {
                if (wholeValue >= 1024) // 2047 - 1024
                    return 1;
                else if (wholeValue >= 512) // 1023 - 512
                    return 3;
                else if (wholeValue >= 256) // 511 - 256
                    return 7;
                else if (wholeValue >= 128)
                    return 15;
                else if (wholeValue >= 4)
                    return 31;
                else
                {
                    jassertfalse;
                    return 0;
                }
            }();
            const auto numIndexBits { mask + 1 };
            const auto fractionalResolution { 1.0 / (1L << (numIndexBits - 1)) };
            const auto maskValue { mask << 27 };
            const auto shiftedWholeValue { wholeValue << numIndexBits };
            const auto fractionalIndex { static_cast<int>(fractionalValue / fractionalResolution) };
            return maskValue + shiftedWholeValue + fractionalIndex;
        }
        else
        {
            return static_cast<uint32_t>(rawValue);
        }
    }

    double decodeLoopLengthValue (uint32_t rawValue)
    {
        auto newVal { rawValue & 0xF8000000 };
        if ((rawValue & 0xF8000000) == 0)
            return rawValue;

        const auto mask { rawValue >> 27 };
        const auto numIndexBits { mask + 1 };
        const auto wholeValue { (rawValue & 0x07FFFFFF) >> numIndexBits};
        const auto fractionalResolution { 1.0 / (1L << (numIndexBits - 1)) };
        const auto fractionalIndex { rawValue & mask };
        const auto fractionalValue { fractionalResolution * fractionalIndex};
        const auto finalValue { std::round ((static_cast<double>(wholeValue) + fractionalValue) * 1000.0) / 1000.0 }; // add the whole and fractional parts and round to 3 decimal places
        return finalValue;
    }
#endif // 0

    void initialise ([[maybe_unused]] const juce::String& commandLine) override
    {
#if 0
        auto testAValue = [this] (double valueToTest)
        {
            const auto encodedValue { encodeLoopLengthValue (valueToTest) };
            const auto decodedValue { decodeLoopLengthValue (encodedValue) };
            jassert (valueToTest == decodedValue);
        };
        testAValue (2047.0);
        testAValue (2047.5);
        testAValue (1024.0);
        testAValue (1023.0);
        testAValue (1023.25);
        testAValue (500.125);
        testAValue (400.875);
        testAValue (212.438);
        testAValue (93.188);
        testAValue (86.813);

#endif // 0
        //valueTreeTest ();
        initAppDirectory ();
        initLogger ();
        initCrashHandler ();
        initPropertyRoots ();
        initAssimil8or ();
        initUi ();

        // async quit timer
        startTimer (125);
    }

    void shutdown () override
    {
        persitentPropertiesFile.save ();
        mainWindow = nullptr; // (deletes our window)
        juce::Logger::setCurrentLogger (nullptr);
    }

    void anotherInstanceStarted ([[maybe_unused]] const juce::String& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }

    void suspended () override
    {
        runtimeRootProperties.triggerAppSuspended (false);
    }

    void resumed () override
    {
        runtimeRootProperties.triggerAppResumed (false);
    }

    void systemRequestedQuit () override
    {
        // reset preferred quit state
        runtimeRootProperties.setPreferredQuitState (RuntimeRootProperties::QuitState::now, false);
        // listeners for 'onSystemRequestedQuit' can do runtimeRootProperties.setPreferredQuitState (RuntimeRootProperties::QuitState::idle);
        // if they need to do something, which also makes them responsible for calling runtimeRootProperties.setQuitState (RuntimeRootProperties::QuitState::now); when they are done...
        runtimeRootProperties.triggerSystemRequestedQuit (false);
        localQuitState.store (runtimeRootProperties.getPreferredQuitState ());
    }

    void timerCallback () override
    {
        if (localQuitState.load () == RuntimeRootProperties::QuitState::now)
            quit ();
    }

    void initAssimil8or ()
    {
        // debug tool for watching changes on the Preset Value Tree
        //presetPropertiesMonitor.assign (presetProperties.getValueTreeRef ());

        PresetManagerProperties presetManagerProperties (runtimeRootProperties.getValueTree (), PresetManagerProperties::WrapperType::owner, PresetManagerProperties::EnableCallbacks::no);
        // initialize the Preset with defaults
        PresetProperties::copyTreeProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType),
                                              presetProperties.getValueTree ());
        presetManagerProperties.addPreset ("edit", presetProperties.getValueTree ());
        presetManagerProperties.addPreset ("unedited", presetProperties.getValueTree ().createCopy ());
        // add the Preset to the Runtime Root
        runtimeRootProperties.getValueTree ().addChild (presetManagerProperties.getValueTree (), -1, nullptr);

        assimil8orValidator.init (rootProperties.getValueTree ());
    }

    void initUi ()
    {
        mainWindow.reset (new MainWindow (getApplicationName () + " - v" + getApplicationVersion (), rootProperties.getValueTree ()));
    }

    void initPropertyRoots ()
    {
        persistentRootProperties.wrap (rootProperties.getValueTree (), PersistentRootProperties::WrapperType::owner, PersistentRootProperties::EnableCallbacks::no);
        // connect the Properties file and the AppProperties ValueTree with the propertiesFile (ValueTreeFile with auto-save)
        persitentPropertiesFile.init (persistentRootProperties.getValueTree (), appDirectory.getChildFile ("app" + PropertiesFileExtension), true);
        appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::owner, AppProperties::EnableCallbacks::no);
        appProperties.setMaxMruEntries (1);
        runtimeRootProperties.wrap (rootProperties.getValueTree (), RuntimeRootProperties::WrapperType::owner, RuntimeRootProperties::EnableCallbacks::yes);
        runtimeRootProperties.setAppVersion (getApplicationVersion (), false);
        runtimeRootProperties.setAppDataPath (appDirectory.getFullPathName (), false);
        runtimeRootProperties.onQuitStateChanged = [this] (RuntimeRootProperties::QuitState quitState) { localQuitState.store (quitState); };

        if (appProperties.getMostRecentFolder ().isEmpty ())
            appProperties.setMostRecentFolder (appDirectory.getFullPathName ());
    }

    void initAppDirectory ()
    {
        // locate the appProperties file in the User Application Data Directory

        //   On Windows, something like "\Documents and Settings\username\Application Data".
        //   On the Mac, it is "~/Library/Company".
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
        MainWindow (juce::String name, juce::ValueTree rootProperties)
            : DocumentWindow (name,
                              juce::Desktop::getInstance ().getDefaultLookAndFeel ().findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent (rootProperties), true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
            centreWithSize (getWidth (), getHeight ());
           #endif

            setVisible (true);

#if ENABLE_MELATONIN_INSPECTOR
            inspector.setVisible (true);
#endif
        }

        void closeButtonPressed () override
        {
            JUCEApplication::getInstance ()->systemRequestedQuit ();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
#if ENABLE_MELATONIN_INSPECTOR
        melatonin::Inspector inspector { *this, false };
#endif
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    juce::File appDirectory;
    RootProperties rootProperties;
    ValueTreeFile persitentPropertiesFile;
    PersistentRootProperties persistentRootProperties;
    AppProperties appProperties;
    RuntimeRootProperties runtimeRootProperties;
    Assimil8orValidator assimil8orValidator;
    PresetProperties presetProperties;
    std::unique_ptr<juce::FileLogger> fileLogger;
    std::atomic<RuntimeRootProperties::QuitState> localQuitState { RuntimeRootProperties::QuitState::idle };
    std::unique_ptr<MainWindow> mainWindow;

    ValueTreeMonitor presetPropertiesMonitor;
};

// This macro generates the main () routine that launches the app.
START_JUCE_APPLICATION (A8ManagerApplication)

#include <JuceHeader.h>
#include "AppProperties.h"
#include "Assimil8or/Assimil8orValidator.h"
#include "Assimil8or/PresetManagerProperties.h"
#include "Assimil8or/Audio/AudioPlayer.h"
#include "Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "Assimil8or/Preset/PresetProperties.h"
#include "GUI/GuiProperties.h"
#include "GUI/MainComponent.h"
#include "GUI/Assimil8or/Editor/SampleManager/SampleManager.h"
#include "Utility/DebugLog.h"
#include "Utility/DirectoryValueTree.h"
#include "Utility/PersistentRootProperties.h"
#include "Utility/RootProperties.h"
#include "Utility/RuntimeRootProperties.h"
#include "Utility/ValueTreeFile.h"
#include "Utility/ValueTreeMonitor.h"

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

    void initialise ([[maybe_unused]] const juce::String& commandLine) override
    {
        initAppDirectory ();
        initLogger ();
        initCrashHandler ();
        initPropertyRoots ();
        initAssimil8or ();
        initAudio ();
        initUi ();

        //ValueTreeHelpers::dumpValueTreeContent (runtimeRootProperties.getValueTree (), false, [this] (juce::String line) { DebugLog ("", line); });

        // async quit timer
        startTimer (125);
    }

    void shutdown () override
    {
        audioPlayer.shutdownAudio ();
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

        // add the Preset Manager to the Runtime Root
        runtimeRootProperties.getValueTree ().addChild (presetManagerProperties.getValueTree (), -1, nullptr);

        // setup the directory scanner
        directoryValueTree.init (runtimeRootProperties.getValueTree ());
        directoryDataProperties.wrap (directoryValueTree.getDirectoryDataPropertiesVT (), DirectoryDataProperties::WrapperType::client, DirectoryDataProperties::EnableCallbacks::no);
        // debug tool for watching changes on the Directory Data Properties Value Tree
        //directoryDataMonitor.assign (directoryDataProperties.getValueTreeRef ());

        // SampleManager requires that the PresetManagerProperties and DirectoryDataProperties are initialized
        sampleManager.init (rootProperties.getValueTree ());


        // when the folder being viewed changes, signal the directory scanner to rescan
        appProperties.onMostRecentFolderChange = [this] (juce::String folderName)
        {
            directoryDataProperties.setRootFolder (folderName, false);
            directoryDataProperties.triggerStartScan (false);
        };

        // start the initial directory scan, based on the last accessed folder stored in the app properties
        directoryDataProperties.setRootFolder (appProperties.getMostRecentFolder (), false);
        directoryDataProperties.triggerStartScan (false);

        // initialize the Validator
        assimil8orValidator.init (rootProperties.getValueTree ());
    }

    void initUi ()
    {
        guiProperties.wrap (persistentRootProperties.getValueTree (), GuiProperties::WrapperType::owner, GuiProperties::EnableCallbacks::no);
        mainWindow.reset (new MainWindow (getApplicationName () + " - v" + getApplicationVersion (), rootProperties.getValueTree ()));
    }

    void initPropertyRoots ()
    {
        persistentRootProperties.wrap (rootProperties.getValueTree (), PersistentRootProperties::WrapperType::owner, PersistentRootProperties::EnableCallbacks::no);
        // connect the Properties file and the AppProperties ValueTree with the propertiesFile (ValueTreeFile with auto-save)
        persitentPropertiesFile.init (persistentRootProperties.getValueTree (), appDirectory.getChildFile ("app" + PropertiesFileExtension), true);
        appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::owner, AppProperties::EnableCallbacks::yes);
        appProperties.setMaxMruEntries (1);
        runtimeRootProperties.wrap (rootProperties.getValueTree (), RuntimeRootProperties::WrapperType::owner, RuntimeRootProperties::EnableCallbacks::yes);
        runtimeRootProperties.setAppVersion (getApplicationVersion (), false);
        runtimeRootProperties.setAppDataPath (appDirectory.getFullPathName (), false);
        runtimeRootProperties.onQuitStateChanged = [this] (RuntimeRootProperties::QuitState quitState) { localQuitState.store (quitState); };

        if (appProperties.getMostRecentFolder ().isEmpty ())
            appProperties.setMostRecentFolder (appDirectory.getFullPathName ());
    }

    void initAudio ()
    {
        audioPlayer.init (rootProperties.getValueTree ());
        //AudioSettingsProperties audioSettingsProperties (persistentRootProperties.getValueTree (), AudioSettingsProperties::WrapperType::client, AudioSettingsProperties::EnableCallbacks::no);
        //audioConfigPropertiesMonitor.assign (audioSettingsProperties.getValueTreeRef ());
    }

    void initAppDirectory ()
    {
        // locate the appProperties file in the User Application Data Directory

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
        MainWindow (juce::String name, juce::ValueTree rootPropertiesVT)
            : DocumentWindow (name,
                              juce::Desktop::getInstance ().getDefaultLookAndFeel ().findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent (rootPropertiesVT), true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
           #endif

            PersistentRootProperties prp (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
            guiProperties.wrap (prp.getValueTree (), GuiProperties::WrapperType::client, GuiProperties::EnableCallbacks::no);
            auto [width, height] { guiProperties.getSize () };
            auto [x, y] { guiProperties.getPosition () };
            if (x == -1 || y == -1)
                centreWithSize (width, height);
            else
                setBounds (x, y, width, height);

            setVisible (true);

#if ENABLE_MELATONIN_INSPECTOR
            inspector.setVisible (true);
#endif
        }

#if (! JUCE_IOS) && (! JUCE_ANDROID)
        void moved () override
        {
            guiProperties.setPosition (getBounds ().getX (), getBounds ().getY (), false);
            DocumentWindow::moved ();
        }

        void resized () override
        {
            guiProperties.setSize (getBounds ().getWidth (), getBounds ().getHeight (), false);
            DocumentWindow::resized ();
        }
#endif // ! JUCE_IOS && ! JUCE_ANDROID

        void closeButtonPressed () override
        {
            JUCEApplication::getInstance ()->systemRequestedQuit ();
        }

    private:
        GuiProperties guiProperties;
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
    GuiProperties guiProperties;
    RuntimeRootProperties runtimeRootProperties;
    SampleManager sampleManager;
    Assimil8orValidator assimil8orValidator;
    PresetProperties presetProperties;
    DirectoryValueTree directoryValueTree;
    DirectoryDataProperties directoryDataProperties;
    std::unique_ptr<juce::FileLogger> fileLogger;
    std::atomic<RuntimeRootProperties::QuitState> localQuitState { RuntimeRootProperties::QuitState::idle };
    std::unique_ptr<MainWindow> mainWindow;
    AudioPlayer audioPlayer;

    ValueTreeMonitor audioConfigPropertiesMonitor;
    ValueTreeMonitor directoryDataMonitor;
    ValueTreeMonitor presetPropertiesMonitor;
};

// This macro generates the main () routine that launches the app.
START_JUCE_APPLICATION (A8ManagerApplication)

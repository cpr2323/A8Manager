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
#include "Assimil8or/Preset/PresetProperties.h"

#include "Assimil8or/Preset/ParameterNames.h"

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

    void initialise ([[maybe_unused]] const juce::String& commandLine) override
    {
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
        // listeners for 'onSystemRequestedQuit' can do runtimeRootPropertiesVT.setPreferredQuitState (RuntimeRootProperties::QuitState::idle);
        // if they need to do something, which also makes them responsible for calling runtimeRootPropertiesVT.setQuitState (RuntimeRootProperties::QuitState::now); when they are done...
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
        // hack the preset data on to the runtime root until we get a proper valuetreewrapper for the preset
        presetPropertiesMonitor.assign (presetProperties.getValueTreeRef ());
        runtimeRootProperties.getValueTree ().addChild (presetProperties.getValueTree (), -1, nullptr);
        assimil8orValidator.init (rootProperties.getValueTree ());

        auto createPresetPropertiersFromParameterData = [] (juce::ValueTree presetPropertiesVT, juce::Identifier ident)
        {
            auto getString = [] (juce::ValueTree vt, juce::Identifier& ident)
            {
                if (!vt.hasProperty (ident))
                    juce::Logger::outputDebugString ("[ Parameter '" + vt.getProperty ("name").toString () + "' does not have a '" + ident.toString () + "' property] ");
                return vt.getProperty (ident).toString ();
            };
            auto getInt = [] (juce::ValueTree vt, juce::Identifier& ident)
            {
                if (!vt.hasProperty (ident))
                    juce::Logger::outputDebugString ("[ Parameter '" + vt.getProperty ("name").toString () + "' does not have a '" + ident.toString () + "' property] ");
                return static_cast<int> (vt.getProperty (ident));
            };
            auto getDouble = [] (juce::ValueTree vt, juce::Identifier& ident)
            { 
                if (!vt.hasProperty (ident))
                    juce::Logger::outputDebugString ("[ Parameter '" + vt.getProperty ("name").toString () + "' does not have a '" + ident.toString () + "' property] ");
                return static_cast<double> (vt.getProperty (ident));
            };

            ParameterDataListProperties pdlp;
            PresetProperties newPresetProperties (presetPropertiesVT, PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
            newPresetProperties.setData2AsCV (getString (pdlp.getParameter (Section::PresetId, Parameter::Preset::Data2asCVId), juce::Identifier (ident)), false);
            newPresetProperties.setXfadeACV (getString (pdlp.getParameter (Section::PresetId, Parameter::Preset::XfadeACVId), juce::Identifier (ident)), false);
            newPresetProperties.setXfadeAWidth (getDouble (pdlp.getParameter (Section::PresetId, Parameter::Preset::XfadeAWidthId), juce::Identifier (ident)), false);
            newPresetProperties.setXfadeBCV (getString (pdlp.getParameter (Section::PresetId, Parameter::Preset::XfadeBCVId), juce::Identifier (ident)), false);
            newPresetProperties.setXfadeBWidth (getDouble (pdlp.getParameter (Section::PresetId, Parameter::Preset::XfadeBWidthId), juce::Identifier (ident)), false);
            newPresetProperties.setXfadeCCV (getString (pdlp.getParameter (Section::PresetId, Parameter::Preset::XfadeCCVId), juce::Identifier (ident)), false);
            newPresetProperties.setXfadeCWidth (getDouble (pdlp.getParameter (Section::PresetId, Parameter::Preset::XfadeCWidthId), juce::Identifier (ident)), false);
            newPresetProperties.setXfadeDCV (getString (pdlp.getParameter (Section::PresetId, Parameter::Preset::XfadeDCVId), juce::Identifier (ident)), false);
            newPresetProperties.setXfadeDWidth (getDouble (pdlp.getParameter (Section::PresetId, Parameter::Preset::XfadeDWidthId), juce::Identifier (ident)), false);
            newPresetProperties.forEachChannel ([&pdlp, &ident, &getString, &getInt, &getDouble] (juce::ValueTree channelPropertiesVT)
            {
                auto setCvInputAndAMount = [&pdlp, &getString, &ident] (juce::String parameterId, std::function<void (juce::String, double)> setter)
                {
                    jassert (setter != nullptr);
                    auto parameter { pdlp.getParameter (Section::ChannelId, parameterId) };
                    const auto value { getString (parameter , juce::Identifier (ident)) };
                    const auto [cvInput, amount] { ChannelProperties::getCvInputAndValueFromString (value) };
                    setter (cvInput, amount);
                };
                ChannelProperties channelProperties (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                channelProperties.setAttack (getDouble (pdlp.getParameter (Section::ChannelId, Parameter::Channel::AttackId), juce::Identifier (ident)), false);
                channelProperties.setAttackFromCurrent (getInt (pdlp.getParameter (Section::ChannelId, Parameter::Channel::AttackFromCurrentId), juce::Identifier (ident)) == 1, false);
                setCvInputAndAMount (Parameter::Channel::AttackModId, [&channelProperties] (juce::String cvInput, double attackModAmount) {channelProperties.setAttackMod (cvInput, attackModAmount, false); });
                channelProperties.setAliasing (getInt (pdlp.getParameter (Section::ChannelId, Parameter::Channel::AliasingId), juce::Identifier (ident)), false);
                setCvInputAndAMount (Parameter::Channel::AliasingModId, [&channelProperties] (juce::String cvInput, double aliasingModAmount) {channelProperties.setAliasingMod (cvInput, aliasingModAmount, false); });
                channelProperties.setAutoTrigger (getInt (pdlp.getParameter (Section::ChannelId, Parameter::Channel::AutoTriggerId), juce::Identifier (ident)) == 1, false);
                channelProperties.setBits (getDouble (pdlp.getParameter (Section::ChannelId, Parameter::Channel::BitsId), juce::Identifier (ident)), false);
                setCvInputAndAMount (Parameter::Channel::BitsModId, [&channelProperties] (juce::String cvInput, double bitsModAmount) {channelProperties.setBitsMod (cvInput, bitsModAmount, false); });
                channelProperties.setChannelMode (getInt (pdlp.getParameter (Section::ChannelId, Parameter::Channel::ChannelModeId), juce::Identifier (ident)), false);
                setCvInputAndAMount (Parameter::Channel::ExpAMId, [&channelProperties] (juce::String cvInput, double expAMAmount) {channelProperties.setExpAM (cvInput, expAMAmount, false); });
                setCvInputAndAMount (Parameter::Channel::ExpFMId, [&channelProperties] (juce::String cvInput, double expFMAmount) {channelProperties.setExpFM (cvInput, expFMAmount, false); });
                channelProperties.setLevel (getDouble (pdlp.getParameter (Section::ChannelId, Parameter::Channel::LevelId), juce::Identifier (ident)), false);
                setCvInputAndAMount (Parameter::Channel::LinAMId, [&channelProperties] (juce::String cvInput, double linAMAmount) {channelProperties.setLinAM (cvInput, linAMAmount, false); });
                channelProperties.setLinAMisExtEnv (getInt (pdlp.getParameter (Section::ChannelId, Parameter::Channel::LinAMisExtEnvId), juce::Identifier (ident)) == 1, false);
                setCvInputAndAMount (Parameter::Channel::LinFMId, [&channelProperties] (juce::String cvInput, double linFMAmount) {channelProperties.setLinFM (cvInput, linFMAmount, false); });
                setCvInputAndAMount (Parameter::Channel::LoopLengthModId, [&channelProperties] (juce::String cvInput, double loopLengthModAmount) {channelProperties.setLoopLengthMod (cvInput, loopLengthModAmount, false); });
                channelProperties.setLoopMode (getInt (pdlp.getParameter (Section::ChannelId, Parameter::Channel::LoopModeId), juce::Identifier (ident)), false);
                setCvInputAndAMount (Parameter::Channel::LoopStartModId, [&channelProperties] (juce::String cvInput, double loopStartModAmount) {channelProperties.setLoopStartMod (cvInput, loopStartModAmount, false); });
                channelProperties.setMixLevel (getDouble (pdlp.getParameter (Section::ChannelId, Parameter::Channel::MixLevelId), juce::Identifier (ident)), false);
                setCvInputAndAMount (Parameter::Channel::MixModId, [&channelProperties] (juce::String cvInput, double mixModAmount) {channelProperties.setMixMod (cvInput, mixModAmount, false); });
                channelProperties.setMixModIsFader (getInt (pdlp.getParameter (Section::ChannelId, Parameter::Channel::MixModIsFaderId), juce::Identifier (ident)) == 1, false);
                channelProperties.setPan (getDouble (pdlp.getParameter (Section::ChannelId, Parameter::Channel::PanId), juce::Identifier (ident)), false);
                setCvInputAndAMount (Parameter::Channel::PanModId, [&channelProperties] (juce::String cvInput, double panModAmount) {channelProperties.setPanMod (cvInput, panModAmount, false); });
                setCvInputAndAMount (Parameter::Channel::PhaseCVId, [&channelProperties] (juce::String cvInput, double phaseCvAmount) {channelProperties.setPhaseCV (cvInput, phaseCvAmount, false); });
                channelProperties.setPitch (getDouble (pdlp.getParameter (Section::ChannelId, Parameter::Channel::PitchId), juce::Identifier (ident)), false);
                setCvInputAndAMount (Parameter::Channel::PitchCVId, [&channelProperties] (juce::String cvInput, double pitchCvAmount) {channelProperties.setPitchCV (cvInput, pitchCvAmount, false); });
                channelProperties.setPlayMode (getInt (pdlp.getParameter (Section::ChannelId, Parameter::Channel::PlayModeId), juce::Identifier (ident)), false);
                channelProperties.setPMIndex (getDouble (pdlp.getParameter (Section::ChannelId, Parameter::Channel::PMIndexId), juce::Identifier (ident)), false);
                setCvInputAndAMount (Parameter::Channel::PMIndexModId, [&channelProperties] (juce::String cvInput, double pMIndexModAmount) {channelProperties.setPMIndexMod (cvInput, pMIndexModAmount, false); });
                channelProperties.setPMSource (getInt (pdlp.getParameter (Section::ChannelId, Parameter::Channel::PMSourceId), juce::Identifier (ident)), false);
                channelProperties.setRelease (getDouble (pdlp.getParameter (Section::ChannelId, Parameter::Channel::ReleaseId), juce::Identifier (ident)), false);
                setCvInputAndAMount (Parameter::Channel::ReleaseModId, [&channelProperties] (juce::String cvInput, double releaseModAmount) {channelProperties.setReleaseMod (cvInput, releaseModAmount, false); });
                channelProperties.setReverse (getInt (pdlp.getParameter (Section::ChannelId, Parameter::Channel::ReverseId), juce::Identifier (ident)) == 1, false);
                setCvInputAndAMount (Parameter::Channel::SampleEndModId, [&channelProperties] (juce::String cvInput, double sampleEndModAmount) {channelProperties.setSampleEndMod (cvInput, sampleEndModAmount, false); });
                setCvInputAndAMount (Parameter::Channel::SampleStartModId, [&channelProperties] (juce::String cvInput, double sampleStartModAmount) {channelProperties.setSampleStartMod (cvInput, sampleStartModAmount, false); });
                channelProperties.setSpliceSmoothing (getInt (pdlp.getParameter (Section::ChannelId, Parameter::Channel::SpliceSmoothingId), juce::Identifier (ident)) == 1, false);
                channelProperties.setXfadeGroup (getString (pdlp.getParameter (Section::ChannelId, Parameter::Channel::XfadeGroupId), juce::Identifier (ident)), false);
                channelProperties.setZonesCV (getString (pdlp.getParameter (Section::ChannelId, Parameter::Channel::ZonesCVId), juce::Identifier (ident)), false);
                channelProperties.setZonesRT (getInt (pdlp.getParameter (Section::ChannelId, Parameter::Channel::ZonesRTId), juce::Identifier (ident)), false);

                channelProperties.forEachZone ([&pdlp, &ident, &getString, &getInt, &getDouble] (juce::ValueTree zonePropertiesVT)
                {
                    ZoneProperties zoneProperties (zonePropertiesVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                    zoneProperties.setLevelOffset (getDouble (pdlp.getParameter (Section::ZoneId, Parameter::Zone::LevelOffsetId), juce::Identifier (ident)), false);
                    zoneProperties.setLoopLength (getDouble (pdlp.getParameter (Section::ZoneId, Parameter::Zone::LoopLengthId), juce::Identifier (ident)), false);
                    zoneProperties.setLoopStart (getInt (pdlp.getParameter (Section::ZoneId, Parameter::Zone::LoopStartId), juce::Identifier (ident)), false);
                    zoneProperties.setMinVoltage (getDouble (pdlp.getParameter (Section::ZoneId, Parameter::Zone::MinVoltageId), juce::Identifier (ident)), false);
                    zoneProperties.setPitchOffset (getDouble (pdlp.getParameter (Section::ZoneId, Parameter::Zone::PitchOffsetId), juce::Identifier (ident)), false);
                    zoneProperties.setSample (getString (pdlp.getParameter (Section::ZoneId, Parameter::Zone::SampleId), juce::Identifier (ident)), false);
                    zoneProperties.setSampleStart (getInt (pdlp.getParameter (Section::ZoneId, Parameter::Zone::SampleStartId), juce::Identifier (ident)), false);
                    zoneProperties.setSampleEnd (getInt (pdlp.getParameter (Section::ZoneId, Parameter::Zone::SampleEndId), juce::Identifier (ident)), false);
                    zoneProperties.setSide (getInt (pdlp.getParameter (Section::ZoneId, Parameter::Zone::SideId), juce::Identifier (ident)), false);
                    return true;
                });
                return true;
            });
        };
        auto makePresetDataFile = [this, &createPresetPropertiersFromParameterData] (juce::String filename, juce::String ident)
        {
            PresetProperties newPresetProperties;
            createPresetPropertiersFromParameterData (newPresetProperties.getValueTree (), ident);
            auto file { juce::File (appDirectory).getChildFile (filename) };
            auto xml = newPresetProperties.getValueTree ().createXml ();
            xml->writeTo (file, {});

        };
        makePresetDataFile ("preset_min.xml", "min");
        makePresetDataFile ("preset_max.xml", "max");
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
        runtimeRootProperties.wrap (rootProperties.getValueTree (), RuntimeRootProperties::WrapperType::owner, RuntimeRootProperties::EnableCallbacks::no);
        runtimeRootProperties.setAppVersion (getApplicationVersion (), false);
        runtimeRootProperties.setAppDataPath (appDirectory.getFullPathName (), false);

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

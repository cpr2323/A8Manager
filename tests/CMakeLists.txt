juce_add_console_app(A8ManagerRegressionTests PRODUCT_NAME "A8Manager Regression Tests")
juce_generate_juce_header(A8ManagerRegressionTests)

# Compile the actual implementation under test, not copies of its algorithms.
target_sources(A8ManagerRegressionTests PRIVATE
    RegressionTests.cpp
    ../Source/Assimil8or/Assimil8orPreset.cpp
    ../Source/Assimil8or/FileTypeHelpers.cpp
    ../Source/Assimil8or/Audio/AudioManager.cpp
    ../Source/Assimil8or/Preset/ChannelProperties.cpp
    ../Source/Assimil8or/Preset/ParameterPresetListProperties.cpp
    ../Source/Assimil8or/Preset/ParameterPresetsSingleton.cpp
    ../Source/Assimil8or/Preset/PresetHelpers.cpp
    ../Source/Assimil8or/Preset/PresetProperties.cpp
    ../Source/Assimil8or/Preset/ZoneProperties.cpp
    ../submodules/oolib/oolib/ValueTree/ValueTreeHelpers.cpp)

target_include_directories(A8ManagerRegressionTests PRIVATE
    ../Source
    ${OOLIB_INCLUDE_DIR})
target_compile_features(A8ManagerRegressionTests PRIVATE cxx_std_17)
target_compile_definitions(A8ManagerRegressionTests PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    DONT_SET_USING_JUCE_NAMESPACE=1)
target_link_libraries(A8ManagerRegressionTests PRIVATE
    A8ManagerBinaryData
    juce::juce_audio_formats
    juce::juce_data_structures
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)

add_test(NAME ParserCvRegression COMMAND A8ManagerRegressionTests --parser-cv)
add_test(NAME StereoSplitRegression COMMAND A8ManagerRegressionTests --stereo-split)
set_tests_properties(ParserCvRegression StereoSplitRegression PROPERTIES TIMEOUT 60)

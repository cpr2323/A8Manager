# A8Manager

A tool to manage Presets and Sample files for the Rossum-Electro Assimil8or

Windows and macOS builds available at: https://cpr2323.github.io/a8manager/index.html

# Building

The JUCE and oolib submodules must be initialised before the first build:

```
git submodule update --init --recursive
```

Then configure and build with CMake:

```
cmake -B cmake_build
cmake --build cmake_build --config Release
```

# Windows

There are no special steps to installing on Windows.

# OSX

Since the application is not signed (I don't want to pay the $99/yr) you will have to do a manual step in the console to allow it to run.

1. Download A8Manager.zip, the app will be automatically extracted during download.
2. Use the following command to allow it to run
3. **_xattr -d com.apple.quarantine ~/Downloads/A8Manager.app_**
4. You can now run it. You can move it to the Applications folder if you do desire.

# Linux

Linux builds are supported through the CMake/JUCE build. On Fedora, install the build tools and JUCE system dependencies first:

```
sudo dnf install git cmake gcc-c++ pkgconf-pkg-config \
    alsa-lib-devel freetype-devel fontconfig-devel libcurl-devel \
    libX11-devel libXcomposite-devel libXcursor-devel libXext-devel \
    libXinerama-devel libXrandr-devel libXrender-devel libXi-devel \
    mesa-libEGL-devel mesa-libGL-devel gtk3-devel webkit2gtk4.1-devel
```

Then initialise the submodules and build:

```
git submodule update --init --recursive
cmake -B cmake_build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake_build --config Release
```

The executable will be written under `cmake_build/A8Manager_artefacts/Release/`.

This was verified on Fedora 44 KDE.

I also did a quick test of this back in 2023 and published the results in a youtube video.
[A8Manager Linux build verification video](https://www.youtube.com/watch?v=fk4RRMh7hZc)

# Thanks

Thanks to Shawn Rakestraw for helping out with reverse engineering, some coding, documentation, testing, etc.

Thanks to Jean Obuchowicz for the great icon!

![](<Source/GUI/Assimil8or/Data/377906243_984640136172516_2914152204379747274_n.png>)

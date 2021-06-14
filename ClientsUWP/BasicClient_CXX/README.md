This is the basic client to be used with the basic Linux TCP server program.

In Visual Studio 2019: 
  1) build a new project type "Blank App (Universal Windows - c++/CX)".
  2) Copy and paste the contents of the <Grid> markup block from MainPage.xaml into the newly created project's MainPage.xaml
    - This will create the GUI
  3) Copy and paste the MainPage.xaml.cpp and MainPage.xaml.h into the newly create versions
  4) Open the newly created Package.appxmanifest, go to "Capabilities" and ensure "Internet (Client & Server"; "Internet (Client)"; and "Private Networks (Client & Server)" are checked.
  5) Build to HoloLens 2 in preferred manner.
    a) E.G. Set to Release, ARM64; and in Project/...Properties in the Debugging tab put the IPv4 of HoloLens 2 in "Machine Name" and ensure "Authentication Type" is "Universal (Unencrypted Protocl)"

The primary application layer for the socket code is in SocketStuff.h - which is loaded into a Windows (non-ISO) standard of C++ (either C++/WinRT C++/CLI or the like)
  MAKE SURE TO PUT IN THE SERVER IP ADDRESS AND PORT NUMBER IN THE PREDEFINED MACROS, FOUND IN SocketStuff.h
  
While running on HoloLens 2, start the basic TCP Linux server. While the server is running click the button within the HoloLens 2 UWP application.
  If successful, message should be echoded back in the text box.
  Else, failure message is printed.

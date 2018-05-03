# UWP Support

This project is a minor fork of the 7z library with modifications to run well on the UWP platform. The following modifications have been made:
* 7z COM-style objects support IAgileObject. Without doing this, the .NET runtime will try to marshal the 7z objects across threads and fail.
* Convenient vcxproj file which excludes code not needed to build the core library, in order to pass the Windows App Certification Kit (WACK).
* All code changes are gated with a "UWP" preprocessor macro check to make it easy to find the places which were modified due to API restrictions.

## What's missing

There is no sample UWP project which uses the 7z library included. This is currently left as an excerise for the reader. Many of the 7z COM-style
interfaces are documented in the 7z header source. In addition, [there is a tutorial](https://www.codeproject.com/Articles/27148/C-NET-Interface-for-Zip-Archive-DLLs)
which covers some of the basics.
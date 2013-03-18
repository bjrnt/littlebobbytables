littlebobbytables
=================

To compile from Git you should replace the Wesnoth/projectfiles folder with your own projectfiles. If you don't have any use the projectfiles folder in the root of the project.

Keep in mind that if you download a clean copy of the repository you must add the Tobii SDK libraries. 

This is done in two steps.
1. Add tobii-gaze-sdk-3.0.45-win-Win32\Cpp\Lib\tetio.lib to your linker settings.
2. Add tobii-gaze-sdk-3.0.45-win-Win32\Cpp\Include to your search directories.

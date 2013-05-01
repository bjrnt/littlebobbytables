littlebobbytables
=================

General guidelines for using Git's issue system:

1. Om ni hittar en bugg, skapa en issue på GitHub och tagga den med rätt label. Typ "bug", "enhancement" etc.
2. Om ni börjar arbeta på en issue, lägg till er själva som "Assigned" på den issuen, så att inte två personer råka arbeta på den samtidigt.
3. Om ni committar något i relation till en issue, ha då med "Issue #X" i commitmeddelandet. Det syns då att ni comittat kod i relation med den issuen i trackingsystemet. Jag rekommenderar även att man har med issuens titel i commitmeddelandet för att göra det extra tydligt. 
4. När ni är klara med en issue, skriv en förklarande kommentar till issuen om hur ni löste problemet. Detta för att underlätta om det skulle råka bli följdfel på fixen eller om en liknande bugg hittas som kanske kan lösas på samma sätt.

To compile from Git you should replace the Wesnoth/projectfiles folder with your own projectfiles. If you don't have any use the projectfiles folder in the root of the project.

Keep in mind that if you download a clean copy of the repository you must add the Tobii SDK libraries. This is done in two steps.

1. Add tobii-gaze-sdk-3.0.45-win-Win32\Cpp\Lib\tetio.lib to your linker settings.
2. Add tobii-gaze-sdk-3.0.45-win-Win32\Cpp\Include to your search directories.

# ModDeclare
A text based way to download minecraft mods (and resources) over the internet via modrinth

# All Keywords
| name | description | notes |
| --- | --- | --- |
| condition | allows you to specify on the command line whether to include the specific mod/resource | |
| if  | alias of condition | |
| require | specifies whether a mod need to be included or not | require/desire is required for mods/resources; if it can't be included, then an error will be given |
| desire | specifies whether a mod should optionally be included | if the mod/resource can't be included, a warning will be given |
| author | helps filter searches, include if possible, but not required | |
| include-all-dependencies | specifies whether to include all dependencies or not | if you only need required dependencies, then don't include this |
| include-all-deps | alias of include-all-dependencies | |
| include-dependencies | specifies whether to download dependencies of a mod, or only the specified mod | if you want to/would rather explicitly include dependencies, then don't include this |
| include-deps | alias of include-dependencies | |
| allow-betas | This allows the program to also include betas of mods in its search for a specific mod | whether or not the mod works during runtime is not the program's concern |
| betas | alias of allow-betas | |
| allow-alphas | This allows the program to also include alphas of mods in its search for a specific mod | whether or not the mod works during runtime is not the program's concern 
| alphas | alias of allow-alphas | |
| latest-version | specifies whether to download the latest version or not | this is the default behavior if no version is included |
| latest | alias of latest-version |  |
| loader | specifies what loader to search for | fabric, forge, neoforge, et cetera |
| minecraft-version | specifies what minecraft version to look for |  |
| mc-version | alias of mc-version |  |
| mod | specifies that the resource type you want to download is a mod | |
| organization | specifies what organization the resource is under | this may not be helpful, author is generally better to search under |
| org | alias of organization | |
| project-version | specifies a specific version of the resource to look for |  |
| proj-version | alias of project-version | |
| resource-pack | specifies that the resource type you want to download is a resource/texture pack  | |
| resource | alias of resource-pack | |
| max-errors | sets the maximum amount of errors that can occur when looking for mods/resources before the program will exit | 0 by default |
| max-warnings | sets the maximum amount of warnings that can occur when looking for mods/resources before the program will exit | 5 by default |
| mod-location | where to download the mods to | should be a folder, relative folder locations do work with this |
| resource-location | where to download the resource packs to | should be a folder, relative folder locations do work with this |

-# Not all keywords may be displayed, this is a laziness issue on my part and will likely be fixed


# Examples
### Generic Example
```
require mod "Concurrent Chunk Management Engine" author "ishland" loader "fabric" mc-version "26.1.2" latest-version
require mod "Sodium" author "jellysquid3" mc-version "26.1.2" latest-version
require mod "Zoomify" author "isxander" mc-version "26.1.2" latest-version
```
### Download Location
```
mod-location "./ModsFolder"
max-errors 0
max-warnings 0

# require mod "Fabric API" author "modmuss50" mc-version "26.2" latest-version
desire mod "Sodium" organization "CaffeineMC" mc-version "26.2" latest-version
desire mod "Fabric API" author "modmuss50" mc-version "26.2" latest-version
desire mod "Xaero's Minimap" author "thexaero" mc-version "26.2" latest-version
desire mod "Xaero's World Map" author "thexaero" mc-version "26.2" latest-version
```
### Command Line Flags
```
# loader fabric requested from commandline
condition "fabric" {
    require mod "sodium" organization "CaffeineMC" loader "fabric"
    require mod "Xaero's minimap" loader "fabric"
}
# loader neoforge requested from commandline
if "neoforge" {
    require mod "sodium" organization "CaffeineMC" loader "neoforge"
    require mod "Xaero's minimap" loader "neoforge"
}
# loader forge requested from commandline
if "forge" {
    require mod "Xaero's minimap" loader "forge"
}
```
### Another Generic Modlist
```
require mod "Sodium" loader "fabric" author "jellysquid3"
require mod "Iris Shaders" loader "fabric" author "coderbot"
desire mod "somecoolmodthatdoesntexist"
require mod "Sodium" org "CaffeineMC"
```
### My Actual Modpack For The Server I Play On
```
mod-location "./M&M_TEST"
max-errors 0
max-warnings 0

require loader "fabric" mc-version "26.1.2" latest-version {
    mod "3d-Skin-Layers" author "tr7zw"
    mod "AppleSkin" author "squeek502"
    mod "Architectury" author "shedaniel"
    mod "Axiom" author "Moulberry"
    mod "Bobby" author "johni0702"
    mod "Chat Heads" author "dzwdz"
    mod "ChatAnimation" author "Ezzenix"
    mod "Chest Tracker (Unofficial port)" author "ponuing"
    mod "Cloth Config API" author "shedaniel"
    mod "CommandKeys"
    mod "Container Preview" author "Xeonz"
    mod "Continuity"
    mod "Controlling" author "Jaredlll08"
    mod "Cubes Without Borders" author "Kira-NT"
    mod "Entity Model Features" author "Traben"
    mod "Entity Texture Features" author "Traben"
    mod "EntityCulling" author "tr7zw"
    mod "Fabric API" author "modmuss50"
    mod "Fabric Language Kotlin" author "modmuss50"
    mod "Forge Config API Port" author "Fuzs"
    mod "Freecam" author "hashalite"
    mod "Fzzy Config" author "fzzyhmstrs"
    mod "Inventory Profiles Next" author "blackd"
    mod "Iris" author "coderbot"
    mod "Item Scroller" author "masa"
    mod "Just Enough Items" author "mezz"
    mod "LibJF"
    mod "Litematica" author "masa"
    mod "MaLiLib" author "masa"
    mod "Mod Menu" author "Prospector"
    mod "Mouse Tweaks"
    mod "Plasmo Voice" author "kpids"
    mod "Punchy!" organization "Punchy Guys Studios"
    mod "Smooth Gui" author "Ezzenix"
    mod "Sodium" author "jellysquid3"
    mod "Tweakeroo" author "masa"
    mod "WorldEdit" organization "EngineHub"
    mod "Xaero's Minimap" author "thexaero"
    mod "Xaero's World Map" author "thexaero"
    mod "YetAnotherConfigLib" author "isxander"
    mod "Zoomify" author "isxander"
    mod "libIPN" author "blackd"
}
desire loader "fabric" mc-version "26.1.2" latest-version {
    mod "Autowalk" author "Ciocolata47"
    mod "Better Advancements" author "way2muchnoise"
    mod "Better Mount HUD" author "Lortseam"
    mod "CameraOverhaul" author "Mirsario"
    mod "Fabrishot" author "ramidzkh"
    mod "Jade" author "Snownee"
    mod "Language Reload" author "Jerozgen"
    mod "Lighty" author "andi-makes"
    mod "Lithium" author "jellysquid3"
    mod "Krypton" author "astei"
    mod "MiniHUD" author "masa"
    mod "More Chat History"
    mod "No Waypoints/Locator Bar" author "litetex"
    mod "Not Enough Crashes" author "natanfudge"
    mod "Nvidium" author "Cortex"
    mod "SearchStats"
    mod "Searchables" author "Jaredlll08"
    mod "Server Pack Unlocker"
    mod "SignFinder" author "Hyouka"
    mod "Remove Reloading Screen" author "dima_dencep"
    mod "ShadowTrace" author "Baktus_79"
    mod "Fullbright"
}
```
### Project Version
```
# require mod "Sodium" author "jellysquid3" mc-version "26.1.2" loader "fabric" project-version "vf7UgZpC"
require mod "Sodium" author "jellysquid3" mc-version "1.17" loader "fabric" latest-version
```
### Test Download
```
# allow warnings, dont allow errors
max-errors 0
max-warnings 5

# rendering
require loader "fabric" mc-version "1.21" {
    mod "Iris Shaders" author "coderbot"
}
# optimization
require loader "fabric" mc-version "1.21" organization "CaffeineMC" {
    mod "Sodium"
    mod "Lithium"
    # desire overwrites "require"
    desire mod "Phosphor"
    desire mod "Hydrogen"
}
```
### Test Scope
```
# rendering mods
require loader "fabric" {
    mod "Sodium" author "jellysquid3"
    mod "Iris" author "coderbot"
}
# helpers
desire mod "AppleSkin" author "squeek502"
desire loader "neoforge" mod "AppleSkin"
desire loader "neoforge" mod "Just Enough Items"
```
### Resources
```
resource-location "../resource"
mod-location "../mod"

require resource-pack "mandalas-gui-dark-mode" author "CesarZorak"
require mod "Sodium" author "jellysquid3" loader "fabric"
```

### TODO
- Add Tests
- Add Shader Support
- Add Modpack Support (i.e. declare what modpack you want and its resources will be downloaded, from there you can add on top of it)

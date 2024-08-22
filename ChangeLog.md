# SuperTuxKart 0.6.3 Alpha-3a (October 2023)

* Bugfix : Powerups didn't calculate pitch while aiming.
* Bugfix : In Mines, the last kart couldn't get the first lap
           in a Grand Prix.

# SuperTuxKart 0.6.3 Alpha-3 (October 2023) (not released)

* Bugfix : The Island track wasn't working.
* Bugfix : Parachute was using light.
* Bugfix : Title screen texture wasn't coordinated to the GUI.
* Bugfix : In Secret Garden, the first kart wasn't counted as first.
* Replaced some textures with ones from 0.7.
* Fixed Fort Magma.
* Added Scotland track.
* Added challenges.
* Updated all karts and lightened some karts to gain performance.
* Updated tracks screenshots.
* Updated Jungle, Shifting Sands, Star Track
* Ported 0.7 Oliver's math class.

# SuperTuxKart 0.6.3 Pre-alpha 3 (July 2023)

* Added two new menu background pictures.
* Added Mozilla kart from 0.7.
* Added Elephpant kart from 0.7-SVN.
* Fixed Tux and Evil Tux being too heavy for the game.
* Fixed Nolok and Puffy's wheels.
* Deleted Subsea track.
* For the first time, four tracks from 0.7 were backported :
  - Hacienda.
  - The Island (broken atm).
  - Mines.
  - Snow Tux Peak.
* Updated Secret Garden and Canyon.
* Added more karts in the add-ons folder.
* Bugfix : It was possible to see through Gnu's beard.

# SuperTuxKart 0.6.3 Alpha-2.2 (June 2023)

* Updated Tux, Evil Tux and Gooey karts 
  but with **HUGE** performance problems.
* Updated XR591.
* Added back Subsea track.
* Replaced old parachute, with the one from 0.7.
* Replaced Nolok's icon.
* Bugfix: Pidgin skidmarks wouldn't display.

# SuperTuxKart 0.6.3 Alpha-2 (June 2023)

* Replaced Hexley's **BIG** wheels with little ones.
* Updated Wilber, Puffy and Evil Tux karts.
* Made Fort Magma's castle bigger, to make AI not going to wall.
* Deleted Subsea.
* Changed speedometer's foreground texture.
* Updated Oliver's Math Class music.

# SuperTuxKart 0.6.3 Alpha-1 (May 2023)

* Added Pidgin kart.
* Added Secret Garden and Subsea track (originally from 0.5).
* Updated Puffy, Wilber, Mozilla, Evil Tux and Nolok karts.
* Replaced small Hexley, with a wide one.
* Added new musics in The Island and Snow Mountain tracks.
* Changed speedometer's background texture.
* Added an add-on folder in the game's directory.
* Added some challenges.
* Changed the maximum number of karts handled by the game to 10.

# SuperTuxKart 0.6.2a Modded (2015-2017) (never released)

* Tried to backport Nolok and did it,
  but with a **VERY** poor design.
* Tried to fix Fort Magma track but failed, AI
  are still going to walls.
* Backported the 0.7 menu background image.

## SuperTuxKart 0.6.2a (October 2009)

* Bugfix: STK would crash while trying to save the config file
          on Windows Vista.

## SuperTuxKart 0.6.2 (July 2009)

* Bugfix: Game could crash in rare circumstances.
* Bugfix: Restarting a GP (with the in-race menu ESC) would
          not subtract already allocated points.
* Bugfix: A race could be finished with an invalid shortcut.
* Bugfix: Playing a challenge after a splitscreen game would
          play the challenge in split screen.
* Bugfix: Items explode over void.
* Bugfix: Grass in castle arena slowed down the kart.
* Bugfix: GP result showed kart identifier instead of name.
* Improvement: there is now 1 1 sec. wait period for the race
          result screen, avoiding the problem that someone 
          presses space/enter at the end of a race, immediately
          quitting the menu before it can be read.

## SuperTuxKart 0.6.1a (February 2009)

* Bugfix: battle mode would not display track groups.

## SuperTuxKart 0.6.1 (February 2009)

* Added new kart ("Puffy"), new battle map ("Cave"), and new music
  for Snow Mountain.
* Fixed bug in track selection screen that could cause a crash
  when track groups were used.
* Fixed crash in character selection that could happen if an
  old user config file existed.
* Fixed incorrect rescues in Fort Magma.
* Improved track selection screen to not display empty track
  groups.
* A plunger in the face is now removed when restarting.
* Added slow-down for karts driving backwards.
* Somewhat reduced 'shaking' of AI driven karts.

## SuperTuxKart 0.6 (January 2009)

* New improved physics and kart handling.
* Added sharp turns and nitro speed boost. (replacing wheelies and jump)
* Totally rewrote powerups (plunger, bowling ball, cake, bubblegum) and new look for bananas.
* New and improved tracks  : Skyline, Snow Mountain, Race Track, Star Track, Mines, XR591.
* New game mode : 3-Strikes Battle.
* Major improvements to AI.
* New/improved karts (and removed some old ones) : 
  - Wilber, 
  - Evil Tux,
  - Hexley.
* Improved user interface.
* Karts now have a visible suspension effect.
* Fully positional audio with OpenAL.
* New music and sound effects. (including engine, braking and skidding sounds)
* Better support for mods and add-ons. (kart and track groups)
* New/updated translations. (ga fi de nl sl fr it es ro sv)
* Allowed 'Grand Prix's of Time Trial, Follow the Leader, or any other mode.
* Challenges are now specified and config files, and are thus easy to create by users
* Improved build system to better detect missing dependencies.
* Improved shortcut-detection.
* Initial work towards networking. (disabled and hidden by default)
* Bug fixes and code refactor/cleanup/documentation :
  - Fixed 'joystick locks' (kart would turn even if the joystick is in neutral),
    thanks to Samjam for the patch.

## SuperTuxKart 0.5 (May 2008)

* Six new tracks and one improved track: Fort Magma, SnowTux Peak, Amazonian Journey, City, 
   Canyon, Crescent Crossing and Star Track.
* Complete Challenges to unlock game modes, new tracks and a skidding preview
* New Follow the Leader game mode.
* New Grand Prix.
* Improved User Interface.
* Improved game pad/joystick handling.
* German, French, Dutch, Spanish, Italian and Swedish translations.
* Additional music.
* Did many bugfixes including:
	a memory leak fix (Charlie Head)
	an AI crash fix (Chris Morris)


## SuperTuxKart 0.4 (February 2008)

* New physics handling using the bullet physics engine.
* New kart: Wilber.
* Improved 'Shifting Sands' and 'Lighthouse' tracks.
* Improved AI.
* New GUI handling, including resolution switching GUI.
* Improved input handling.
* Jump and look-back feature.
* Additional music and main theme.
	
	
## SuperTuxKart 0.3 (May 2007)

* Highscore lists.
* Shortcut detection.
* Improved AI.
* Fullscreen support.
* New track: The Island.
* New penalty: Bomb.
* Added MacOSX support.
* Added OpenAL and ogg-vorbis support.
* Added two new Grand Prixs.
* Improved user interface:
  - New racing interface.
  - Better track map.
  - Player kart dots in the track map are bigger than AI dots.
  - Track selection screen has topview pictures.
  - Added "Setup new race" option when a track is finished.
  - Added "Restart race" option when a track is finished.
  - The keyboard can skip vertical spaces between buttons.
  - Better control configuration.
  - Better in-game help.
  - Added .desktop file for menus and icon.
* Bugfixes:
  - Fixed bug in ssg_help::MinMax, which could cause a significant performance loss.
  - Fixed bug that allowed the joystick to erase the main menu.
  - Fixed bug that allowed the joystick to "play the game while paused".
  - Fixed screen_manager assert failure bug.
  - Fixed sound_manager assert failure bug.
  - Fixed keyboard keys unable to work on the first key press bug.
  - And others...

## SuperTuxKart 0.2 (22. Sep 2006)

* Significant performance improvement by using display lists.
* Improved AI.
* Support for different Grand Prixs.
* Too many bug fixes to list them all, but the important ones are:
  - Work around for 'karts fall through track' compiler bug.
  - Fixed rescue mode.
* Two new collectables: parachute and anvil.
* Added tracks screenshots in the track select screen.
* Keyboard handling allows gradual turning.
* Improved physics (still work in progress).
  * All hard-coded properties like maximum velocity have
    been replaced by dynamically computed data dependent
    on kart parameters, allowing for karts having different
    characteristics.
* Added help and about screens, added credits to track designers.
* Items were added to all tracks.

## SuperTuxKart 0.1 (04. May 2006) (not officially released)

* Significant speedup by using a new HOT and collision algorithm
  --> all tracks are now playable.
* Removed all SDL dependencies, only PLIB is needed.
* Single and multi-window menu can be used.
* Did some code structure changes.
* Did some bug fixes and small improvements.
* Added profile option to support automatic profiling.

## SuperTuxKart 0.0.0 (22. Dec 2004)

* New tracks.
* New characters and karts.
* New user-interface.
* Added some additional effects. (skid-marks, smoke)

### TuxKart v0.4.0 (March 19th 2004)

* Changes for compatibility with PLIB 1.8.0 and later.
* Removed some features that were only there to support
  truly ancient graphics cards like 3Dfx Voodoo-1/2.

### TuxKart v0.3.0 (??)

* Converted to use the new PLIB/PW library and thus
  avoid the need to link to GLUT.

### TuxKart v0.2.0 (Sept 3rd 2002)

* Changes for compatibility with PLIB 1.6.0 and later.

### TuxKart v0.0.5 (??)

* Changes for compatibility with PLIB 1.4.0 and later.

### TuxKart v0.0.4 (??)

* Changes to suit rassin-frassin-Windows-junk.
* Steady-cam camera - courtesy of cowtan@ysbl.york.ac.uk
* Changes for compatibility with PLIB 1.3.1 and later.
* Added new music courtesy of Matt Thomas.

### TuxKart v0.0.3 (July 4th 2000)

* Fixed bug in Keyboard driver when no
  joystick driver is installed.
* More CygWin fixes.
* Started new feature to allow you to be
  rescued from lava, etc.

### TuxKart v0.0.2 (July 2nd 2000)

* Added ability to add new tracks without
  recompiling.
* Can now drive using keyboard only - no joystick
  required.
* Should compile and run under Windows using CygWin.

### TuxKart v0.0.1 (July 1st 2000)

* Fixed a couple of files missing in initial
  Distro.

### TuxKart v0.0.0 (June 29th 2000)

* First CVS release.

### TuxKart (unnumbered) (April 13th 2000)

* First hack.
 
 
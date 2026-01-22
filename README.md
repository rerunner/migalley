MigAlleyRevisited
=================

A Flight Simulator.

# TARGET
A *native* Linux version of Rowan's MigAlley.

# The .plan

- [x] take the original Mig Alley source code from Rowan software.

- [x] create a CMake edition build system from scratch. 

- [x] Remove dependencies on Windows, MFC and DirectX/play/input libraries. LARGE changes are needed to accomplish this, and it will disable all UI related code, including the dynamic campaign map/planning tool.

- [x] Replace DirectDraw surfaces with SDL2 surfaces, run 3d world in non directx "software" mode

- [x] Replace Miles Sound System with SDL2 audio mixer.

- [x] Replace DirectInput keyboard code with SDL2 keyboard code.

- [x] Create a temporary simple portable UI, as the old UI was all MFC or WPC stuff. 

- [x] Add Vulkan staging and pipeline as alternative to SDL Renderer. Include a simple vulkan shader with CRT effect.

- [x] Activate in-flight Map and radio messages menu.

- [x] Create a Joystick port from DirectInput to SDL2.

- [ ] Recreate the full UI, activating all menus, maps, campaign planner. This is going to be a lot of work.

- [ ] See what is next (e.g. multiplayer, full vulkan replacement, etc).


# Status
The core game can be played, tested using Manjaro Linux. It currently uses:
- SDL2 for the 3D window management and graphics surfaces. 
- Vulkan (including shaders) for the staging buffers, fixed pipeline and display inside the SDL window.
- SDL Mixer for the audio
- SDL TTF for the in-game fonts+text.
- SDL input for mouse and joystick
 
The Main UI is not yet complete. It allows to start Hot Shot, Quick Mission, Campaign and Full War in single player mode. The Quick Mission UI is fairly complete, all missions and flights can be selected. The UI will grow to a real replacement of the original MFC UI. The main remaining challenge is the UI planning map for Campaign and Full War.

The project now relies on 32 bits SDL2 & GTK2 dependencies to build and run.

## Some status videos
[![Selecting Quick Mission]()](https://github.com/user-attachments/assets/c8b1d0c6-a937-49f9-b482-3a584d59f462)

[![HotShots Start]()](https://github.com/user-attachments/assets/7ef28ce2-03be-4202-b602-619b0a85203d)

[![In-Flight Menu]()](https://github.com/user-attachments/assets/022d6f0e-3e7d-4ec2-b3dc-a4c88da8168d)


# Game description
Set during the Korean War, Mig Alley is an interactive campaign-based flight simulation, with the option of instant action in minicampaigns, single historical missions, and head-to-head play. Add to this the astounding graphics, stunning landscapes, accurate flight models, and the world's largest jet-to-jet dogfights, and you have one the best games of next year on your hands.

Mig Alley includes the world's first and largest jet-to-jet dogfights, with over 50 aircraft in the sky at any one time.

You can fly any of the following aircraft, all with accurate flight models:

    F86 Sabre (the ultimate dogfighter), F84 Thunderjet(long-range strike-and-escort jet),
    F80 Shooting Star (ground-attack and fighter-jet aircraft),
    P51 Mustang (classic World War 2 propeller aircraft, serving as a ground-attack aircraft),
    MiG 15 and MiG 15bis (the fighter that nearly drove the UN air force from Korean skies).

Other aircraft appearing in the game include the B29 Super Fortress, Meteor, F9 Panther, A1 Skyraider, YAK piston-engined fighter, and many others.

Start off in a World War 2-vintage P51 Mustang, eventually moving on to a F86 Sabre "Mig Killer," or choose between a MiG or Sabre in the head-to-head modes.

Just as in Flying Corps, the ground detail in Mig Alley is painstakingly taken from aerial photography and satellite spy cameras from the era. Ground information has been drawn from satellite photographs covering areas of 45 miles long by 2.5 miles across. There's also a huge difference in terrain, with widely different colors in different parts of the peninsula and massive alterations in the ground altitude--there are mountains rising 9,000 feet. To give the Korean landscape justice, and because of the size of the peninsula and speed of the aircraft, a lot more information needs to be stored in the game.

The core of Mig Alley is the campaign game, set in spring 1951, a period during which the ground war was still fluid and both sides could have won a battlefield victory. During this period, the crack Russian MiG squadrons were introduced to the conflict. As a consequence, you'll find yourself under constant pressure from the start.

As the commander of a group of UN aircraft, your task is to move the front line to the Chinese border and win the war before the start of the peace talks. However, at the same time, the North Koreans and Chinese are busy building up their forces and launching attacks aimed at driving you from the Korean peninsula. You must plan your daily missions, maximizing the strengths of the various aircraft while minimizing their weaknesses.

Select missions from a wide range of targets--concentrate on attacking enemy troops, try to cut off their supplies, destroy bridges, or attempt to achieve air superiority. The success or failure of these missions has direct implications to the course of the ground battle in a cause-and-effect style.
